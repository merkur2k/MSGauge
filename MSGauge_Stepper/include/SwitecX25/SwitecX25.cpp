/*
 *  SwitecX25 Arduino Library
 *  Guy Carpenter, Clearwater Software - 2012
 *
 *  Licensed under the BSD2 license, see license.txt for details.
 *
 *  All text above must be included in any redistribution.
 */
#include "mbed.h"
#include "SwitecX25.h"

// During zeroing we will step the motor CCW
// with a fixed step period defined by RESET_STEP_MICROSEC
#define RESET_STEP_MICROSEC 800

// This table defines the acceleration curve.
// 1st value is the speed step, 2nd value is delay in microseconds
// 1st value in each row must be > 1st value in subsequent row
// 1st value in last row should be == maxVel, must be <= maxVel
static unsigned short defaultAccelTable[][2] = {
  {   20, 3000},
  {   50, 1500},
  {  100, 1000},
  {  150,  800},
  {  300,  600}
};
#define DEFAULT_ACCEL_TABLE_SIZE (sizeof(defaultAccelTable)/sizeof(*defaultAccelTable))

// experimentation suggests that 400uS is about the step limit
// with my hand-made needles made by cutting up aluminium from
// floppy disk sliders.  A lighter needle will go faster.

// State  3 2 1 0   Value
// 0      1 0 0 1   0x9
// 1      0 0 0 1   0x1
// 2      0 1 1 1   0x7
// 3      0 1 1 0   0x6
// 4      1 1 1 0   0xE
// 5      1 0 0 0   0x8
static uint8_t stateMap[] = {0x9, 0x1, 0x7, 0x6, 0xE, 0x8};

SwitecX25::SwitecX25(unsigned int steps, PinName pin1, PinName pin2, PinName pin3, PinName pin4)
{
  this->currentState = 0;
  this->steps = steps;
  this->pins[0] = new DigitalOut(pin1, false);
  this->pins[1] = new DigitalOut(pin2, false);
  this->pins[2] = new DigitalOut(pin3, false);
  this->pins[3] = new DigitalOut(pin4, false);

  dir = 0;
  vel = 0;
  stopped = true;
  currentStep = 0;
  targetStep = 0;

  accelTable = defaultAccelTable;
  maxVel = defaultAccelTable[DEFAULT_ACCEL_TABLE_SIZE-1][0]; // last value in table.
  this->stepTimer.start();
}

void SwitecX25::writeIO()
{

  uint8_t mask = stateMap[currentState];
  for (int i=0;i<pinCount;i++) {
    pins[i]->write(mask & 0x1?1:0);
    mask >>= 1;
  }
}

void SwitecX25::stepUp()
{
  if (currentStep < steps) {
    currentStep++;
    currentState = (currentState + 1) % stateCount;
    writeIO();
  }
}

void SwitecX25::stepDown()
{
  if (currentStep > 0) {
    currentStep--;
    currentState = (currentState + 5) % stateCount;
    writeIO();
  }
}

void SwitecX25::zero()
{
  currentStep = steps - 1;
  for (unsigned int i=0;i<steps;i++) {
    stepDown();
    wait_us(RESET_STEP_MICROSEC);
  }
  currentStep = 0;
  targetStep = 0;
  vel = 0;
  dir = 0;
}

// This function determines the speed and accel
// characteristics of the motor.  Ultimately it
// steps the motor once (up or down) and computes
// the delay until the next step.  Because it gets
// called once per step per motor, the calcuations
// here need to be as light-weight as possible, so
// we are avoiding floating-point arithmetic.
//
// To model acceleration we maintain vel, which indirectly represents
// velocity as the number of motor steps travelled under acceleration
// since starting.  This value is used to look up the corresponding
// delay in accelTable.  So from a standing start, vel is incremented
// once each step until it reaches maxVel.  Under deceleration
// vel is decremented once each step until it reaches zero.

void SwitecX25::advance()
{
  stepTimer.reset();

  // detect stopped state
  if (currentStep==targetStep && vel==0) {
    stopped = true;
    dir = 0;
    vel = 0;
    return;
  }

  // if stopped, determine direction
  if (vel==0) {
    dir = currentStep<targetStep ? 1 : -1;
    // do not set to 0 or it could go negative in case 2 below
    vel = 1;
  }

  if (dir>0) {
    stepUp();
  } else {
    stepDown();
  }

  // determine delta, number of steps in current direction to target.
  // may be negative if we are headed away from target
  int delta = dir>0 ? targetStep-currentStep : currentStep-targetStep;

  if (delta>0) {
    // case 1 : moving towards target (maybe under accel or decel)
    if (delta < vel) {
      // time to declerate
      vel--;
    } else if (vel < maxVel) {
      // accelerating
      vel++;
    } else {
      // at full speed - stay there
    }
  } else {
    // case 2 : at or moving away from target (slow down!)
    vel--;
  }

  // vel now defines delay
  unsigned char i = 0;
  // this is why vel must not be greater than the last vel in the table.
  while (accelTable[i][0]<vel) {
    i++;
  }
  microDelay = accelTable[i][1];
}

void SwitecX25::setPosition(unsigned int pos)
{
  // pos is unsigned so don't need to check for <0
  if (pos >= steps) pos = steps-1;
  targetStep = pos;
  if (stopped) {
    // reset the timer to avoid possible time overflow giving spurious deltas
    stopped = false;
    stepTimer.reset();
    microDelay = 0;
  }
}

void SwitecX25::update()
{
  if (!stopped) {
    if (stepTimer.read_us() >= microDelay) {
      advance();
    }
  }
}


//This updateMethod is blocking, it will give you smoother movements, but your application will wait for it to finish
void SwitecX25::updateBlocking()
{
  while (!stopped) {
    if (stepTimer.read_us() >= microDelay) {
      advance();
    }
  }
}

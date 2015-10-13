/*
 *  SwitecX25 Arduino Library
 *  Guy Carpenter, Clearwater Software - 2012
 *
 *  Ported to mbed by Jason Richardson - 2015
 *
 *  Licensed under the BSD2 license, see license.txt for details.
 *
 *  All text above must be included in any redistribution.
 */
#include "mbed.h"
#ifndef SwitecX25_h
#define SwitecX25_h

class SwitecX25
{
 public:
   static const uint8_t pinCount = 4;
   static const uint8_t stateCount = 6;
   DigitalOut* pins[pinCount];
   uint8_t currentState;      // 6 steps
   uint16_t currentStep;      // step we are currently at
   uint16_t targetStep;       // target we are moving to
   uint16_t steps;            // total steps available
   uint16_t microDelay;       // microsecs until next state
   uint16_t (*accelTable)[2]; // accel table can be modified.
   uint16_t maxVel;           // fastest vel allowed
   uint16_t vel;              // steps travelled under acceleration
   int8_t dir;                // direction -1,0,1
   bool stopped;              // true if stopped

   SwitecX25(unsigned int steps, PinName pin1, PinName pin2, PinName pin3, PinName pin4);

   void stepUp();
   void stepDown();
   void zero();
   void update();
   void updateBlocking();
   void setPosition(unsigned int pos);

 private:
   void advance();
   void writeIO();
   Timer stepTimer;
};


#endif

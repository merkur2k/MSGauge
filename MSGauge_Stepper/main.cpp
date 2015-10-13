/*
 *  Copyright (c) 2015 Jason Richardson
 *
 *  License: MIT open source (http://opensource.org/licenses/MIT)
 *      Summary;
 *      Use / modify / distribute / publish it how you want and
 *      if you use it, or don't, you can't hold me liable for how
 *      it does or doesn't work.
 *      If it doesn't work how you want, don't use it, or change
 *      it so that it does work.
 */

#include "mbed.h"
#include "Gauge_config.h"
#include "include/SwitecX25/SwitecX25.h"

uint16_t value = 1;
Timer alarmTimer;
Timer commTimer;
Timer displayTimer;

bool connectionState = true;
bool alarmDisplayStatus = false;

DigitalOut led2(LED2);
CAN can1(p9, p10);

// 315 degrees of range = 315x3 steps = 945 steps
// declare motor1 with 945 steps on pins 4-7
SwitecX25 motor(315*3, P2_10,P2_8,P2_6,P2_7);

// CAN receive interrupt handler
void canRX() {
  CANMessage msg;
  while(can1.read(msg)) {
    if (msg.id == 1520) {
      connectionState = true;
      commTimer.reset();
      value = msg.data[7] | (msg.data[6] << 8);
      led2 = !led2;
    }
  }
}

int main() {
  commTimer.start();
  alarmTimer.start();
  displayTimer.start();
  printf("Initializing...\r\n");
  can1.frequency(500000);

  can1.attach(canRX);

  motor.zero();   // this is a slow, blocking operation
  ////motor.setPosition(value);

  while(1) {
    // detect error conditions and toggle a bit to be used by display routines
    if (alarmTimer.read_ms() > 500) {
      alarmDisplayStatus = !alarmDisplayStatus;
      alarmTimer.reset();
    }

    // see if we have gotten any CAN messages in the last second. display an error if not
    if (commTimer.read_ms() > 1000) {
      commTimer.reset();
      connectionState = false;
      //value += 50;
      //if (value > 900) value = 1;
      //motor.setPosition(rand()%945);
      //motor.update();
      led2 = !led2;
    }

    if (!connectionState && alarmDisplayStatus) {

    }

//    if (displayTimer.read_ms() > 10) {
//      displayTimer.reset();
      //motor.update();
      // scale raw value
      // rev limit = 7000
      // full sweep (315 deg) = 945
      //printf("Setting position %d\r\n", value * 945 / 7000);
      if (value > 7000) value = 7000;
      motor.setPosition(value * 945 / 7000);
//    }
    motor.update();
  }
}

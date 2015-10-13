MegaSquirt CANbus gauge
==================

This is a simple single gauge for the [MegaSquirt](http://www.msextra.com/) Engine Controller.
It connects via CANbus and can output either via a 1.3" OLED display
or a small Switec X25 series stepper motor gauge needle driver.

It is built using the ARM [mbed](https://www.mbed.com/en/) platform, or more specicically a
port of it to GCC - [gcc4mbed](https://github.com/adamgreen/gcc4mbed/).

Quick start:
-----------
```
git clone https://github.com/merkur2k/MSGauge.git
cd MSGauge
git clone https://github.com/adamgreen/gcc4mbed.git```
Follow the instructions at https://github.com/adamgreen/gcc4mbed/ to build gcc4mbed for your platform.
Build one of the gauge types:
```
cd MSGauge_OLED
make```
or
```
cd MSGauge_Stepper
make```
Use an ISP utility such as flash Magic (Windows) or lpc21isp (Linux) to flash the .bin or .hex in the LPC11C24 folder.

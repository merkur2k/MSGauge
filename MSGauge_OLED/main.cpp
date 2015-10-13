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
#include "include/SSD1306/Adafruit_SSD1306.h"

uint16_t value = 1;
Timer alarmTimer;
Timer commTimer;
Timer displayTimer;

#if defined(GAUGE_MAP)
uint16_t baro = 0;
#endif

char tempchars[11];

bool connectionState = true;
bool alarmDisplayStatus = false;

DigitalOut led2(LED2);
CAN can1(p9, p10);

// an SPI sub-class that provides a constructed default
class SPIPreInit : public SPI
{
public:
    SPIPreInit(PinName mosi, PinName miso, PinName clk) : SPI(mosi,miso,clk)
    {
        format(8,3);
        frequency(4000000);
    };
};

SPIPreInit gSpi(P0_9,P0_8,P0_6);
//                             DC,  RST, CS
Adafruit_SSD1306_Spi oled(gSpi,P1_0,P1_1,P1_2,64,128);

void divby10(int val) {
  uint8_t length;

  itoa(val, tempchars, 10);
  length=strlen(tempchars);

  tempchars[length + 1]=tempchars[length]; // null shift right
  tempchars[length]=tempchars[length - 1]; //
  tempchars[length - 1]='.';
}

// CAN receive interrupt handler
void canRX() {
  CANMessage msg;
  while(can1.read(msg)) {
    if (msg.id == 1522) {
      connectionState = true;
      commTimer.reset();
#if defined(GAUGE_CLT)
      value = msg.data[7] | (msg.data[6] << 8);
#elif defined(GAUGE_MAT)
      value = msg.data[5] | (msg.data[4] << 8);
#elif defined(GAUGE_MAP)
      value = msg.data[3] | (msg.data[2] << 8);
      baro = msg.data[1] | (msg.data[0] << 8);
#endif
      led2 = !led2;
    }
  }
}

void drawGauge(void) {
  oled.drawLine(0, 12, 12, 22, WHITE);
  oled.drawLine(20, 12, 28, 22, WHITE);
  oled.drawLine(40, 12, 45, 22, WHITE);
  oled.drawLine(88, 12, 83, 22, WHITE);
  oled.drawLine(108, 12, 100, 22, WHITE);
  oled.drawLine(128, 12, 116, 22, WHITE);
  oled.drawFastVLine(64, 12, 10, WHITE);

  oled.setTextSize(1);
  oled.setTextColor(WHITE);
#if defined(GAUGE_CLT)
  oled.setTextCursor(44,27);
  oled.printf("COOLANT");
  oled.setTextCursor(32,36);
  oled.printf("TEMPERATURE");
  oled.setTextCursor(0,0);
  oled.printf("COLD");
  oled.setTextCursor(47,0);
  oled.printf("NORMAL");
  oled.setTextCursor(110,0);
  oled.printf("HOT");
#elif defined(GAUGE_MAP)
  oled.setTextCursor(40,27);
  oled.printf("MANIFOLD");
  oled.setTextCursor(40,36);
  oled.printf("PRESSURE");
  oled.setTextCursor(0,0);
  oled.printf("0");
  oled.setTextCursor(60,0);
  oled.printf("10");
  oled.setTextCursor(116,0);
  oled.printf("20");
#endif
}

void drawNeedle(uint16_t value) {
  uint8_t level;
  uint16_t alarmValue = 0;

  // scale "value" to 1-128 and store it in "level", and set bounds for error condition
  // value * 128 / full_scale
#if defined(GAUGE_CLT)
  //value /= 10;
  // scale 160-260, alarm > 230
  if (value >= 1600) {
    level = (value / 10 - 160) * 128 / 70;
  } else {
    level = 0;
  }
  alarmValue = 2300;
  oled.setTextSize(2);
  oled.setTextCursor(36,48);
  //oled.printf("%4d.%1d",value/10,value%10);
#elif defined(GAUGE_MAT)
  value /= 10;
  if (value >= 160) {
    level = (value - 160) * 128 / 70;
  } else {
    level = 0;
  }
  oled.setTextSize(2);
  oled.setTextCursor(36,48);
  //oled.printf("%4d.%1d",value/10,value%10);
  divby10(value);
  oled.printf("%s",tempchars);

#elif defined(GAUGE_MAP)
  // assume 2.5 bar MAP sensor with forced induction
  // needle scales for boost only, 0-21ish psi
  if (value > baro) {
    // in boost. 6.895kpa = 1psi
    value = (value - baro) * 200 / 1379;
    level = value * 128 / 210;
    oled.setTextSize(2);
    oled.setTextCursor(52,48);
    //oled.printf("%2d.%1d",value/10,value%10);
    divby10(value);
    oled.printf("%s",tempchars);
    oled.setTextSize(1);
    oled.setTextCursor(104,56);
    oled.printf("psi");
  } else {
    // in vacuum. 1 kPa = 0.2953 in Hg
    level = 0;
    value = abs(baro - value) * 2953 / 10000;
    oled.setTextSize(2);
    oled.setTextCursor(0,48);
    //oled.printf("%2d.%1d",value/10,value%10);
    divby10(value);
    oled.printf("%s",tempchars);
    oled.setTextSize(1);
    oled.setTextCursor(52,56);
    oled.printf("in/Hg");
  }
#endif
  if (level > 128) level = 128;
  oled.drawLine(level,  6, 62,64, WHITE, 45);
  oled.drawLine(level+1,6, 63,64, WHITE, 45);
  oled.drawLine(level+2,6, 64,64, WHITE, 45);



  // "flash" the display (invert) during error condition
  if (alarmDisplayStatus && alarmValue > 0 && value > alarmValue) {
    oled.invertDisplay(true);
  } else {
    oled.invertDisplay(false);
  }
}

void updateGauge(void) {
  drawGauge();
  drawNeedle(value);
  oled.display();
  oled.clearDisplay();
}

int main() {
  commTimer.start();
  alarmTimer.start();
  displayTimer.start();
  printf("Initializing...\r\n");
  can1.frequency(500000);

  oled.clearDisplay();
  oled.display();
//wait(3);
  updateGauge();

  can1.attach(canRX);

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
    }

    if (!connectionState && alarmDisplayStatus) {
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setTextCursor(0,56);
      oled.printf("No data");
    }

    if (displayTimer.read_ms() > 100) {
      displayTimer.reset();
      updateGauge();
    }
  }
}

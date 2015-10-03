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
#include "include/SSD1306/Adafruit_SSD1306.h"

uint16_t value = 1;
Timer alarmTimer;
Timer commTimer;
Timer displayTimer;

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

void drawNeedleLine(int16_t x0, int16_t y0,  int16_t x1, int16_t y1, uint16_t color, int16_t yStop)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (; x0<=x1; x0++)
    {
        if (steep) {
            oled.drawPixel(y0, x0, color);
            if (x0 > yStop) return;
        } else {
            if (y0 <= yStop) oled.drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
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
  oled.drawFastHLine(0,12, 128, BLACK);

  oled.setTextSize(1);
  oled.setTextColor(WHITE);
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
}

void drawNeedle(uint16_t value) {
  uint8_t level;
  uint8_t hpos;
  // scale "value" to 1-128 and store it in "level"
  // value * 128 / full_scale
  if (value >= 160) {
    level = (value - 160) * 128 / 70;
  } else {
    level = 0;
  }
  if (level > 128) level = 128;
  drawNeedleLine(level,  6, 62,64, WHITE, 45);
  drawNeedleLine(level+1,6, 63,64, WHITE, 45);
  drawNeedleLine(level+2,6, 64,64, WHITE, 45);

  oled.setTextSize(2);
  if (value < 10) {
    hpos = 72;
  } else if (value < 100) {
    hpos = 60;
  } else if (value < 1000) {
    hpos = 48;
  } else {
    hpos = 36;
  }
  oled.setTextCursor(hpos,47);
  oled.printf("%d",value);
  // "flash" the display (invert) during error condition
  if (alarmDisplayStatus && value > 200) {
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
  CANMessage msg;

  oled.clearDisplay();
  updateGauge();

  while(1) {
    if (alarmTimer.read_ms() > 500) {
      alarmDisplayStatus = !alarmDisplayStatus;
      alarmTimer.reset();
    }
    while(can1.read(msg)) {
      if (msg.id == 1522) {
        printf("Message received: %d\r\n", msg.id);
        connectionState = true;
        commTimer.reset();
        value = (msg.data[7] | (msg.data[6] << 8)) / 10;
        led2 = !led2;
      }
    }

    if (commTimer.read_ms() > 1000) { // see if we have gotten any CAN messages in the last second. display an error if not
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

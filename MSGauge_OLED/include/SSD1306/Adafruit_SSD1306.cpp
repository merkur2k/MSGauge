/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

/*
 *  Modified by Neal Horman 7/14/2012 for use in mbed
 */

#include "mbed.h"
#include "Adafruit_SSD1306.h"

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D

void Adafruit_SSD1306::begin(uint8_t vccstate)
{
    rst = 1;
    // VDD (3.3V) goes high at start, lets just chill for a ms
    wait_ms(1);
    // bring reset low
    rst = 0;
    // wait 10ms
    wait_ms(10);
    // bring out of reset
    rst = 1;
    // turn on VCC (9V?)

    command(SSD1306_DISPLAYOFF);
    command(SSD1306_SETDISPLAYCLOCKDIV);
    command(0x80);                                  // the suggested ratio 0x80

    command(SSD1306_SETMULTIPLEX);
    command(_rawHeight-1);

    command(SSD1306_SETDISPLAYOFFSET);
    command(0x0);                                   // no offset

    command(SSD1306_SETSTARTLINE | 0x0);            // line #0

    command(SSD1306_CHARGEPUMP);
    command((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    command(SSD1306_MEMORYMODE);
    command(0x00);                                  // 0x0 act like ks0108

    command(SSD1306_SEGREMAP | 0x1);

    command(SSD1306_COMSCANDEC);

    command(SSD1306_SETCOMPINS);
    command(_rawHeight == 32 ? 0x02 : 0x12);        // TODO - calculate based on _rawHieght ?

    command(SSD1306_SETCONTRAST);
    command(_rawHeight == 32 ? 0x8F : ((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF) );

    command(SSD1306_SETPRECHARGE);
    command((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);

    command(SSD1306_SETVCOMDETECT);
    command(0x40);

    command(SSD1306_DISPLAYALLON_RESUME);

    command(SSD1306_NORMALDISPLAY);

    command(SSD1306_DISPLAYON);
}

// Set a single pixel
void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
        return;

    // check rotation, move pixel around if necessary
    switch (getRotation())
    {
        case 1:
            swap(x, y);
            x = _rawWidth - x - 1;
            break;
        case 2:
            x = _rawWidth - x - 1;
            y = _rawHeight - y - 1;
            break;
        case 3:
            swap(x, y);
            y = _rawHeight - y - 1;
            break;
    }

    // x is which column
    if (color == WHITE)
        buffer[x+ (y/8)*_rawWidth] |= _BV((y%8));
    else // else black
        buffer[x+ (y/8)*_rawWidth] &= ~_BV((y%8));
}

void Adafruit_SSD1306::invertDisplay(bool i)
{
	command(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

// Send the display buffer out to the display
void Adafruit_SSD1306::display(void)
{
	command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
	command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
	command(SSD1306_SETSTARTLINE | 0x0); // line #0
	sendDisplayBuffer();
}

// Clear the display buffer. Requires a display() call at some point afterwards
void Adafruit_SSD1306::clearDisplay(void)
{
	std::fill(buffer.begin(),buffer.end(),0);
}

void Adafruit_SSD1306::splash(void)
{
#ifndef NO_SPLASH_ADAFRUIT
	uint8_t adaFruitLogo[] =
	{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x3E, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x07, 0x00, 0x3F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x07, 0x00, 0x73, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE7, 0x00, 0x63, 0x86, 0x0F, 0x87, 0xE0, 0x38, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFE, 0x03, 0xE7, 0x00, 0x61, 0x86, 0x3F, 0x8F, 0xF0, 0x3C, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xF8, 0x0F, 0xC7, 0x00, 0x61, 0x86, 0x7F, 0x9F, 0xF8, 0x7C, 0x00, 0x00,
  0xE1, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0x07, 0x00, 0x61, 0x86, 0x78, 0x1E, 0x38, 0x7E, 0x00, 0x00,
  0xE0, 0x7F, 0xFF, 0xFF, 0x80, 0xF8, 0x07, 0x00, 0x61, 0x86, 0xE0, 0x38, 0x38, 0x6E, 0x00, 0x00,
  0xE0, 0x1F, 0xFF, 0xFF, 0x07, 0xE0, 0x07, 0x00, 0x61, 0x86, 0xE0, 0x38, 0x00, 0xEE, 0x00, 0x00,
  0xE0, 0x07, 0xFF, 0xFC, 0x1F, 0x00, 0x07, 0x00, 0x61, 0x86, 0xFE, 0x30, 0xFC, 0xFE, 0x00, 0x00,
  0xE0, 0x01, 0xFF, 0xF0, 0x78, 0x00, 0x07, 0x00, 0x61, 0x86, 0xFE, 0x30, 0xFC, 0xFF, 0x00, 0x00,
  0xE0, 0x00, 0x7F, 0xE1, 0xE0, 0x01, 0xE7, 0x00, 0x61, 0x86, 0xFE, 0x30, 0xFC, 0xFF, 0x00, 0x00,
  0xE0, 0x00, 0x1F, 0x86, 0x00, 0x1F, 0xE7, 0x00, 0x61, 0x86, 0xC0, 0x30, 0xFD, 0xFF, 0x00, 0x00,
  0xE0, 0x00, 0x0E, 0x18, 0x00, 0xFF, 0xE7, 0x00, 0x61, 0x86, 0xE0, 0x38, 0x1D, 0xC3, 0x80, 0x00,
  0xE0, 0x00, 0x08, 0xC0, 0x0F, 0xFC, 0x07, 0x00, 0x61, 0x86, 0xE0, 0x38, 0x39, 0x83, 0x80, 0x00,
  0xE0, 0x00, 0x23, 0x01, 0xFF, 0x00, 0x07, 0x00, 0x61, 0x86, 0x70, 0x1E, 0x7B, 0x83, 0x80, 0x00,
  0xE0, 0x00, 0x88, 0x1F, 0xE0, 0x00, 0x07, 0x00, 0x61, 0x86, 0x7F, 0xDF, 0xFB, 0x83, 0x80, 0x00,
  0xE0, 0x01, 0x60, 0xF0, 0x00, 0x00, 0x07, 0x00, 0x61, 0x86, 0x3F, 0xCF, 0xF3, 0x81, 0x80, 0x00,
  0xE0, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x08, 0x40, 0x00, 0x03, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x18, 0x01, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x1C, 0x00, 0x1F, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x02, 0x00, 0x00, 0x00, 0x07, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x8F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x01, 0x00, 0xFC, 0x00, 0x00, 0x07, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x48, 0x07, 0xF0, 0x00, 0x07, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x23, 0x00, 0xFF, 0xE0, 0x07, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x08, 0x60, 0x0F, 0xFF, 0x07, 0x00, 0x60, 0x00, 0x3C, 0x18, 0x6C, 0xF9, 0xFF, 0x00,
  0xE0, 0x00, 0x0E, 0x18, 0x00, 0x7F, 0xE7, 0x00, 0x60, 0x00, 0xFE, 0x18, 0x6C, 0xFD, 0xFF, 0x00,
  0xE0, 0x00, 0x1F, 0x87, 0x00, 0x07, 0xE7, 0x00, 0x60, 0x01, 0xFF, 0x18, 0x6C, 0xFD, 0xFF, 0x00,
  0xE0, 0x00, 0x7F, 0xE1, 0xE0, 0x00, 0x67, 0x00, 0x70, 0x01, 0xE7, 0x98, 0x6C, 0x1E, 0x18, 0x00,
  0xE0, 0x03, 0xFF, 0xF8, 0x7C, 0x00, 0x07, 0x00, 0x78, 0x01, 0xC3, 0x98, 0x6C, 0x0E, 0x18, 0x00,
  0xE0, 0x0F, 0xFF, 0xFE, 0x1F, 0x00, 0x07, 0x00, 0x7F, 0xF3, 0x81, 0x98, 0x6C, 0x0E, 0x18, 0x00,
  0xE0, 0x1F, 0xFF, 0xFF, 0x03, 0xE0, 0x07, 0x00, 0x1F, 0xF3, 0x81, 0xD8, 0x6C, 0xFE, 0x18, 0x00,
  0xE0, 0x7F, 0xFF, 0xFF, 0xC0, 0xF8, 0x07, 0x00, 0x00, 0x3B, 0x0D, 0xD8, 0x6C, 0xFC, 0x18, 0x00,
  0xE3, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0x07, 0x00, 0x00, 0x3B, 0x1D, 0xD8, 0x6C, 0xFC, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFC, 0x0F, 0xC7, 0x00, 0x00, 0x1B, 0x9D, 0xD8, 0x6C, 0xE0, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFE, 0x03, 0xE7, 0x00, 0x00, 0x1B, 0x8F, 0x98, 0x6C, 0xF0, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE7, 0x00, 0x00, 0x19, 0xCF, 0x98, 0x6C, 0x70, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x07, 0x00, 0x00, 0x79, 0xE7, 0x9C, 0xEC, 0x78, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0x00, 0x3F, 0xF1, 0xFF, 0x1F, 0xEC, 0x3C, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x07, 0x00, 0x3F, 0xF0, 0xFF, 0x9F, 0xCC, 0x1C, 0x18, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	std::copy(
		&adaFruitLogo[0]
		, &adaFruitLogo[0] + (_rawHeight == 32 ? sizeof(adaFruitLogo)/2 : sizeof(adaFruitLogo))
		, buffer.begin()
		);
#endif
}

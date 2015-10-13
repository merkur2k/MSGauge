/***********************************
This is a our graphics core library, for all our displays.
We'll be adapting all the
existing libaries to use this core to make updating, support
and upgrading easier!

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

/*
 *  Modified by Neal Horman 7/14/2012 for use in mbed
 */

#include "mbed.h"

#include "Adafruit_GFX.h"
#include "glcdfont.h"


#if defined(GFX_WANT_ABSTRACTS) || defined(GFX_SIZEABLE_TEXT)
// bresenham's algorithm - thx wikpedia
void Adafruit_GFX::drawLine(int16_t x0, int16_t y0,  int16_t x1, int16_t y1, uint16_t color, uint16_t yStop)
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
          drawPixel(y0, x0, color);
          if (yStop > 0 && x0 >= yStop) return;
      } else {
          if (yStop > 0) {
            if (y0 <= yStop) drawPixel(x0, y0, color);
          } else {
            drawPixel(x0, y0, color);
          }
      }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // stupidest version - update in subclasses if desired!
    drawLine(x, y, x, y+h-1, color);
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // stupidest version - update in subclasses if desired!
    for (int16_t i=x; i<x+w; i++)
        drawFastVLine(i, y, h, color);
}
#endif

#if defined(GFX_WANT_ABSTRACTS)
// draw a rectangle
void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y+h-1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x+w-1, y, h, color);
}

void Adafruit_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // stupidest version - update in subclasses if desired!
    drawLine(x, y, x+w-1, y, color);
}

void Adafruit_GFX::fillScreen(uint16_t color)
{
    fillRect(0, 0, _width, _height, color);
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
    for (int16_t j=0; j<h; j++)
    {
        for (int16_t i=0; i<w; i++ )
        {
            if (bitmap[i + (j/8)*w] & _BV(j%8))
                drawPixel(x+i, y+j, color);
        }
    }
}
#endif

size_t Adafruit_GFX::writeChar(uint8_t c)
{
    if (c == '\n')
    {
        cursor_y += textsize*8;
        cursor_x = 0;
    }
    else if (c == '\r')
        cursor_x = 0;
    else
    {
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
        cursor_x += textsize*6;
        if (wrap && (cursor_x > (_width - textsize*6)))
        {
            cursor_y += textsize*8;
            cursor_x = 0;
        }
    }
    return 1;
}

// draw a character
void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if(
        (x >= _width) || // Clip right
        (y >= _height) || // Clip bottom
        ((x + 5 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0) // Clip top
        )
    return;

    for (int8_t i=0; i<6; i++ )
    {
        uint8_t line = 0;

        if (i == 5)
            line = 0x0;
        else
            line = font[(c*5)+i];

        for (int8_t j = 0; j<8; j++)
        {
            if (line & 0x1)
            {
#if defined(GFX_WANT_ABSTRACTS) || defined(GFX_SIZEABLE_TEXT)
                if (size == 1) // default size
                    drawPixel(x+i, y+j, color);
                else // big size
                    fillRect(x+(i*size), y+(j*size), size, size, color);
#else
                drawPixel(x+i, y+j, color);
#endif
            }
            else if (bg != color)
            {
#if defined(GFX_WANT_ABSTRACTS) || defined(GFX_SIZEABLE_TEXT)
                if (size == 1) // default size
                    drawPixel(x+i, y+j, bg);
                else // big size
                    fillRect(x+i*size, y+j*size, size, size, bg);
#else
                drawPixel(x+i, y+j, bg);
#endif
            }
            line >>= 1;
        }
    }
}

void Adafruit_GFX::setRotation(uint8_t x)
{
    x %= 4;  // cant be higher than 3
    rotation = x;
    switch (x)
    {
        case 0:
        case 2:
            _width = _rawWidth;
            _height = _rawHeight;
            break;
        case 1:
        case 3:
            _width = _rawHeight;
            _height = _rawWidth;
            break;
    }
}

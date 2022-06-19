#ifndef LCDDISPLAYPROXY_H_
#define LCDDISPLAYPROXY_H_

/* LCD screen driver for MSP432P401R
 *
 * I2C Port used is PORT 1 -> P1.6 = SDA, P1.7 = SCL
 *
 *
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <LCDdisplayI2C.h>
#include <utils.h>
#include "shared_vars.h"
#include <interrupt.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04
#define Rw 0x02
#define Rs 0x01

#define LCD_ADDR 0x27

typedef struct
{
    uint8_t addr;
    uint8_t displayfunction;
    uint8_t displaycontrol;
    uint8_t displaymode;
    uint8_t numlines;
    uint8_t cols;
    uint8_t rows;
    uint8_t backlightval;
    DisplayI2C *displayI2C;
} DisplayProxy;

extern DisplayProxy* LCD_create(void);
extern void LCD_destroy(DisplayProxy *lcd_config);
extern void LCD_init(DisplayProxy *lcd_config);
extern void LCD_configure(DisplayProxy *lcd_config);
extern void LCD_clear(DisplayProxy *lcd_config);
extern void LCD_home(DisplayProxy *lcd_config);
extern void LCD_noDisplay(DisplayProxy *lcd_config);
extern void LCD_display(DisplayProxy *lcd_config);
extern void LCD_noBlink(DisplayProxy *lcd_config);
extern void LCD_blink(DisplayProxy *lcd_config);
extern void LCD_noCursor(DisplayProxy *lcd_config);
extern void LCD_cursor(DisplayProxy *lcd_config);
extern void LCD_scrollDisplayLeft(DisplayProxy *lcd_config);
extern void LCD_scrollDisplayRight(DisplayProxy *lcd_config);
extern void LCD_printLeft(DisplayProxy *lcd_config);
extern void LCD_printRight(DisplayProxy *lcd_config);
extern void LCD_leftToRight(DisplayProxy *lcd_config);
extern void LCD_rightToLeft(DisplayProxy *lcd_config);
extern void LCD_shiftIncrement(DisplayProxy *lcd_config);
extern void LCD_shiftDecrement(DisplayProxy *lcd_config);
extern void LCD_noBacklight(DisplayProxy *lcd_config);
extern void LCD_backlight(DisplayProxy *lcd_config);
extern void LCD_autoscroll(DisplayProxy *lcd_config);
extern void LCD_noAutoscroll(DisplayProxy *lcd_config);
extern void LCD_createChar(DisplayProxy *lcd_config, uint8_t, uint8_t[]);
extern void LCD_setCursor(DisplayProxy *lcd_config, uint8_t, uint8_t);
extern void LCD_write(DisplayProxy *lcd_config, const char *msg, uint8_t size);

/* Lower level functions & private*/

extern void mutate(DisplayProxy *lcd_config, uint8_t value, uint8_t mode);

#endif

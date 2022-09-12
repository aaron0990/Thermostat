#include <LCDdisplayProxy.h>
#include <ti/drivers/dpl/HwiP.h>

DisplayProxy* LCD_create(void)
{
    DisplayProxy *lcd_config = (DisplayProxy*) malloc(sizeof(DisplayProxy));
    if (lcd_config != NULL)
    {
        lcd_config->displayI2C = DisplayI2C_create();
        //LCD_configure(lcd_config);
        //LCD_init(lcd_config);
    }
    return lcd_config;
}

void LCD_destroy(DisplayProxy *lcd_config)
{
    if (lcd_config != NULL)
    {
        free(lcd_config);
    }
}

void LCD_init(DisplayProxy *lcd_config)
{

    //lcd_config->displayI2C = DisplayI2C_create();
//mutate(lcd_config, LCD_FUNCTIONSET | lcd_config->displayfunction, 0);

// set the entry mode
    DisplayI2C_initialize(lcd_config->displayI2C);
    LCD_configure(lcd_config);
    LCD_clear(lcd_config);
    LCD_display(lcd_config);
    LCD_blink(lcd_config);
    LCD_home(lcd_config);

}

void LCD_configure(DisplayProxy *lcd_config)
{
    lcd_config->addr = LCD_ADDR;
    lcd_config->cols = 16;
    lcd_config->rows = 2;
    lcd_config->backlightval = LCD_BACKLIGHT;
    lcd_config->displayfunction = LCD_2LINE | LCD_4BITMODE | LCD_5x8DOTS; // set # lines, font size, etc.
    lcd_config->displaycontrol = LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF; // turn the display on with no cursor or blinking default
    lcd_config->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT; // Initialize to default text direction (for roman languages)
    lcd_config->numlines = lcd_config->rows;
}

void LCD_clear(DisplayProxy *lcd_config)
{
    mutate(lcd_config, LCD_CLEARDISPLAY, 0); // clear display, set cursor position to zero
    delay_us(2000);  // this command takes a long time!
}

void LCD_home(DisplayProxy *lcd_config)
{
    mutate(lcd_config, LCD_RETURNHOME, 0);  // set cursor position to zero
    delay_us(2000);  // this command takes a long time!
}

void LCD_setCursor(DisplayProxy *lcd_config, uint8_t col, uint8_t row)
{
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if (row > lcd_config->numlines)
    {
        row = (lcd_config->numlines) - 1;    // we count rows starting w/0
    }
    mutate(lcd_config, LCD_SETDDRAMADDR | (col + row_offsets[row]), 0);
}

// Turn the display on/off (quickly)
void LCD_noDisplay(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol &= ~LCD_DISPLAYON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}
void LCD_display(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol |= LCD_DISPLAYON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}

// Turns the underline cursor on/off
void LCD_noCursor(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol &= ~LCD_CURSORON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}
void LCD_cursor(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol |= LCD_CURSORON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}

// Turn on and off the blinking cursor
void LCD_noBlink(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol &= ~LCD_BLINKON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}
void LCD_blink(DisplayProxy *lcd_config)
{
    lcd_config->displaycontrol |= LCD_BLINKON;
    mutate(lcd_config, LCD_DISPLAYCONTROL | lcd_config->displaycontrol, 0);
}

// These commands scroll the display without changing the RAM
void LCD_scrollDisplayLeft(DisplayProxy *lcd_config)
{
    mutate(lcd_config, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, 0);
}
void LCD_scrollDisplayRight(DisplayProxy *lcd_config)
{
    mutate(lcd_config, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, 0);
}

// This is for text that flows Left to Right
void LCD_leftToRight(DisplayProxy *lcd_config)
{
    lcd_config->displaymode |= LCD_ENTRYLEFT;
    mutate(lcd_config, LCD_ENTRYMODESET | lcd_config->displaymode, 0);
}
// This is for text that flows Right to Left
void LCD_rightToLeft(DisplayProxy *lcd_config)
{
    lcd_config->displaymode &= ~LCD_ENTRYLEFT;
    mutate(lcd_config, LCD_ENTRYMODESET | lcd_config->displaymode, 0);
}

// This will 'right justify' text from the cursor
void LCD_autoscroll(DisplayProxy *lcd_config)
{
    lcd_config->displaymode |= LCD_ENTRYSHIFTINCREMENT;
    mutate(lcd_config, LCD_ENTRYMODESET | lcd_config->displaymode, 0);
}

// This will 'left justify' text from the cursor
void LCD_noAutoscroll(DisplayProxy *lcd_config)
{
    lcd_config->displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    mutate(lcd_config, LCD_ENTRYMODESET | lcd_config->displaymode, 0);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
/*void LCD_createChar(uint8_t location, uint8_t charmap[]) {
 location &= 0x7; // we only have 8 locations 0-7
 command(LCD_SETCGRAMADDR | (location << 3));
 for (int i=0; i<8; i++) {
 write(charmap[i]);
 }
 }*/

// Turn the (optional) backlight off/on
void LCD_noBacklight(DisplayProxy *lcd_config)
{
    lcd_config->backlightval = LCD_NOBACKLIGHT;
    mutate(lcd_config, LCD_NOBACKLIGHT, 0);
}

void LCD_backlight(DisplayProxy *lcd_config)
{
    lcd_config->backlightval = LCD_BACKLIGHT;
    mutate(lcd_config, LCD_BACKLIGHT, 0);
}

void LCD_write(DisplayProxy *lcd_config, const char *msg, uint8_t size)
{
    uint8_t ch_idx;
    for (ch_idx = 0; ch_idx < size; ++ch_idx)
    {
        mutate(lcd_config, msg[ch_idx], Rs);
    }
}

/************ low level data pushing commands & private methods **********/

void mutate(DisplayProxy *lcd_config, uint8_t value, uint8_t mode)
{
    //Disable task switching while sending

    uint8_t highnib = value & 0xf0;
    uint8_t lownib = (value << 4) & 0xf0;

    DisplayI2C_send(lcd_config->displayI2C,
                    (int) ((highnib) | mode) | lcd_config->backlightval);
    DisplayI2C_send(lcd_config->displayI2C,
                    (int) (((highnib) | mode) | En) | lcd_config->backlightval); // En high
    delay_us(50);                  // enable pulse must be >450ns
    DisplayI2C_send(
            lcd_config->displayI2C,
            (int) (((highnib) | mode) & ~En) | lcd_config->backlightval); // En low
    delay_us(100);                 // commands need > 37us to settle

    DisplayI2C_send(lcd_config->displayI2C,
                    (int) ((lownib) | mode) | lcd_config->backlightval);
    DisplayI2C_send(lcd_config->displayI2C,
                    (int) (((lownib) | mode) | En) | lcd_config->backlightval); // En high
    delay_us(50);                  // enable pulse must be >450ns
    DisplayI2C_send(lcd_config->displayI2C,
                    (int) (((lownib) | mode) & ~En) | lcd_config->backlightval); // En low
    delay_us(100);


}


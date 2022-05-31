/*
 * utils.c
 *
 *  Created on: 10 feb. 2022
 *      Author: aaron
 */

#include "utils.h"

void delay(uint32_t sec)
{
    uint32_t clk_freq = CS_getMCLK();
    uint32_t count = clk_freq * sec;
    Timer32_setCount(TIMER32_BASE, count);
    Timer32_startTimer(TIMER32_BASE, true); //Start timer in one-shot mode
    while (Timer32_getValue(TIMER32_BASE))
    {
    }
}

void delay_us(uint32_t usec)
{
    uint32_t clk_freq = CS_getMCLK();
    uint32_t count = ((uint32_t) (clk_freq / 1000000)) * usec;
    Timer32_setCount(TIMER32_BASE, count);
    Timer32_startTimer(TIMER32_BASE, true); //Start timer in one-shot mode
    while (Timer32_getValue(TIMER32_BASE))
    {
    }
}

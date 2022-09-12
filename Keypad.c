/*
 * Keypad.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include "Keypad.h"


void Keypad_init(Keypad *const me)
{
    /* Initialize + and - buttons to control target temperature*/
    GPIO_setConfig(INC_TEMP_PIN_IDX, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(DEC_TEMP_PIN_IDX, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setCallback(INC_TEMP_PIN_IDX, Keypad_InterruptHandler);
    GPIO_setCallback(DEC_TEMP_PIN_IDX, Keypad_InterruptHandler);
    GPIO_enableInt(INC_TEMP_PIN_IDX);
    GPIO_enableInt(DEC_TEMP_PIN_IDX);
}

void Keypad_clean(Keypad *const me)
{

}

void Keypad_readTemp(Keypad *const me)
{

}

void Keypad_newData(Keypad *const me)
{

}

Keypad* Keypad_create(void)
{
    Keypad *me = (Keypad*) malloc(sizeof(Keypad));
    if (me != NULL)
    {
        //Keypad_init(me);
    }
    return me;
}

void Keypad_destroy(Keypad *const me)
{
    if (me != NULL)
    {
        Keypad_clean(me);
    }
    free(me);
}



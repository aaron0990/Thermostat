/*
 * Keypad.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include "Keypad.h"

Keypad *kp;

void Keypad_init(Keypad *const me)
{
    me->itsTargetTemp = TempData_create();
    kp = me;

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
    if (me->itsTargetTemp != NULL)
    {
        me->itsTargetTemp = NULL;
    }
}

void Keypad_readTemp(Keypad *const me)
{
    Keypad_newData(me);
}

void Keypad_newData(Keypad *const me)
{
    if (!me->itsTargetTemp)
    {
        me->itsTargetTemp = TempData_create();
    }
    me->itsTargetTemp->humidity = me->itsTargetTemp->humidity;
    me->itsTargetTemp->temperature = me->itsTargetTemp->temperature;
}

Keypad* Keypad_create(void)
{
    Keypad *me = (Keypad*) malloc(sizeof(Keypad));
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

void *keypadThread(void *arg0){
    struct keypadThreadArgs *args = (struct keypadThreadArgs*) arg0;
    Keypad *me = (Keypad*) malloc(sizeof(Keypad));
    me->qDispConsole = args->qDispConsoleArg;
    while(1){
        sched_yield();
    }
}

/********************************* Interrupt handler **********************************************/

void Keypad_InterruptHandler(uint_least8_t idx)
{
    GPIO_clearInt(idx);
    GPIO_disableInt(idx);
    if (idx == INC_TEMP_PIN_IDX)
    {
        ++kp->itsTargetTemp->temperature;
    }
    if (idx == DEC_TEMP_PIN_IDX)
    {
        --kp->itsTargetTemp->temperature;
    }
    GPIO_enableInt(idx);


}

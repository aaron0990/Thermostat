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

#pragma CODE_SECTION(keypadThread, ".TI.ramfunc")
void *keypadThread(void *arg0){
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    struct keypadThreadArgs *args = (struct keypadThreadArgs*) arg0;
    Keypad *me = Keypad_create();
    me->qDispConsole = args->qDispConsoleArg;
    me->qKeypadToTCtrl = args->qKeypadToTCtrlArg;
    Keypad_init(me);
    kp = me;
    Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    while(1){
        sleep(10);
        sched_yield();
    }
}

/********************************* Interrupt handler **********************************************/

void Keypad_InterruptHandler(uint_least8_t idx)
{
    GPIO_clearInt(idx);
    GPIO_disableInt(idx);
    KeypadMsg kp_msg;
    if (idx == INC_TEMP_PIN_IDX)
    {
        kp_msg.cmd = INC_TARGET_T;
        xQueueSendFromISR(kp->qKeypadToTCtrl, &kp_msg, NULL);
    }
    if (idx == DEC_TEMP_PIN_IDX)
    {
        kp_msg.cmd = DEC_TARGET_T;
        xQueueSendFromISR(kp->qKeypadToTCtrl, &kp_msg, NULL);
    }
    GPIO_enableInt(idx);
}

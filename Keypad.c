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
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        me->itsKeypadNH[pos] = NULL;
    }

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

void Keypad_dumpList(Keypad *const me)
{
    //TODO: shows a list of subscribed clients (its id, instance pointer and acceptor function pointer)
}

void Keypad_newData(Keypad *const me)
{
    if (!me->itsTargetTemp)
    {
        me->itsTargetTemp = TempData_create();
    }
    me->itsTargetTemp->humidity = me->itsTargetTemp->humidity;
    me->itsTargetTemp->temperature = me->itsTargetTemp->temperature;
    Keypad_notify(me);
}

void Keypad_notify(Keypad *const me)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsKeypadNH[pos])
        {
            if (me->itsKeypadNH[pos]->acceptorPtr)
            {
                me->itsKeypadNH[pos]->acceptorPtr(
                        me->itsKeypadNH[pos]->instancePtr, me->itsTargetTemp);
            }
        }
    }
}

void Keypad_subscribe(Keypad *const me, void *instancePtr,
                      const tempDataAcceptorPtr aPtr)
{
    KeypadNotificationHandle *tnh;
    tnh = KeypadNotificationHandle_create();
    tnh->instancePtr = instancePtr;
    tnh->acceptorPtr = aPtr;
    Keypad_addItsKeypadNH(me, tnh);
}

void Keypad_unsubscribe(Keypad *const me, const tempDataAcceptorPtr aPtr)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsKeypadNH[pos])
        {
            if (me->itsKeypadNH[pos]->acceptorPtr == aPtr)
            {
                KeypadNotificationHandle_destroy(me->itsKeypadNH[pos]);
                me->itsKeypadNH[pos] = NULL;
            }
        }
    }
}

struct TempData* Keypad_getItsTempData(const Keypad *const me)
{
    return (struct TempData*) me->itsTargetTemp;
}

void Keypad_setItsTempData(Keypad *const me, struct TempData *p_tempData)
{
    me->itsTargetTemp = p_tempData;
}

int Keypad_getItsKeypadNH(const Keypad *const me)
{
    //TODO: idk
    return 0;
}

void Keypad_addItsKeypadNH(
        Keypad *const me,
        struct KeypadNotificationHandle *p_KeypadNotificationHandle)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (!me->itsKeypadNH[pos])
        {
            me->itsKeypadNH[pos] = p_KeypadNotificationHandle;
            break;
        }
    }
}

void Keypad_removeItsKeypadNH(
        Keypad *const me,
        struct KeypadNotificationHandle *p_KeypadNotificationHandle)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsKeypadNH[pos] == p_KeypadNotificationHandle)
        {
            me->itsKeypadNH[pos] = NULL;
            break;
        }
    }
}

void Keypad_clearItsKeypadNH(Keypad *const me)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        me->itsKeypadNH[pos] = NULL;
    }
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
    Keypad_notify(kp);
    GPIO_enableInt(idx);


}

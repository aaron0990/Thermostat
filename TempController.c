/*
 * TempController.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include "TempController.h"

void TempController_init(TempController *const me, TempSensor *const ts, Keypad *const kp)
{
    me->itsTempSensor = ts;
    me->itsKeypad = kp;
    me->readTemp = NULL;
    me->targetTemp = 0;
}

void TempController_clean(TempController *const me)
{
    if (me->itsTempSensor != NULL)
    {
        me->itsTempSensor = NULL;
    }
    if (me->readTemp != NULL)
    {
        me->readTemp = NULL;
    }
}

void TempController_acceptSensedTemp(TempController *const me, TempData *sensedTemp)
{
    if (!me->readTemp)
    {
        me->readTemp = TempData_create();
    }
    if (me->readTemp)
    {
        me->readTemp->temperature = sensedTemp->temperature;
        me->readTemp->humidity = sensedTemp->humidity;
    }
}

void TempController_acceptTargetTemp(TempController *const me, TempData *tgtTemp)
{
    if (!me->readTemp)
    {
        me->readTemp = TempData_create();
    }
    if (me->readTemp)
    {
        me->targetTemp->temperature = tgtTemp->temperature;
        me->targetTemp->humidity = tgtTemp->humidity;
    }
}

TempController* TempController_create(void)
{
    TempController *me = (TempController*) malloc(sizeof(TempController));
    return me;
}

void TempController_destroy(TempController *const me)
{
    if (me != NULL)
    {
        TempController_clean(me);
    }
    free(me);
}

TempData* TempController_getReadTemp(TempController *const me)
{
    return (TempData*) me->readTemp;
}
void TempController_setReadTemp(TempController *const me, TempData *p_td)
{
    me->readTemp = p_td;
}
TempSensor* TempController_getItsTempSensor(TempController *const me)
{
    return (TempSensor*) me->itsTempSensor;
}
void TempController_setItsTempSensor(TempController *const me, TempSensor *p_ts)
{
    me->itsTempSensor = p_ts;
}
void TempController_setTargetTemp(TempController* const me, TempData* tgtTmp)
{
    me->targetTemp = tgtTmp;
}

void temperatureControllerThread(void *arg0){
    struct temperatureControllerThreadArgs *args = (struct temperatureControllerThreadArgs*) arg0;
    TempController *me = (TempController*) malloc(sizeof(TempController));
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToTCtrl = args->qTReadToTCtrlArg;
    while(1){
        sched_yield();
    }

}


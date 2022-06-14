/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "TempSensor.h"

void TempSensor_init(TempSensor *const me)
{
    me->itsTempSensorProxy = TempSensorProxy_create();
}

void TempSensor_readTemp(TempSensor *const me)
{
    TempSensorProxy_access(me->itsTempSensorProxy);
    xQueueSend(me->qTReadToLCD, (const void *)(me->itsTempSensorProxy->itsTempData), 0);
    //Insert new data to queue
}


TempSensor* TempSensor_create(TempSensor *const me)
{

    if (me != NULL)
    {
        TempSensor_init(me);
    }
    return me;
}

void TempSensor_destroy(TempSensor *const me)
{
    if (me != NULL)
    {
        TempSensor_clean(me);
    }
    free(me);
}

void temperatureReadingThread(void *arg0){
    struct temperatureReadingThreadArgs *args = (struct temperatureReadingThreadArgs*) arg0;
    TempSensor *me = (TempSensor*) malloc(sizeof(TempSensor));
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToLCD = args->qTReadToLCDArg;
    me->qTReadToTCtrl = args->qTReadToTCtrlArg;

    TempSensor_init(me);
    while(1){
        TempSensor_readTemp(me);
        sched_yield();
        sleep(15);
    }
}

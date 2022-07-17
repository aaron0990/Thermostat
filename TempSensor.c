/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include <stdio.h>
#include <string.h>
#include "TempSensor.h"

void TempSensor_init(TempSensor *const me)
{
    me->itsTempSensorProxy = TempSensorProxy_create();
}

void TempSensor_readTemp(TempSensor *const me)
{
    TempSensorProxy_access(me->itsTempSensorProxy); //Read temperature
    xQueueSend(me->qTReadToLCD, (void *)(me->itsTempSensorProxy->itsTempData), 0); //Push data to LCD queue

    //Print to console
    char buff[128];
    sprintf(buff, "T: %.1fºC, H: %.1f%% \n", me->itsTempSensorProxy->itsTempData->temperature,
                                         me->itsTempSensorProxy->itsTempData->humidity); //whitespaces to clear line
    DisplayConsoleMsg msg;
    msg.buff = buff;
    msg.len = strlen(buff);
    xQueueSend(me->qDispConsole, (void *)&msg, 0);

    TempData td;
    td.temperature = me->itsTempSensorProxy->itsTempData->temperature;
    td.humidity = me->itsTempSensorProxy->itsTempData->humidity;
    xQueueSend(me->qTReadToTCtrl, (void *)&td, 0);
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

void *temperatureReadingThread(void *arg0){
    struct temperatureReadingThreadArgs *args = (struct temperatureReadingThreadArgs*) arg0;
    TempSensor *me = (TempSensor*) malloc(sizeof(TempSensor));
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToLCD = args->qTReadToLCDArg;
    me->qTReadToTCtrl = args->qTReadToTCtrlArg;
    TempSensor_init(me);

    while(1){
        TempSensor_readTemp(me);
        //sched_yield();
        sleep(3);

    }
}

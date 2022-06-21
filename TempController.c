/*
 * TempController.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include "TempController.h"

void TempController_init(TempController *const me, TempSensor *const ts,
                         Keypad *const kp)
{

}

void TempController_clean(TempController *const me)
{

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


void* temperatureControllerThread(void *arg0)
{
    struct temperatureControllerThreadArgs *args =
            (struct temperatureControllerThreadArgs*) arg0;
    TempController *me = (TempController*) malloc(sizeof(TempController));
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToTCtrl = args->qTReadToTCtrlArg;
    me->qKeypadToTCtrl = args->qKeypadToTCtrl;

    TempData tSensed; //where to store the dequed elem
    KeypadMsg kpMsg;
    while (1)
    {
        if (xQueueReceive(me->qTReadToTCtrl, &tSensed, 0))
        {
            //New temperature reading available
            me->readTemp = tSensed;
        }
        if (xQueueReceive(me->qKeypadToTCtrl, &kpMsg, 0))
        {
            //New temperature reading available
            //me->readTemp = tSensed;
        }
        sched_yield();
        sleep(1);
    }

}


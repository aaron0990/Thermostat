/*
 * TempController.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include <gpio.h>
#include "TempController.h"

void TempController_init(TempController *const me)
{
    me->targetTemp.temperature = 21.0;
    me->targetTemp.humidity = 0.0;

    me->readTemp.temperature = 0.0;
    me->readTemp.humidity = 0.0;

    me->heatingOn = 1; //Heating is ON when system starts up.

    //Relay control pin
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);

    xQueueSend(me->qTCtrlToLCD, (void* )&me->targetTemp, 0);
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
    me->qKeypadToTCtrl = args->qKeypadToTCtrlArg;
    me->qTCtrlToLCD = args->qTCtrlToLCDArg;
    TempController_init(me);

    TempData tSensed;   //to store the dequed elem
    KeypadMsg kpMsg;    //to store the dequed elem
    while (1)
    {
        if (xQueueReceive(me->qTReadToTCtrl, &tSensed, 0))
        {
            //New temperature reading available
            me->readTemp = tSensed;
        }
        if (xQueueReceive(me->qKeypadToTCtrl, &kpMsg, 0))
        {
            //New target temperature available
            switch (kpMsg.cmd)
            {
            case INC_TARGET_T: //Increment target temperature value
                me->targetTemp.temperature += 0.5;
                break;
            case DEC_TARGET_T: //Decrement target temperature value
                me->targetTemp.temperature -= 0.5;
                break;
            }
            xQueueSend(me->qTCtrlToLCD, (void* )&me->targetTemp, 0);

        }
        if (me->targetTemp.temperature > me->readTemp.temperature)
        {
            //Heating has to be turned ON
            if (me->heatingOn == 0)
            {
                //If heating is OFF, turn it ON
                GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
                me->heatingOn = 1;
            }
            //else, do nothing because it was already ON
        }
        else
        {
            //Heating has to be turned OFF
            if (me->heatingOn == 1)
            {
                //If heating is ON, turn it OFF
                GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
                me->heatingOn = 0;
            }
        }
        sched_yield();
    }

}


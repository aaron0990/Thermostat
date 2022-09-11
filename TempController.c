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

    //Relay control pin
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);

    me->heatingOn = 1; //Heating is ON when system starts up.
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
}

void TempController_clean(TempController *const me)
{

}

TempController* TempController_create(void)
{
    TempController *me = (TempController*) malloc(sizeof(TempController));
    if (me != NULL)
    {
        TempController_init(me);
    }
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

void TempController_setReadTemp(TempController *const me, TempData* td)
{
    if (me != NULL)
    {
        me->readTemp = *td;
    }
}

void TempController_setTargetTemp(TempController *const me, TempData* td)
{
    if (me != NULL)
    {
        me->targetTemp = *td;
    }
}

void TempController_updateHeatingState(TempController *const me)
{
    if (me->readTemp.temperature != 0.0)
    {
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
    }
}

#pragma CODE_SECTION(temperatureControllerThread, ".TI.ramfunc")
void* temperatureControllerThread(void *arg0)
{
    TempData tSensed;   //to store the dequed elem
    KeypadMsg kpMsg;    //to store the dequed elem

    Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    while (1)
    {
        //if (xQueueReceive(me->qKeypadToTCtrl, &kpMsg, MAX_WAIT_QUEUE))

            /*
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
        }

        sched_yield();*/
    }

}


/*
 * TempController.c
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#include <gpio.h>
#include "TempController.h"

void TempController_init(TempController *const me, TempData* readTemp, TempData* targetTemp)
{
    me->readTemp = readTemp;
    me->targetTemp = targetTemp;

    //Relay control pin
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    //Relay GND
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

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
        me->readTemp = td;
    }
    TempController_updateHeatingState(me);
}

void TempController_setTargetTemp(TempController *const me, TempData* td)
{
    if (me != NULL)
    {
        me->targetTemp = td;
    }
    TempController_updateHeatingState(me);
}

void TempController_updateHeatingState(TempController *const me)
{
    if (me->readTemp->temperature != 0.0)
    {
        if (me->targetTemp->temperature > me->readTemp->temperature)
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



/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "TempSensor.h"


void TempSensor_init(TempSensor *const me, TempData* readTemp)
{
    TempSensorProxy_init(me->itsTempSensorProxy, readTemp);
}

//#pragma CODE_SECTION(TempSensor_readTemp, ".TI.ramfunc")
void TempSensor_readTemp(TempSensor *const me)
{
    TempSensorProxy_access(me->itsTempSensorProxy); //Read temperature
}

void TempSensor_configure(TempSensor *const me, TempData* readTemp)
{
    TempSensorProxy_configure(me->itsTempSensorProxy);
}

TempSensor* TempSensor_create()
{
    TempSensor* me = (TempSensor*) malloc(sizeof(TempSensor));
    if (me != NULL)
    {
        me->itsTempSensorProxy = TempSensorProxy_create();
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

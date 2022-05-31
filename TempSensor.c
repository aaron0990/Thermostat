/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "TempSensor.h"

void TempSensor_init(TempSensor *const me)
{
    me->itsTempData = TempData_create();
    me->itsTempSensorProxy = TempSensorProxy_create();

    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        me->itsTempNH[pos] = NULL;
    }
}

void TempSensor_clean(TempSensor *const me)
{
    if (me->itsTempData != NULL)
    {
        me->itsTempData = NULL;
    }
}

void TempSensor_readTemp(TempSensor *const me)
{
    TempSensorProxy_access(me->itsTempSensorProxy);
    TempSensor_newData(me);
}

void TempSensor_dumpList(TempSensor *const me)
{
    //TODO: shows a list of subscribed clients (its id, instance pointer and acceptor function pointer)
}

void TempSensor_newData(TempSensor *const me)
{
    if (!me->itsTempData)
    {
        me->itsTempData = TempData_create();
    }
    me->itsTempData->humidity = me->itsTempSensorProxy->itsTempData->humidity;
    me->itsTempData->temperature =
            me->itsTempSensorProxy->itsTempData->temperature;
    TempSensor_notify(me);
}

void TempSensor_notify(TempSensor *const me)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsTempNH[pos])
        {
            if (me->itsTempNH[pos]->acceptorPtr)
            {
                me->itsTempNH[pos]->acceptorPtr(me->itsTempNH[pos]->instancePtr,
                                                me->itsTempData);
            }
        }
    }
}

void TempSensor_subscribe(TempSensor *const me, void *instancePtr,
                          const tempDataAcceptorPtr aPtr)
{
    struct TempNotificationHandle *tnh;
    tnh = TempNotificationHandle_create();
    tnh->instancePtr = instancePtr;
    tnh->acceptorPtr = aPtr;
    TempSensor_addItsTempNH(me, tnh);
}

void TempSensor_unsubscribe(TempSensor *const me,
                            const tempDataAcceptorPtr aPtr)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsTempNH[pos])
        {
            if (me->itsTempNH[pos]->acceptorPtr == aPtr)
            {
                TempNotificationHandle_destroy(me->itsTempNH[pos]);
                me->itsTempNH[pos] = NULL;
            }
        }
    }
}

struct TempData* TempSensor_getItsTempData(const TempSensor *const me)
{
    return (struct TempData*) me->itsTempData;
}

void TempSensor_setItsTempData(TempSensor *const me,
                               struct TempData *p_tempData)
{
    me->itsTempData = p_tempData;
}

int TempSensor_getItsTempNH(const TempSensor *const me)
{
    //TODO: idk
    return 0;
}

void TempSensor_addItsTempNH(
        TempSensor *const me,
        struct TempNotificationHandle *p_tempNotificationHandle)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (!me->itsTempNH[pos])
        {
            me->itsTempNH[pos] = p_tempNotificationHandle;
            break;
        }
    }
}

void TempSensor_removeItsTempNH(
        TempSensor *const me,
        struct TempNotificationHandle *p_tempNotificationHandle)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        if (me->itsTempNH[pos] == p_tempNotificationHandle)
        {
            me->itsTempNH[pos] = NULL;
            break;
        }
    }
}

void TempSensor_clearItsTempNH(TempSensor *const me)
{
    int pos;
    for (pos = 0; pos < MAX_SUBSCRIBERS; ++pos)
    {
        me->itsTempNH[pos] = NULL;
    }
}

TempSensor* TempSensor_create(void)
{
    TempSensor *me = (TempSensor*) malloc(sizeof(TempSensor));
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

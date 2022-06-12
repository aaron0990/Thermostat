/*
 * TempSensor.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_

#define MAX_SUBSCRIBERS 100

#include "TempData.h"
#include "TempSensorProxy.h"
#include "TempNotificationHandle.h"
#include <string.h>
#include <timer_a.h>
#include <interrupt.h>
#include <gpio.h>
#include <stdlib.h>
#include "shared_vars.h"

#define TEMP_UPDATE_INT 15 //seconds

/*Class TempSensor*/
typedef struct TempSensor TempSensor;
struct TempSensor
{
    TempData *itsTempData;
    TempNotificationHandle *itsTempNH[MAX_SUBSCRIBERS];
    TempSensorProxy *itsTempSensorProxy;
};

/*Constructors and destructors*/
void TempSensor_init(TempSensor *const me);
void TempSensor_clean(TempSensor *const me);

/*Operations*/
void TempSensor_dumpList(TempSensor *const me);
void TempSensor_readTemp(TempSensor *const me);
void TempSensor_newData(TempSensor *const me);
void TempSensor_notify(TempSensor *const me);
void TempSensor_subscribe(TempSensor *const me, void *instancePtr,
                          const tempDataAcceptorPtr aPtr);
void TempSensor_unsubscribe(TempSensor *const me,
                            const tempDataAcceptorPtr aPtr);
struct TempData* TempSensor_getItsTempData(const TempSensor *const me);
void TempSensor_setItsTempData(TempSensor *const me,
                               struct TempData *p_tempData);
int TempSensor_getItsTempNH(const TempSensor *const me);
void TempSensor_addItsTempNH(
        TempSensor *const me,
        TempNotificationHandle *p_tempNotificationHandle);
void TempSensor_removeItsTempNH(
        TempSensor *const me,
        struct TempNotificationHandle *p_tempNotificationHandle);
void TempSensor_clearItsTempNH(TempSensor *const me);
TempSensor* TempSensor_create(void);
void TempSensor_destroy(TempSensor *const me);

//TODO: Check aPtr argument in subscribe() and unsubscribe() functions. I'm not sure if the type is tempDataAcceptorPtr or tempDataAcceptorPtr*
#endif /* TEMPSENSOR_H_ */

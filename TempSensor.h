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
#include <string.h>
#include <timer_a.h>
#include <interrupt.h>
#include <gpio.h>
#include <stdlib.h>
#include "shared_vars.h"
#include "TempReadToLCDQueue.h"
#include "TempReadToTempCtrlQueue.h"
#include "ThreadsArgStruct.h"

#define TEMP_UPDATE_INT 15 //seconds

/*Class TempSensor*/
typedef struct TempSensor TempSensor;
struct TempSensor
{
    QueueHandle_t qDispConsole;
    QueueHandle_t qTReadToLCD;
    QueueHandle_t qTReadToTCtrl;
    TempSensorProxy *itsTempSensorProxy;
};

/*Constructors and destructors*/
void TempSensor_init(TempSensor *const me);
void TempSensor_clean(TempSensor *const me);

/*Operations*/
void TempSensor_readTemp(TempSensor *const me);
void TempSensor_newData(TempSensor *const me);
TempSensor* TempSensor_create(TempSensor *const me);
void TempSensor_destroy(TempSensor *const me);

//TODO: Check aPtr argument in subscribe() and unsubscribe() functions. I'm not sure if the type is tempDataAcceptorPtr or tempDataAcceptorPtr*
#endif /* TEMPSENSOR_H_ */

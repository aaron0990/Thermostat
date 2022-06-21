/*
 * LCDdisplayClient.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef LCDDISPLAYCLIENT_H_
#define LCDDISPLAYCLIENT_H_
#include "TempSensor.h"
#include "TempData.h"
#include <stdio.h>
#include "LCDdisplayProxy.h"
#include "shared_vars.h"
#include "InterThreadQueues.h"
#include "Keypad.h"
#include "ThreadsArgStruct.h"

typedef struct DisplayClient DisplayClient;

struct DisplayClient{
    TempSensor* itsTempSensor;
    TempData* itsTempSensed;
    TempData* itsTempTarget;
    Keypad* itsKeypad;
    DisplayProxy* itsDisplayProxy;

    //These ones are necessary
    QueueHandle_t qDispConsole;
    QueueHandle_t qTReadToLCD;
};

void DisplayClient_init(DisplayClient *const me, TempSensor *const ts, Keypad *const kp);
void DisplayClient_clean(DisplayClient* const me);
void DisplayClient_acceptTempSensed(DisplayClient* const me, TempData* td);
void DisplayClient_acceptTempTarget(DisplayClient* const me, TempData* td);
void DisplayClient_register(DisplayClient* const me); //it calls the subscribe() function of the sensor
void DisplayClient_showTempSensed(DisplayClient* const me);
void DisplayClient_showTempTarget(DisplayClient *const me);
DisplayClient* DisplayClient_create(void);
void DisplayClient_destroy(DisplayClient* const me);

//Getters and setters
TempData* DisplayClient_getItsTempSensed(DisplayClient* const me);
void DisplayClient_setItsTempSensed(DisplayClient* const me, TempData* p_td);
TempSensor* DisplayClient_getItsTempSensor(DisplayClient* const me);
void DisplayClient_setItsTempSensor(DisplayClient* const me, TempSensor* p_ts);

#endif /* LCDDISPLAYCLIENT_H_ */

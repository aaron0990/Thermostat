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

#define LCD_ON_BACKLIGHT_T  5000 //ms

typedef struct DisplayClient DisplayClient;

struct DisplayClient{
    TempData itsTempSensed;
    TempData itsTempTarget;
    DisplayProxy* itsDisplayProxy;

    //These ones are necessary
    QueueHandle_t qDispConsole;
    QueueHandle_t qTReadToLCD;
    QueueHandle_t qTCtrlToLCD;
};

void DisplayClient_init(DisplayClient *const me);
void DisplayClient_clean(DisplayClient* const me);
void DisplayClient_acceptTempSensed(DisplayClient* const me, TempData* td);
void DisplayClient_acceptTempTarget(DisplayClient* const me, TempData* td);
void DisplayClient_register(DisplayClient* const me); //it calls the subscribe() function of the sensor
void DisplayClient_showInfo(DisplayClient *const me);
DisplayClient* DisplayClient_create(void);
void DisplayClient_destroy(DisplayClient* const me);

//Getters and setters
TempData* DisplayClient_getItsTempSensed(DisplayClient* const me);
void DisplayClient_setItsTempSensed(DisplayClient* const me, TempData* p_td);
TempSensor* DisplayClient_getItsTempSensor(DisplayClient* const me);
void DisplayClient_setItsTempSensor(DisplayClient* const me, TempSensor* p_ts);

#endif /* LCDDISPLAYCLIENT_H_ */

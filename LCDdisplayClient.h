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
#include <timers.h>
#include "LCDdisplayProxy.h"
#include "shared_vars.h"
#include "Keypad.h"

#define LCD_ON_BACKLIGHT_T  5 //s

//Flags used to indicate why the thread is awake
#define SLEEP               0x00
#define PRINT_DATA          0x01
#define OFF_BACKLIGHT       0x02

typedef struct DisplayClient DisplayClient;

struct DisplayClient{
    TempData* itsTempSensed;
    TempData* itsTempTarget;
    DisplayProxy* itsDisplayProxy;
    uint32_t nextBacklightOffTime;
    uint32_t backlightOnDuration;
    uint8_t flags;
};

void DisplayClient_init(DisplayClient *const me, TempData* readTemp, TempData* targetTemp);
void DisplayClient_clean(DisplayClient* const me);
void DisplayClient_acceptTempSensed(DisplayClient* const me, TempData* td);
void DisplayClient_acceptTempTarget(DisplayClient* const me, TempData* td);
void DisplayClient_register(DisplayClient* const me); //it calls the subscribe() function of the sensor
void DisplayClient_showInfo(DisplayClient *const me);
DisplayClient* DisplayClient_create(void);
void DisplayClient_updateNextBacklightOffTime(DisplayClient *const me, uint32_t currentTime);
void DisplayClient_turnOffLCDbacklight(DisplayClient *const me);

void DisplayClient_destroy(DisplayClient* const me);

//Getters and setters
TempData* DisplayClient_getItsTempSensed(DisplayClient* const me);
void DisplayClient_setItsTempSensed(DisplayClient* const me, TempData* p_td);
TempData* DisplayClient_getItsTempSensed(DisplayClient* const me);
void DisplayClient_setItsTempSensed(DisplayClient* const me, TempData* p_td);
TempSensor* DisplayClient_getItsTempSensor(DisplayClient* const me);
void DisplayClient_setItsTempSensor(DisplayClient* const me, TempSensor* p_ts);

#endif /* LCDDISPLAYCLIENT_H_ */

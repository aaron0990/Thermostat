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


#define LCD_ON_BACKLIGHT_T  60 //s

// Events
typedef enum
{
    PRINT_DATA,
    OFF_BACKLIGHT,
} DCEventType;

typedef struct
{
    DCEventType eventType;
    char* textR1;   //Text to be printed in LCD's row 1
    uint8_t textR1Length; //length of textR1 (max 16)
    char* textR2;   //Text to be printed in LCD's row 2
    uint8_t textR2Length; //length of textR2 (max 16)
} DCEvent;

typedef struct DisplayClient DisplayClient;

struct DisplayClient{
    DisplayProxy* itsDisplayProxy;
    uint32_t nextBacklightOffTime;
    uint32_t backlightOnDuration;
};

extern QueueHandle_t displayClientEventQueue; //Queue used to pass events to the DisplayClient.

void DisplayClient_init(DisplayClient *const me);
void DisplayClient_clean(DisplayClient* const me);
void DisplayClient_register(DisplayClient* const me); //it calls the subscribe() function of the sensor
void DisplayClient_showInfo(DisplayClient *const me, DCEvent* event);
DisplayClient* DisplayClient_create(void);
void DisplayClient_updateNextBacklightOffTime(DisplayClient *const me, uint32_t currentTime);
void DisplayClient_turnOffLCDbacklight(DisplayClient *const me);

void DisplayClient_destroy(DisplayClient* const me);

#endif /* LCDDISPLAYCLIENT_H_ */

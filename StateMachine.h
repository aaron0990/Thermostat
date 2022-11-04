/*
 * StateMachine.h
 *
 *  Created on: 1 nov. 2022
 *      Author: aaron
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

/*Application includes*/
#include "LCDdisplayClient.h"
#include "TempController.h"
#include "TempSensor.h"
#include "RTC.h"
#include <stdio.h>
#include "shared_vars.h"
#include "Keypad.h"
#include "TempData.h"
#include "DS3231.h"

/* States */
typedef enum
{
    IDLE_STATE,
    DISPLAY_SET_DATE_STATE,
    DISPLAY_SET_PROG_STATE,
    SET_DOW_STATE,
    SET_HOUR_STATE,
    SET_MINUTES_STATE,
    PROG_STATE,
    PROG_SET_DOW_STATE,
    PROG_SET_TIME_SLOT_STATE,
    PROG_SET_SETPOINT_STATE,
    PROG_SET_START_HOUR_STATE,
    PROG_SET_START_MINUTES_STATE,
    PROG_SET_END_HOUR_STATE,
    PROG_SET_END_MINUTES_STATE,
} SMState;

// Events
typedef enum
{
  INC_BTN_PRESSED,
  DEC_BTN_PRESSED,
  MODE_BTN_PRESSED,
  OK_BTN_PRESSED,
} SMEventType;

typedef struct
{
    SMEventType eventType;
} SMEvent;

extern QueueHandle_t stateMachineEventQueue; //Queue used to pass events to the FSM.

#endif /* STATEMACHINE_H_ */

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
#include <stdarg.h>
#include <flash.h>

#define NUM_TIME_SLOTS_PER_DOW  5
#define DEFAULT_SETPOINT_TEMP   19.0

#define SCHEDULE_FLASH_START    0x0003F000 //Start address of last sector (31) from bank 1. Each sector is 4KB.

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
} SMState_t;

// Events
typedef enum
{
    INC_BTN_PRESSED, DEC_BTN_PRESSED, MODE_BTN_PRESSED, OK_BTN_PRESSED
} SMEventType_t;

typedef struct
{
    SMEventType_t eventType;
} SMEvent_t;

typedef enum
{
    MON, TUE, WED, THU, FRI, SAT, SUN, NUM_DOW
} dow_t;

typedef struct
{
    float setpointTemp;
    uint8_t startHour;
    uint8_t endHour;
    uint8_t startMin;
    uint8_t endMin;
} timeSlot_t;

typedef struct
{
    timeSlot_t timeSlot[NUM_TIME_SLOTS_PER_DOW];
} dow_schedule_t;

typedef struct
{
    dow_schedule_t dowSched[NUM_DOW];
} schedule_t;

typedef enum
{
    OFF, ON, NUM_PROG_STATES
} progState_t;

typedef struct
{
    SMEvent_t smEvent;
    SMState_t stateMachineCurrState;
    progState_t progState;
    uint8_t dowIdx;
    uint8_t timeSlotIdx;
    uint8_t hour;
    uint8_t minute;
    char startTime[6];
    char endTime[6];

    DCEvent dcEvt;
    char text1[16];
    char text2[16];
    //Vars used to set the current date and the scheduler
    schedule_t schedule;
} StateMachine;

extern const char *const daysOfWeek[NUM_DOW];
extern const char *const progStatus[NUM_PROG_STATES];

extern QueueHandle_t stateMachineEventQueue; //Queue used to pass events to the FSM.

void StateMachine_init(StateMachine* const me);
#endif /* STATEMACHINE_H_ */

/*
 * StateMachine.c
 *
 *  Created on: 1 nov. 2022
 *      Author: aaron
 */

#include "StateMachine.h"

extern TempController *tempController;
extern TempSensor *tempSensor;
extern DisplayClient *displayClient;
extern Keypad *keypad;
extern RTC *rtc;
extern TempData *readTemp;
extern TempData *targetTemp;
extern DS3231Proxy *ds3231hdl;

extern sem_t startStateMachine;

SMState_t *stateMachineCurrState;

const char *const daysOfWeek[NUM_DOW] = { "Lunes", "Martes", "Miercoles",
                                          "Jueves", "Viernes", "Sabado",
                                          "Domingo" };
const char *const progStatus[NUM_PROG_STATES] = { "OFF", "ON" };

#pragma DATA_SECTION(schedule, ".schedule_data");
#pragma DATA_ALIGN(schedule, 4);
schedule_t schedule;

void StateMachine_init(StateMachine *const me)
{
    //FlashCtl_setWaitState(FLASH_BANK0, 1);
    //FlashCtl_setWaitState(FLASH_BANK1, 1);
    //memcpy(&schedule, &me->schedule, sizeof(schedule_t));
    stateMachineCurrState = &me->stateMachineCurrState;
    me->stateMachineCurrState = IDLE_STATE;
    memset(&me->smEvent, 0, sizeof(SMEvent_t));
    me->dowIdx = 0;
    me->timeSlotIdx = 0;
    me->hour = 0;
    me->minute = 0;
    me->progState = OFF;

    FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);
    memcpy(&me->schedule, (void*) SCHEDULE_FLASH_START, sizeof(schedule_t));
    FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);

    /*memset(&me->schedule, 0, sizeof(schedule_t));
    int dow, ts;
    //Initialize default setpoint temperatures to 19.0ºC (to save clicks)
    for (dow = 0; dow < NUM_DOW; ++dow)
    {
        for (ts = 0; ts < NUM_TIME_SLOTS_PER_DOW; ++ts)
        {
            me->schedule.dowSched[dow].timeSlot[ts].setpointTemp =
            DEFAULT_SETPOINT_TEMP;
        }

    }*/
}

#pragma CODE_SECTION(stateMachineThread, ".TI.ramfunc")
void* stateMachineThread(void *arg0)
{
    sem_wait(&startStateMachine);
    StateMachine *me = malloc(sizeof(StateMachine));
    StateMachine_init(me);
    while (1)
    {
        //DO NOT DECLARE VARIABLES INSIDE while(1) SCOPE. A HARD FAULT IS RAISED OTHERWISE!
        xQueueReceive(stateMachineEventQueue, &me->smEvent, portMAX_DELAY);
        //Clear structures
        me->dcEvt.textR1 = NULL;
        me->dcEvt.textR1Length = 0;
        me->dcEvt.textR2 = NULL;
        me->dcEvt.textR2Length = 0;
        strcpy(me->text1, "");
        strcpy(me->text2, "");
        if (me->stateMachineCurrState == IDLE_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                targetTemp->temperature += 0.5;
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                TempController_updateHeatingState(tempController);
            }
            if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                targetTemp->temperature -= 0.5;
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                TempController_updateHeatingState(tempController);
            }
            if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                strcpy(me->text1, "Ajustes fecha");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_DATE_STATE;
            }
        }
        else if (me->stateMachineCurrState == DISPLAY_SET_DATE_STATE)
        {
            if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                strcpy(me->text1, "Ajustes Prog");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            if (me->smEvent.eventType == OK_BTN_PRESSED)
            {

                ds3231_read(ds3231hdl, DAY_OF_WEEK, &me->dowIdx);
                strcpy(me->text1, "Dia Semana");
                sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = SET_DOW_STATE;
            }
        }
        else if (me->stateMachineCurrState == DISPLAY_SET_PROG_STATE)
        {
            if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            if (me->smEvent.eventType == OK_BTN_PRESSED)
            {
                strcpy(me->text1, "Estado prog");
                sprintf(me->text2, "%s", progStatus[me->progState]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_STATE;
            }
        }
        else if (me->stateMachineCurrState == SET_DOW_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->dowIdx;
                me->dowIdx %= NUM_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
                strcpy(me->text1, "Dia Semana");
                sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);

            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->dowIdx;
                if (me->dowIdx > (NUM_DOW - 1))
                    me->dowIdx = NUM_DOW - 1; //If me->dowIdx is greater than 6, set it to 6 again.
                strcpy(me->text1, "Dia Semana");
                sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                strcpy(me->text1, "Ajustes Prog");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, DAY_OF_WEEK, &me->dowIdx); //Set DOW
                ds3231_read(ds3231hdl, HOUR, &me->hour); //Read me->hour
                strcpy(me->text1, "Horas");
                sprintf(me->text2, "%02d", me->hour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = SET_HOUR_STATE;
            }
        }
        else if (me->stateMachineCurrState == SET_HOUR_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->hour;
                me->hour %= 24; //If me->hour is greater than 23, set it to 0 again.
                strcpy(me->text1, "Horas");
                sprintf(me->text2, "%02d", me->hour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->hour;
                if (me->hour > 23)
                    me->hour = 23; //If me->hour is greater than 23, set it to 23 again.
                strcpy(me->text1, "Horas");
                sprintf(me->text2, "%02d", me->hour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                strcpy(me->text1, "Ajustes Prog");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, HOUR, &me->hour);
                ds3231_read(ds3231hdl, MINUTE, &me->minute); //Read me->hour
                strcpy(me->text1, "Minutos");
                sprintf(me->text2, "%02d", me->minute);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = SET_MINUTES_STATE;
            }
        }
        else if (me->stateMachineCurrState == SET_MINUTES_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->minute;
                me->minute %= 60; //If me->hour is greater than 59, set it to 0 again.
                strcpy(me->text1, "Minutos");
                sprintf(me->text2, "%02d", me->minute);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->minute;
                if (me->minute > 59)
                    me->minute = 59; //If me->hour is greater than 59, set it to 59 again.
                strcpy(me->text1, "Minutos");
                sprintf(me->text2, "%02d", me->minute);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                strcpy(me->text1, "Ajustes Prog");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, MINUTE, &me->minute);
                strcpy(me->text1, "Ajustes fecha");
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->stateMachineCurrState = DISPLAY_SET_DATE_STATE;
            }
        }
        else if (me->stateMachineCurrState == PROG_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->progState;
                me->progState %= NUM_PROG_STATES; //If me->dowIdx is greater than 1, set it to 0 again.
                strcpy(me->text1, "Estado prog");
                sprintf(me->text2, "%s", progStatus[me->progState]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->progState;
                if (me->progState > (NUM_PROG_STATES - 1))
                    me->progState = OFF; //If me->dowIdx is greater than 1, set it to 1 again.
                strcpy(me->text1, "Estado prog");
                sprintf(me->text2, "%s", progStatus[me->progState]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                if (me->progState == ON) //Schedule is enabled
                {
                    ds3231_read(ds3231hdl, DAY_OF_WEEK, &me->dowIdx);
                    strcpy(me->text1, "Dia Semana");
                    sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                    me->dcEvt.eventType = PRINT_DATA;
                    me->dcEvt.textR1 = me->text1;
                    me->dcEvt.textR1Length = strlen(me->text1);
                    me->dcEvt.textR2 = me->text2;
                    me->dcEvt.textR2Length = strlen(me->text2);
                    me->stateMachineCurrState = PROG_SET_DOW_STATE;

                }
                else    //Schedule is disabled
                {
                    strcpy(me->text1, "Ajustes Prog");
                    me->dcEvt.eventType = PRINT_DATA;
                    me->dcEvt.textR1 = me->text1;
                    me->dcEvt.textR1Length = strlen(me->text1);
                    me->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
                }
            }

        }
        else if (me->stateMachineCurrState == PROG_SET_DOW_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->dowIdx;
                me->dowIdx %= NUM_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
                strcpy(me->text1, "Dia Semana");
                sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);

            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->dowIdx;
                if (me->dowIdx > (NUM_DOW - 1))
                    me->dowIdx = NUM_DOW - 1; //If me->dowIdx is greater than 6, set it to 6 again.
                strcpy(me->text1, "Dia Semana");
                sprintf(me->text2, "%s", daysOfWeek[me->dowIdx]);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                me->timeSlotIdx = 0;

                sprintf(me->text1,
                        "F%d = %02u:%02u-%02u:%02u",
                        me->timeSlotIdx,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);

                sprintf(me->text2,
                        "%.1f'C",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);

                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_TIME_SLOT_STATE;
            }

        }
        else if (me->stateMachineCurrState == PROG_SET_TIME_SLOT_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                ++me->timeSlotIdx;
                me->timeSlotIdx %= NUM_TIME_SLOTS_PER_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
                sprintf(me->text1,
                        "F%d = %02u:%02u-%02u:%02u",
                        me->timeSlotIdx,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);

                //Show second timeslot (F2)
                sprintf(me->text2,
                        "%.1f'C",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);

                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                --me->timeSlotIdx;
                if (me->timeSlotIdx > (NUM_TIME_SLOTS_PER_DOW - 1))
                    me->timeSlotIdx = NUM_TIME_SLOTS_PER_DOW - 1; //If me->timeSlotIdx is greater than NUM_TIME_SLOTS_PER_DOW-1, set it to NUM_TIME_SLOTS_PER_DOW-1 again.
                sprintf(me->text1,
                        "F%d = %02u:%02u-%02u:%02u",
                        me->timeSlotIdx,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);

                //Show second timeslot (F2)
                sprintf(me->text2,
                        "%.1f'C",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);

                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                sprintf(me->text1, "Temp deseada F%d", me->timeSlotIdx);
                sprintf(me->text2,
                        "%.1f",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_SETPOINT_STATE;
            }
        }
        else if (me->stateMachineCurrState == PROG_SET_SETPOINT_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                float *setpointTemp =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);
                (*setpointTemp) += 0.5;
                sprintf(me->text1, "Temp deseada F%d", me->timeSlotIdx);
                sprintf(me->text2, "%.1f", *setpointTemp);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);

            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                float *setpointTemp =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);
                (*setpointTemp) -= 0.5;
                sprintf(me->text1, "Temp deseada F%d", me->timeSlotIdx);
                sprintf(me->text2, "%.1f", *setpointTemp);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);

            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                sprintf(me->text1, "Hora inicio F%d", me->timeSlotIdx);
                sprintf(me->text2,
                        "%02u",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_START_HOUR_STATE;
            }

        }
        else if (me->stateMachineCurrState == PROG_SET_START_HOUR_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                uint8_t *startHour =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour);
                ++(*startHour);
                (*startHour) %= 24;
                sprintf(me->text1, "Hora inicio F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *startHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                uint8_t *startHour =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour);
                --(*startHour);
                if (*startHour > 23)
                    *startHour = 23; //If me->hour is greater than 23, set it to 23 again.
                sprintf(me->text1, "Hora inicio F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *startHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                sprintf(me->text1, "Minuto inicio F%d", me->timeSlotIdx);
                sprintf(me->text2,
                        "%02u",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_START_MINUTES_STATE;
            }

        }
        else if (me->stateMachineCurrState == PROG_SET_START_MINUTES_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                uint8_t *startMin =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin);
                ++(*startMin);
                (*startMin) %= 60;
                sprintf(me->text1, "Minuto inicio F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *startMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                uint8_t *startMin =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin);
                --(*startMin);
                if (*startMin > 59)
                    *startMin = 59; //If me->hour is greater than 59, set it to 59 again.
                sprintf(me->text1, "Minuto inicio F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *startMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                sprintf(me->text1, "Hora fin F%d", me->timeSlotIdx);
                sprintf(me->text2,
                        "%02u",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_START_MINUTES_STATE;
                me->stateMachineCurrState = PROG_SET_END_HOUR_STATE;
            }

        }
        else if (me->stateMachineCurrState == PROG_SET_END_HOUR_STATE)
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                uint8_t *endHour =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour);
                ++(*endHour);
                (*endHour) %= 24;
                sprintf(me->text1, "Hora fin F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *endHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                uint8_t *endHour =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour);
                --(*endHour);
                if (*endHour > 23)
                    *endHour = 23; //If me->hour is greater than 23, set it to 23 again.
                sprintf(me->text1, "Hora fin F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *endHour);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                sprintf(me->text1, "Minuto fin F%d", me->timeSlotIdx);
                sprintf(me->text2,
                        "%02u",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_END_MINUTES_STATE;
            }
        }
        else
        {
            if (me->smEvent.eventType == INC_BTN_PRESSED)
            {
                uint8_t *endMin =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);
                ++(*endMin);
                (*endMin) %= 60;
                sprintf(me->text1, "Minuto fin F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *endMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == DEC_BTN_PRESSED)
            {
                uint8_t *endMin =
                        &(me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);
                --(*endMin);
                if (*endMin > 59)
                    *endMin = 59; //If me->hour is greater than 59, set it to 59 again.
                sprintf(me->text1, "Minuto fin F%d", me->timeSlotIdx);
                sprintf(me->text2, "%02u", *endMin);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
            }
            else if (me->smEvent.eventType == MODE_BTN_PRESSED)
            {
                sprintf(me->text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(me->text2, "T. Act:%.1f'C", readTemp->temperature);
                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = IDLE_STATE;
            }
            else //if me->smEvent.eventType == OK_BTN_PRESSED
            {
                //Update schedule configuration in Flash.
                //memcpy(&schedule, &me->schedule, sizeof(schedule_t));
                FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);
                while (!FlashCtl_eraseSector(SCHEDULE_FLASH_START));
                while (!FlashCtl_programMemory(&me->schedule, (void*) SCHEDULE_FLASH_START, sizeof(schedule_t)));
                FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);

                sprintf(me->text1,
                        "F%d = %02u:%02u-%02u:%02u",
                        me->timeSlotIdx,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].startMin,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endHour,
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].endMin);

                sprintf(me->text2,
                        "%.1f'C",
                        me->schedule.dowSched[me->dowIdx].timeSlot[me->timeSlotIdx].setpointTemp);

                me->dcEvt.eventType = PRINT_DATA;
                me->dcEvt.textR1 = me->text1;
                me->dcEvt.textR1Length = strlen(me->text1);
                me->dcEvt.textR2 = me->text2;
                me->dcEvt.textR2Length = strlen(me->text2);
                me->stateMachineCurrState = PROG_SET_TIME_SLOT_STATE;
            }

        }
        DisplayClient_updateNextBacklightOffTime(displayClient,
                                                 rtc->secondsCount);
        xQueueSend(displayClientEventQueue, &me->dcEvt, 0);

        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}


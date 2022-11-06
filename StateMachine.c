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

SMState_t stateMachineCurrState = IDLE_STATE;

const char *const daysOfWeek[NUM_DOW] = { "Lunes", "Martes", "Miercoles",
                                          "Jueves", "Viernes", "Sabado",
                                          "Domingo" };
const char *const progStatus[NUM_PROG_STATES] = { "OFF", "ON" };

#pragma CODE_SECTION(stateMachineThread, ".TI.ramfunc")
void* stateMachineThread(void *arg0)
{
    sem_wait(&startStateMachine);
    SMEvent_t *smEvent;

    //Vars used to set the current date and the scheduler
    static schedule_t schedule;
    memset(&schedule, 0, sizeof(schedule_t));
    static progState_t progState = OFF;
    static uint8_t dowIdx = 0;
    static uint8_t timeSlotIdx = 0;
    static uint8_t hour = 0;
    static uint8_t minute = 0;
    static char startTime[6];
    static char endTime[6];

    static DCEvent dcEvt;
    static char text1[16];
    static char text2[16];
    while (1)
    {
        //DO NOT DECLARE VARIABLES INSIDE while(1) SCOPE. A HARD FAULT IS RAISED OTHERWISE!
        xQueueReceive(stateMachineEventQueue, smEvent, portMAX_DELAY);
        //Clear structures
        dcEvt.textR1 = NULL;
        dcEvt.textR1Length = 0;
        dcEvt.textR2 = NULL;
        dcEvt.textR2Length = 0;
        strcpy(text1, "");
        strcpy(text2, "");
        if (stateMachineCurrState == IDLE_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                targetTemp->temperature += 0.5;
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                TempController_updateHeatingState(tempController);
            }
            if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                targetTemp->temperature -= 0.5;
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                TempController_updateHeatingState(tempController);
            }
            if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes fecha");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_DATE_STATE;
            }
        }
        else if (stateMachineCurrState == DISPLAY_SET_DATE_STATE)
        {
            if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes Prog");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            if (smEvent->eventType == OK_BTN_PRESSED)
            {

                ds3231_read(ds3231hdl, DAY_OF_WEEK, &dowIdx);
                strcpy(text1, "Dia Semana");
                sprintf(text2, "%s", daysOfWeek[dowIdx]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = SET_DOW_STATE;
            }
        }
        else if (stateMachineCurrState == DISPLAY_SET_PROG_STATE)
        {
            if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            if (smEvent->eventType == OK_BTN_PRESSED)
            {
                strcpy(text1, "Estado prog");
                sprintf(text2, "%s", progStatus[progState]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = PROG_STATE;
            }
        }
        else if (stateMachineCurrState == SET_DOW_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++dowIdx;
                dowIdx %= NUM_DOW; //If dowIdx is greater than 6, set it to 0 again.
                strcpy(text1, "Dia Semana");
                sprintf(text2, "%s", daysOfWeek[dowIdx]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);

            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                --dowIdx;
                if (dowIdx > (NUM_DOW - 1))
                    dowIdx = NUM_DOW - 1; //If dowIdx is greater than 6, set it to 6 again.
                strcpy(text1, "Dia Semana");
                sprintf(text2, "%s", daysOfWeek[dowIdx]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes Prog");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, DAY_OF_WEEK, &dowIdx); //Set DOW
                ds3231_read(ds3231hdl, HOUR, &hour); //Read hour
                strcpy(text1, "Horas");
                sprintf(text2, "%02d", hour);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = SET_HOUR_STATE;
            }
        }
        else if (stateMachineCurrState == SET_HOUR_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++hour;
                hour %= 24; //If hour is greater than 23, set it to 0 again.
                strcpy(text1, "Horas");
                sprintf(text2, "%02d", hour);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                --hour;
                if (hour > 23)
                    hour = 23; //If hour is greater than 23, set it to 23 again.
                strcpy(text1, "Horas");
                sprintf(text2, "%02d", hour);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes Prog");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, HOUR, &hour);
                ds3231_read(ds3231hdl, MINUTE, &minute); //Read hour
                strcpy(text1, "Minutos");
                sprintf(text2, "%02d", minute);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = SET_MINUTES_STATE;
            }
        }
        else if (stateMachineCurrState == SET_MINUTES_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++minute;
                minute %= 60; //If hour is greater than 59, set it to 0 again.
                strcpy(text1, "Minutos");
                sprintf(text2, "%02d", minute);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                --minute;
                if (minute > 59)
                    minute = 59; //If hour is greater than 59, set it to 59 again.
                strcpy(text1, "Minutos");
                sprintf(text2, "%02d", minute);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes Prog");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
                ds3231_set(ds3231hdl, MINUTE, &minute);
                strcpy(text1, "Ajustes fecha");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_DATE_STATE;
            }
        }
        else if (stateMachineCurrState == PROG_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++progState;
                progState %= NUM_PROG_STATES; //If dowIdx is greater than 1, set it to 0 again.
                strcpy(text1, "Estado prog");
                sprintf(text2, "%s", progStatus[progState]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                --progState;
                if (progState > (NUM_PROG_STATES - 1))
                    progState = OFF; //If dowIdx is greater than 1, set it to 1 again.
                strcpy(text1, "Estado prog");
                sprintf(text2, "%s", progStatus[progState]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
                if (progState == ON) //Schedule is enabled
                {
                    ds3231_read(ds3231hdl, DAY_OF_WEEK, &dowIdx);
                    strcpy(text1, "Dia Semana");
                    sprintf(text2, "%s", daysOfWeek[dowIdx]);
                    dcEvt.eventType = PRINT_DATA;
                    dcEvt.textR1 = text1;
                    dcEvt.textR1Length = strlen(text1);
                    dcEvt.textR2 = text2;
                    dcEvt.textR2Length = strlen(text2);
                    stateMachineCurrState = PROG_SET_DOW_STATE;

                }
                else    //Schedule is disabled
                {
                    strcpy(text1, "Ajustes Prog");
                    dcEvt.eventType = PRINT_DATA;
                    dcEvt.textR1 = text1;
                    dcEvt.textR1Length = strlen(text1);
                    stateMachineCurrState = DISPLAY_SET_PROG_STATE;
                }
            }

        }
        else if (stateMachineCurrState == PROG_SET_DOW_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++dowIdx;
                dowIdx %= NUM_DOW; //If dowIdx is greater than 6, set it to 0 again.
                strcpy(text1, "Dia Semana");
                sprintf(text2, "%s", daysOfWeek[dowIdx]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);

            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                --dowIdx;
                if (dowIdx > (NUM_DOW - 1))
                    dowIdx = NUM_DOW - 1; //If dowIdx is greater than 6, set it to 6 again.
                strcpy(text1, "Dia Semana");
                sprintf(text2, "%s", daysOfWeek[dowIdx]);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
                timeSlotIdx = 0; //Point to first time slot (F1)
                //Show first timeslot (F1)
                sprintf(startTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].startHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].startMin);
                sprintf(endTime, "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].endHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].endMin);
                //sprintf(text1, "F%d = %s-%s", timeSlotIdx, startTime, endTime);

                //Show second timeslot (F2)
                sprintf(startTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx + 1].startHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx + 1].startMin);
                sprintf(endTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx + 1].endHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx + 1].endMin);
                sprintf(text1, "F1 = %s-%s", startTime, endTime);

                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState == PROG_SET_TIME_SLOT_STATE;
            }

        }
        else if (stateMachineCurrState == PROG_SET_TIME_SLOT_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                /*++timeSlotIdx;
                timeSlotIdx %= NUM_TIME_SLOTS_PER_DOW; //If dowIdx is greater than 6, set it to 0 again.
                sprintf(startTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].startHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].startMin);
                sprintf(endTime, "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].endHour,
                        schedule.dowSched[dowIdx].timeSlot[timeSlotIdx].endMin);
                sprintf(text1, "F%c = %s-%s", timeSlotIdx, startTime, endTime);

                //Show second timeslot (F2)
                sprintf(startTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[(timeSlotIdx+1) % NUM_TIME_SLOTS_PER_DOW].startHour,
                        schedule.dowSched[dowIdx].timeSlot[(timeSlotIdx+1) % NUM_TIME_SLOTS_PER_DOW].startMin);
                sprintf(endTime,
                        "%02u:%02u",
                        schedule.dowSched[dowIdx].timeSlot[(timeSlotIdx+1) % NUM_TIME_SLOTS_PER_DOW].endHour,
                        schedule.dowSched[dowIdx].timeSlot[(timeSlotIdx+1) % NUM_TIME_SLOTS_PER_DOW].endMin);
                sprintf(text2, "F%u = %s-%s", (timeSlotIdx+1) % NUM_TIME_SLOTS_PER_DOW, startTime, endTime);*/
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_SETPOINT_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_START_HOUR_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_START_MINUTES_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_END_HOUR_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                stateMachineCurrState = IDLE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        DisplayClient_updateNextBacklightOffTime(displayClient,
                                                 rtc->secondsCount);
        xQueueSend(displayClientEventQueue, &dcEvt, 0);

        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}


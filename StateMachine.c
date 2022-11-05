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

SMState stateMachineCurrState = IDLE_STATE;

const char *const daysOfWeek[7] = { "Lunes", "Martes", "Miercoles", "Jueves",
                                    "Viernes", "Sabado", "Domingo" };

void* stateMachineThread(void *arg0)
{
    sem_wait(&startStateMachine);
    SMEvent *smEvent;

    //Vars used to set the current date and the scheduler
    static uint8_t dowIdx = 0;
    static uint8_t hour = 0;
    static uint8_t minute = 0;

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
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                targetTemp->temperature += 0.5;
                TempController_updateHeatingState(tempController);
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
                sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                dcEvt.textR2 = text2;
                dcEvt.textR2Length = strlen(text2);
                targetTemp->temperature -= 0.5;
                TempController_updateHeatingState(tempController);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                strcpy(text1, "Ajustes fecha");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                stateMachineCurrState = DISPLAY_SET_DATE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }
        }
        else if (stateMachineCurrState == DISPLAY_SET_DATE_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
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
        else if (stateMachineCurrState == SET_DOW_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                ++dowIdx;
                dowIdx %= 7; //If dowIdx is greater than 6, set it to 0 again.
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
                if (dowIdx > 6)
                    dowIdx = 6; //If dowIdx is greater than 6, set it to 6 again.
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
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_DOW_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }

        }
        else if (stateMachineCurrState == PROG_SET_TIME_SLOT_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
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

    //memset(text1, 0x00, 16); //Clear output buffer
    //memset(text2, 0x00, 16); //Clear output buffer
}


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

void* stateMachineThread(void *arg0)
{
    sem_wait(&startStateMachine);
    SMState curr_state = IDLE_STATE;
    SMEvent *smEvent;
    //char text1[16];
    static DCEvent dcEvt;
    static char text1[16];
    static char text2[16];
    while (1)
    {
        //DO NOT DECLARE VARIABLES INSIDE while(1) SCOPE! A HARD FAULT IS RAISED.
        xQueueReceive(stateMachineEventQueue, smEvent, portMAX_DELAY);
        if (curr_state == IDLE_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                //dcEvt.textR1 = text1;
                targetTemp->temperature += 0.5;
                TempController_updateHeatingState(tempController);
            }
            else if (smEvent->eventType == DEC_BTN_PRESSED)
            {
                targetTemp->temperature -= 0.5;
                TempController_updateHeatingState(tempController);
            }
            else if (smEvent->eventType == MODE_BTN_PRESSED)
            {
                //Show the text "Ajustes fecha"
                strcpy(text1, "Ajustes fecha");
                dcEvt.eventType = PRINT_DATA;
                dcEvt.textR1 = text1;
                dcEvt.textR1Length = strlen(text1);
                curr_state = DISPLAY_SET_DATE_STATE;
            }
            else //if smEvent->eventType == OK_BTN_PRESSED
            {
            }
        }
        else if (curr_state == DISPLAY_SET_DATE_STATE)
        {
            if (smEvent->eventType == INC_BTN_PRESSED)
            {
                //Show the text "Ajustes prog"
                curr_state = DISPLAY_SET_PROG_STATE;
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
        else if (curr_state == SET_DOW_STATE)
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
        else if (curr_state == SET_HOUR_STATE)
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
        else if (curr_state == SET_MINUTES_STATE)
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
        else if (curr_state == PROG_STATE)
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
        else if (curr_state == PROG_SET_DOW_STATE)
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
        else if (curr_state == PROG_SET_TIME_SLOT_STATE)
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
        else if (curr_state == PROG_SET_SETPOINT_STATE)
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
        else if (curr_state == PROG_SET_START_HOUR_STATE)
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
        else if (curr_state == PROG_SET_START_MINUTES_STATE)
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
        else if (curr_state == PROG_SET_END_HOUR_STATE)
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


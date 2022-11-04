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
extern TempData* readTemp;
extern TempData* targetTemp;
extern DS3231Proxy* ds3231hdl;

extern sem_t startStateMachine;

#pragma CODE_SECTION(stateMachineThread, ".TI.ramfunc")
void* stateMachineThread(void *arg0)
{
    sem_wait(&startStateMachine);
    SMState curr_state = IDLE_STATE;
    SMEvent* smEvent;
    while(1){
        xQueueReceive(stateMachineEventQueue, smEvent, portMAX_DELAY);
        DCEvent dcEvent;
        char text1[16];
        char text2[16];
        switch(curr_state)
        {
            case IDLE_STATE:
                if (smEvent->eventType == INC_BTN_PRESSED)
                {
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
                    sprintf(text1, "Ajustes fecha");
                    dcEvent.eventType = PRINT_DATA;
                    dcEvent.textR1 = text1;
                    dcEvent.textR1Length = strlen(text1);
                    curr_state = DISPLAY_SET_DATE_STATE;
                }
                break;
            case DISPLAY_SET_DATE_STATE:
                if (smEvent->eventType == MODE_BTN_PRESSED)
                {
                    //Show the text "Ajustes prog"
                    curr_state = DISPLAY_SET_PROG_STATE;
                }
                break;
            case DISPLAY_SET_PROG_STATE:
                break;
            case SET_DOW_STATE:
                break;
            case SET_HOUR_STATE:
                break;
            case SET_MINUTES_STATE:
                break;
            case PROG_STATE:
                break;
            case PROG_SET_DOW_STATE:
                break;
            case PROG_SET_TIME_SLOT_STATE:
                break;
            case PROG_SET_SETPOINT_STATE:
                break;
            case PROG_SET_START_HOUR_STATE:
                break;
            case PROG_SET_START_MINUTES_STATE:
                break;
            case PROG_SET_END_HOUR_STATE:
                break;
            case PROG_SET_END_MINUTES_STATE:
                break;
            default:
                break;
        }
        DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);
        xQueueSend(displayClientEventQueue, &dcEvent, 0);
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);

        //memset(text1, 0x00, 16); //Clear output buffer
        //memset(text2, 0x00, 16); //Clear output buffer
    }
}

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

extern sem_t unlockDisplayThread;

void* stateMachineThread(void* arg0){
    State curr_state = IDLE_STATE;
    Event* event;
    while(1){
        xQueueReceive(stateMachineEventQueue, event, portMAX_DELAY);
        switch(curr_state)
        {
            case IDLE_STATE:
                if (event->eventType == INC_BTN_PRESSED)
                {
                    targetTemp->temperature += 0.5;
                }
                else if (event->eventType == DEC_BTN_PRESSED)
                {
                    targetTemp->temperature -= 0.5;
                }
                TempController_updateHeatingState(tempController);
                displayClient->flags = PRINT_DATA;
                DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);
                sem_post(&unlockDisplayThread);
                break;
            case DISPLAY_SET_DATE_STATE:
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
        }
    }
}

/*
 * mainThread.c
 *
 *  Created on: 7 sept. 2022
 *      Author: aaron
 */

#include "mainThread.h"

TempController *tempController;
TempSensor *tempSensor;
DisplayClient *displayClient;
Keypad *keypad;
RTC *rtc;
TempData *readTemp;
TempData *targetTemp;
DS3231Proxy *ds3231hdl;

uint32_t secondsCount;

extern sem_t startReadingTemp;
extern sem_t initDisplayDone;
extern sem_t startStateMachine;
extern sem_t initDs3231Done;

Power_NotifyObj notifyObj;

extern StateMachine *stateMachine;

//#pragma CODE_SECTION(mainThread, ".TI.ramfunc")
void* mainThread(void *arg)
{
    /*Power_registerNotify(&notifyObj, PowerMSP432_ENTERING_DEEPSLEEP | PowerMSP432_AWAKE_DEEPSLEEP,
     (Power_NotifyFxn)notifyFxn, 0);*/

    //Initialize data structures
    readTemp = TempData_create();
    TempData_init(readTemp, 21.0, 0.0);

    targetTemp = TempData_create();
    TempData_init(targetTemp, 21.0, 0.0);

    rtc = RTC_C_create();
    RTC_C_configure(rtc);

    tempSensor = TempSensor_create();
    TempSensor_init(tempSensor, readTemp);
    TempSensor_updateNextTempUpdateTime(tempSensor, rtc->secondsCount); //Init timer to read temperature periodically

    tempController = TempController_create();
    TempController_init(tempController, readTemp, targetTemp);

    keypad = Keypad_create();
    Keypad_init(keypad);

    uint8_t data_array[7] = { 0, 26, 20, 5, 5, 11, 22 };
    ds3231hdl = ds3231_create();
    ds3231_init(ds3231hdl, data_array, CLOCK_RUN, NO_RESET);

    displayClient = DisplayClient_create();
    DisplayClient_init(displayClient);
    DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);

    Power_setPolicy((Power_PolicyFxn) PowerMSP432_deepSleepPolicy);
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_1);
    Power_enablePolicy();

    RTC_C_init(rtc);

    sem_post(&initDisplayDone); //Allow DisplayLCDThread to continue since displayClient instance is already initialized
    sem_post(&initDs3231Done);
    sem_post(&startStateMachine);
    DCEvent event;
    char text1[16];
    char text2[16];

    while (1)
    {
        Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        TempSensor_readTemp(tempSensor);
        if (stateMachine->stateMachineCurrState == IDLE_STATE) //Print new read temperature only when in IDLE_STATE to prevent current displayed data in LCD to be overwritten
        {
            sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
            event.eventType = PRINT_DATA;
            event.textR1 = text1;
            event.textR1Length = strlen(text1);
            event.textR2 = text2;
            event.textR2Length = strlen(text2);
            xQueueSend(displayClientEventQueue, &event, 0);
            memset(text1, 0x00, 16); //Clear output buffer
            memset(text2, 0x00, 16); //Clear output buffer
        }
        TempController_updateHeatingState(tempController);
        DisplayClient_updateNextBacklightOffTime(displayClient,
                                                 rtc->secondsCount);
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        sem_wait(&startReadingTemp);
    }
}

/***************** RTC interrupt handler (called every 1s) ***************/

void RTC_C_IRQHandler(uint32_t arg)
{
    uint32_t status;

    status = RTC_C_getEnabledInterruptStatus();
    RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_PRESCALE_TIMER1_INTERRUPT)
    {
        if (RTC_C_isTimerExpired(rtc, tempSensor->nextTempUpdateTime))
        {
            //Allow reading temp every 15s
            TempSensor_updateNextTempUpdateTime(tempSensor, rtc->secondsCount);
            sem_post(&startReadingTemp);
            //Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        }
        if (RTC_C_isTimerExpired(rtc, displayClient->nextBacklightOffTime))
        {
            DCEvent dcEvent;
            dcEvent.eventType = OFF_BACKLIGHT;
            xQueueSendFromISR(displayClientEventQueue, &dcEvent, NULL);
            //Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        }
        if (RTC_C_isTimerExpired(rtc, ds3231hdl->nextMinuteCheck))
        {
            schedule_t sched = stateMachine->schedule;
            //Check schedule
            ds3231_updateNextMinuteCheck(ds3231hdl, rtc->secondsCount);
            if (stateMachine->progState == ON)
            {
                //Read current time from DS3231
                DS3231Event ds3231Event;
                ds3231Event.eventType = READ_TIME;
                xQueueSendFromISR(ds3231EventQueue, &ds3231Event, NULL);
                //Check if we should jump to another timeslot setpoint temperature

                uint8_t currDow = ds3231hdl->dow;
                uint8_t currHour = ds3231hdl->hour;
                uint8_t currMin = ds3231hdl->min;
                uint8_t tsIdx;
                for (tsIdx = F0; tsIdx < NUM_TIME_SLOTS_PER_DOW; ++tsIdx)
                {
                    timeSlot_t *ts = &(sched.dowSched[currDow].timeSlot[tsIdx]);
                    if (currHour >= ts->startHour && currHour <= ts->endHour
                            && (currHour != ts->startHour
                                    || currMin >= ts->startMin)
                            && (currHour != ts->endHour || currMin < ts->endMin))
                    {
                        targetTemp->temperature = ts->setpointTemp;
                        break;
                    }
                }
                TempController_updateHeatingState(tempController);
            }
            /*sprintf(text1, "T. Obj:%.1f'C", targetTemp->temperature);
             sprintf(text2, "T. Act:%.1f'C", readTemp->temperature);
             dcEvent.eventType = PRINT_DATA;
             dcEvent.textR1 = text1;
             dcEvent.textR1Length = strlen(text1);
             dcEvent.textR2 = text2;
             dcEvent.textR2Length = strlen(text2);*/
        }
    }
    ++(rtc->secondsCount); // A second has elapsed
}

/********************************* Keypad Interrupt handler **********************************************/

void Keypad_InterruptHandler(uint_least8_t idx)
{
    GPIO_clearInt(idx);
    GPIO_disableInt(idx);
    delay_us(30000);
    SMEvent_t ev;
    if (idx == INC_BUTTON_PIN_IDX)
    {
        if (!GPIO_read(INC_BUTTON_PIN_IDX)) //When button is pressed, read value is 0.
        {
            ev.eventType = INC_BTN_PRESSED;
            xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);

            delay_us(250000);
            while (!GPIO_read(INC_BUTTON_PIN_IDX))
            {
                int i;
                for (i = 0; i < 5; ++i)
                {
                    xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);
                }
                delay_us(250000);
            }
        }
    }
    if (idx == DEC_BUTTON_PIN_IDX)
    {
        if (!GPIO_read(DEC_BUTTON_PIN_IDX))
        {
            ev.eventType = DEC_BTN_PRESSED;
            xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);
            delay_us(250000);
            while (!GPIO_read(DEC_BUTTON_PIN_IDX))
            {
                int i;
                for (i = 0; i < 5; ++i)
                {
                    xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);
                }
                delay_us(250000);
            }
        }
    }
    if (idx == MODE_BUTTON_PIN_IDX)
    {
        if (!GPIO_read(MODE_BUTTON_PIN_IDX))
        {
            ev.eventType = MODE_BTN_PRESSED;
            xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);
        }

    }
    if (idx == OK_BUTTON_PIN_IDX)
    {
        if (!GPIO_read(OK_BUTTON_PIN_IDX))
        {
            ev.eventType = OK_BTN_PRESSED;
            xQueueSendFromISR(stateMachineEventQueue, &ev, NULL);
        }
    }
    GPIO_enableInt(idx);
    GPIO_clearInt(idx);
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
}

/*
 *  ======== notifyFxn ========
 *  Notification function to call as the CPU is being transitioned to sleep
 */
unsigned int notifyFxn(unsigned int eventType, unsigned int eventArg,
                       unsigned int clientArg)
{
    if ((eventType == PowerMSP432_ENTERING_SLEEP)
            || (eventType == PowerMSP432_ENTERING_DEEPSLEEP))
    {
        /*
         * Turn off PSS high-side supervisors to consume lower power in deep sleep
         */
        PSS_disableHighSide();
        //CS_initClockSignal(CS_MCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    }
    else if ((eventType == PowerMSP432_AWAKE_SLEEP)
            || (eventType == PowerMSP432_AWAKE_DEEPSLEEP))
    {
        //CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    }
    else if (eventType == PowerMSP432_START_CHANGE_PERF_LEVEL)
    {

    }
    else if (eventType == PowerMSP432_DONE_CHANGE_PERF_LEVEL)
    {

    }
    else
    {

    }
    return (Power_NOTIFYDONE);
}

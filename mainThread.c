/*
 * mainThread.c
 *
 *  Created on: 7 sept. 2022
 *      Author: aaron
 */


/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */


#include "mainThread.h"

TempController *tempController;
TempSensor *tempSensor;
DisplayClient *displayClient;
Keypad *keypad;
RTC *rtc;
TempData* readTemp;
TempData* targetTemp;
DS3231Proxy* ds3231hdl;

uint32_t secondsCount;

extern sem_t startReadingTemp;
extern sem_t unlockDisplayThread;
extern sem_t initDisplayDone;

Power_NotifyObj notifyObj;

//#pragma CODE_SECTION(mainThread, ".TI.ramfunc")
void *mainThread(void* arg)
{

    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);

    /*Power_registerNotify(&notifyObj, PowerMSP432_ENTERING_DEEPSLEEP | PowerMSP432_AWAKE_DEEPSLEEP,
            (Power_NotifyFxn)notifyFxn, 0);*/

    //Initialize data structures
    readTemp = TempData_create();
    TempData_init(readTemp, 0.0, 0.0);

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

    uint8_t data_array[7] = {0, 45, 19, 6, 30, 10, 22};
    ds3231hdl = ds3231_create();
    ds3231_init(ds3231hdl, data_array, CLOCK_RUN, FORCE_RESET);

    displayClient = DisplayClient_create();
    DisplayClient_init(displayClient, readTemp, targetTemp);
    DisplayClient_updateNextBacklightOffTime(displayClient,  rtc->secondsCount);
    sem_post(&initDisplayDone); //Allow DisplayLCDThread to continue since displayClient instance is already initialized

    RTC_C_init(rtc);

    while(1){
        sem_wait(&startReadingTemp);
        TempSensor_readTemp(tempSensor);
        TempController_updateHeatingState(tempController);
        displayClient->flags = PRINT_DATA;
        DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);
        ds3231_read(ds3231hdl, TIME, data_array);
        sem_post(&unlockDisplayThread);
    }
}

/***************** RTC interrupt handler (called every 1s) ***************/

void RTC_C_IRQHandler(uint32_t arg)
{
    uint32_t status;

    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
    status = RTC_C_getEnabledInterruptStatus();
    RTC_C_clearInterruptFlag(status);
    ++(rtc->secondsCount); // A second has elapsed

    if(status & RTC_C_PRESCALE_TIMER1_INTERRUPT)
    {
        if (RTC_C_isTimerExpired(rtc, tempSensor->nextTempUpdateTime))
        {
            //Allow reading temp every 15s
            TempSensor_updateNextTempUpdateTime(tempSensor, rtc->secondsCount);
            sem_post(&startReadingTemp);
            Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        }
        if(RTC_C_isTimerExpired(rtc, displayClient->nextBacklightOffTime))
        {
            displayClient->flags = OFF_BACKLIGHT;
            sem_post(&unlockDisplayThread);
            Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        }
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);

}

/********************************* Keypad Interrupt handler **********************************************/


void Keypad_InterruptHandler(uint_least8_t idx)
{
    GPIO_clearInt(idx);
    GPIO_disableInt(idx);

    if (idx == INC_TEMP_PIN_IDX)
    {
        targetTemp->temperature += 0.5;
    }
    if (idx == DEC_TEMP_PIN_IDX)
    {
        targetTemp->temperature -= 0.5;
    }
    TempController_updateHeatingState(tempController);
    displayClient->flags = PRINT_DATA;
    DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);
    sem_post(&unlockDisplayThread);
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    GPIO_enableInt(idx);
}



/*
 *  ======== notifyFxn ========
 *  Notification function to call as the CPU is being transitioned to sleep
 */
unsigned int notifyFxn(unsigned int eventType, unsigned int eventArg,
    unsigned int clientArg)
{
    if( (eventType == PowerMSP432_ENTERING_SLEEP) ||
        (eventType == PowerMSP432_ENTERING_DEEPSLEEP))
    {
        /*
         * Turn off PSS high-side supervisors to consume lower power in deep sleep
         */
        PSS_disableHighSide();
        //CS_initClockSignal(CS_MCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    }
    else if((eventType == PowerMSP432_AWAKE_SLEEP) ||
            (eventType == PowerMSP432_AWAKE_DEEPSLEEP))
    {
        //CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    }
    else if(eventType == PowerMSP432_START_CHANGE_PERF_LEVEL)
    {

    }
    else if(eventType == PowerMSP432_DONE_CHANGE_PERF_LEVEL)
    {

    }
    else
    {

    }
    return(Power_NOTIFYDONE);
}

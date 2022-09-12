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

#define TEMP_READ_PERIOD    0.2 //minutes

TempController *tempController;
DisplayClient *displayClient;
TempSensor *tempSensor;
Keypad *keypad;
RTC *rtc;
TempData* readTemp;
TempData* targetTemp;

uint32_t secondsCount;

sem_t startReadingTemp;
Power_NotifyObj notifyObj;

//#pragma CODE_SECTION(mainThread, ".TI.ramfunc")
void *mainThread(void* arg)
{
    int retc;
    retc = sem_init(&startReadingTemp, 0, 0);
    if (retc == -1) {
        while (1);
    }
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);

    /*Power_registerNotify(&notifyObj, PowerMSP432_ENTERING_DEEPSLEEP | PowerMSP432_AWAKE_DEEPSLEEP,
            (Power_NotifyFxn)notifyFxn, 0);*/

    //Initialize data structures
    readTemp = TempData_create();
    TempData_init(readTemp, 0.0, 0.0);

    targetTemp = TempData_create();
    TempData_init(targetTemp, 21.0, 0.0);

    tempSensor = TempSensor_create();
    TempSensor_init(tempSensor, readTemp);

    tempController = TempController_create();
    TempController_init(tempController, readTemp, targetTemp);

    displayClient = DisplayClient_create();
    DisplayClient_init(displayClient, readTemp, targetTemp);

    keypad = Keypad_create();
    Keypad_init(keypad);

    rtc = RTC_C_create();
    RTC_C_configure(rtc, TEMP_READ_PERIOD);
    RTC_C_init(rtc);

    while(1){
        sem_wait(&startReadingTemp);
        TempSensor_readTemp(tempSensor);
        //DisplayClient_setItsTempSensed(displayClient, tempSensor->itsTempSensorProxy->itsTempData);
        //TempController_setReadTemp(tempController, tempSensor->itsTempSensorProxy->itsTempData);
        DisplayClient_showInfo(displayClient);
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}

/***************** RTC interrupt handler (called every 1s) ***************/

void RTC_C_IRQHandler(uint32_t arg)
{
    uint32_t status;

    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
    status = RTC_C_getEnabledInterruptStatus();
    RTC_C_clearInterruptFlag(status);
    ++(rtc->secondsCount); // A second has elapsed

    /*
     * reset scalers for the next second
     */
    if(status & RTC_C_PRESCALE_TIMER1_INTERRUPT)
    {
        if (rtc->secondsCount == (uint32_t)(rtc->tempReadPeriod * 60))
        {   //Allow reading temp every 12s
            sem_post(&startReadingTemp);
            rtc->secondsCount = 0;
            Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
        }
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);

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
    //DisplayClient_showInfo(displayClient);
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

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
RTC* rtc;

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

    Power_registerNotify(&notifyObj, PowerMSP432_ENTERING_DEEPSLEEP | PowerMSP432_AWAKE_DEEPSLEEP,
            (Power_NotifyFxn)notifyFxn, 0);

    tempSensor = TempSensor_create();
    tempController = TempController_create();
    displayClient = DisplayClient_create();
    rtc = RTC_C_create();
    RTC_C_configure(rtc, TEMP_READ_PERIOD);
    RTC_C_init(rtc);

    while(1){
        sem_wait(&startReadingTemp);
        TempSensor_readTemp(tempSensor);
        DisplayClient_setItsTempSensed(displayClient, tempSensor->itsTempSensorProxy->itsTempData);
        TempController_setReadTemp(tempController, tempSensor->itsTempSensorProxy->itsTempData);
        DisplayClient_showInfo(displayClient);
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
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

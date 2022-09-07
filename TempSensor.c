/*
 * TempSensor.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include <interrupt.h>
#include <LCDdisplayClient.h>
#include <rtc_c.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <ti/drivers/dpl/HwiP.h>
#include <TempController.h>
#include "TempSensor.h"

TempController *tempController;
DisplayClient *displayClient;
TempSensor *tempSensor;

sem_t startReadingTemp;
Power_NotifyObj notifyObj;

void RTC_C_Init(void);
void RTC_C_IRQHandler(uintptr_t arg);

unsigned int notifyFxn(unsigned int eventType, unsigned int eventArg, unsigned int clientArg);

void TempSensor_init(TempSensor *const me)
{
    me->itsTempSensorProxy = TempSensorProxy_create();
}

//#pragma CODE_SECTION(TempSensor_readTemp, ".TI.ramfunc")
void TempSensor_readTemp(TempSensor *const me)
{
    TempSensorProxy_access(me->itsTempSensorProxy); //Read temperature
    //xQueueSend(me->qTReadToLCD, (void *)(me->itsTempSensorProxy->itsTempData), 0); //Push data to LCD queue

    //Print to console
    /*
    char buff[128];
    sprintf(buff, "T: %.1fºC, H: %.1f%% \n", me->itsTempSensorProxy->itsTempData->temperature,
                                         me->itsTempSensorProxy->itsTempData->humidity); //whitespaces to clear line
    DisplayConsoleMsg msg;
    msg.buff = buff;
    msg.len = strlen(buff);
    xQueueSend(me->qDispConsole, (void *)&msg, 0);

    TempData td;
    td.temperature = me->itsTempSensorProxy->itsTempData->temperature;
    td.humidity = me->itsTempSensorProxy->itsTempData->humidity;
    xQueueSend(me->qTReadToTCtrl, (void *)&td, 0);*/
}


TempSensor* TempSensor_create()
{
    TempSensor* me = (TempSensor*) malloc(sizeof(TempSensor));
    if (me != NULL)
    {
        TempSensor_init(me);
    }
    return me;
}

void TempSensor_destroy(TempSensor *const me)
{
    if (me != NULL)
    {
        TempSensor_clean(me);
    }
    free(me);
}

//#pragma CODE_SECTION(temperatureReadingThread, ".TI.ramfunc")
void *temperatureReadingThread(void* arg){

    tempSensor = TempSensor_create();
    tempController = TempController_create();
    displayClient = DisplayClient_create();

    int retc;
    retc = sem_init(&startReadingTemp, 0, 0);
    if (retc == -1) {
        while (1);
    }
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);

    RTC_C_Init();
    TempSensor_readTemp(tempSensor);
    while(1){
        sem_wait(&startReadingTemp);
        TempSensor_readTemp(tempSensor);
        DisplayClient_setItsTempSensed(displayClient, tempSensor->itsTempSensorProxy->itsTempData);
        DisplayClient_showInfo(displayClient);
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}

void RTC_C_Init(void)
{
    /* Starting LFXT in non-bypass mode without a timeout */
    CS_initClockSignal(CS_BCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    Power_setPolicy((Power_PolicyFxn)PowerMSP432_deepSleepPolicy);
    Power_enablePolicy();
    /*
     * allowing down to deepsleep_0 = LPM3
     */
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_1);

    /* Register for entering shutdown notifications */
    Power_registerNotify(&notifyObj, PowerMSP432_ENTERING_DEEPSLEEP | PowerMSP432_AWAKE_DEEPSLEEP,
        (Power_NotifyFxn)notifyFxn, 0);

    /* RTC, 1s period */
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_0,0x00);
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_1,0x00);
    RTC_C->PS1CTL = RTC_C_PS1CTL_RT1IP_7;
    RTC_C_enableInterrupt(RTC_C_PRESCALE_TIMER1_INTERRUPT);
    Interrupt_enableInterrupt(INT_RTC_C);

    HwiP_Params hwiParams;
    HwiP_Handle myHwi;
    HwiP_Params_init(&hwiParams);
    hwiParams.arg=5;
    hwiParams.priority=0x40;

    myHwi = HwiP_create(INT_RTC_C, RTC_C_IRQHandler, &hwiParams);

    if (myHwi == NULL)
    {
        while (1);
    }

    /* Start the RTC */
    RTC_C_startClock();
}

void RTC_C_IRQHandler(uint32_t arg)
{
    uint32_t status;

    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);

    status = RTC_C_getEnabledInterruptStatus();
    RTC_C_clearInterruptFlag(status);

    /*
     * reset scalers for the next second
     */
    if(status & RTC_C_PRESCALE_TIMER1_INTERRUPT)
    {
        RTC_C_holdClock();
        RTC_C_setPrescaleValue(RTC_C_PRESCALE_0,0x00);
        RTC_C_setPrescaleValue(RTC_C_PRESCALE_1,0x00);
        /* Re-start RTC Clock */
        RTC_C_startClock();
        /* Post to the Task */
        sem_post(&startReadingTemp);
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
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

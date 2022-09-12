/*
 * RTC.c
 *
 *  Created on: 9 sept. 2022
 *      Author: aaron
 */

#include <cs.h>
#include <gpio.h>
#include <interrupt.h>
#include <rtc_c.h>
#include <stdlib.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/power/PowerMSP432.h>
#include <ti/drivers/Power.h>
#include "RTC.h"


RTC* RTC_C_create(void)
{
    RTC* me = (RTC*) malloc(sizeof(RTC));
    return me;
}

void RTC_C_init(RTC* const me)
{
    Power_setPolicy((Power_PolicyFxn)PowerMSP432_deepSleepPolicy);
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_1);
    Power_enablePolicy();

    /* RTC, 1s period */
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_0,0x00);
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_1,0x00);
    RTC_C_definePrescaleEvent(RTC_C_PRESCALE_1, RTC_C_PSEVENTDIVIDER_128);
    //RTC_C_PRESCALE_TIMER1_INTERRUPT, con divider 256 permite generar interrupcion cada 2 segundos.
    //Es lo m�ximo que se puede en este modo.
    RTC_C_enableInterrupt(RTC_C_PRESCALE_TIMER1_INTERRUPT);
    Interrupt_enableInterrupt(INT_RTC_C);

    HwiP_Params hwiParams;
    HwiP_Handle myHwi;
    HwiP_Params_init(&hwiParams);
    hwiParams.arg=5;
    hwiParams.priority=0x40;

    myHwi = HwiP_create(INT_RTC_C, RTC_C_IRQHandler, &hwiParams);

    /* Start the RTC */
    RTC_C_startClock();
}

void RTC_C_configure(RTC* const me, float newTmpReadPeriod)
{
    if (me != NULL)
    {
        me->tempReadPeriod = newTmpReadPeriod;
        me->secondsCount = 0;
    }
}
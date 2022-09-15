/*
 * LCDdisplayClient.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include <semaphore.h>
#include <ti/drivers/Power.h>
#include "LCDdisplayClient.h"

DisplayClient *gObj; //global DisplayClient "object" reference
extern DisplayClient *displayClient;

extern sem_t displayData;
extern sem_t initDisplayDone;

void DisplayClient_turnOffLCDbacklight(TimerHandle_t xTimer)
{
    LCD_noDisplay(gObj->itsDisplayProxy);
    LCD_noBacklight(gObj->itsDisplayProxy);
}

void DisplayClient_init(DisplayClient *const me, TempData* readTemp, TempData* targetTemp)
{
    me->itsTempSensed = readTemp;
    me->itsTempTarget = targetTemp;
    me->secondsCount = 0;
    me->nextBacklightOffTime = 0;
    me->backlightOnDuration = LCD_ON_BACKLIGHT_T;
    LCD_init(me->itsDisplayProxy);
}

void DisplayClient_clean(DisplayClient *const me)
{

}

void DisplayClient_acceptTempSensed(DisplayClient *const me, TempData *td)
{

}

void DisplayClient_acceptTempTarget(DisplayClient *const me, TempData *td)
{

}

#pragma CODE_SECTION(DisplayClient_showInfo, ".TI.ramfunc")
void DisplayClient_showInfo(DisplayClient *const me)
{
    char output[50];
    LCD_backlight(me->itsDisplayProxy);
    LCD_display(me->itsDisplayProxy);

    LCD_setCursor(me->itsDisplayProxy, 0, 1); //Move to second line in LCD
    sprintf(output, "                "); //whitespaces to clear line
    LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
    LCD_setCursor(me->itsDisplayProxy, 0, 1);

    //Update target temperature in the screen
    if (me->itsTempTarget->temperature)
    {
        sprintf(output, "T. Obj:%.1f'C", me->itsTempTarget->temperature);
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
    else
    {
        strcpy(output, "No data");
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }

    memset(output, 0x00, 50); //Clear output buffer
    LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
    sprintf(output, "                "); //whitespaces to clear line
    LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
    LCD_setCursor(me->itsDisplayProxy, 0, 0);
    //Update sensed temperature in the screen
    if (me->itsTempSensed->temperature)
    {
        sprintf(output, "T. Act:%.1f'C", me->itsTempSensed->temperature); //DIVIDE BY 10 TO GET REAL TEMPERATURE!
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Print sensed temperature
    }
    else
    {
        strcpy(output, "No data");
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
}

void DisplayClient_setItsTempSensed(DisplayClient* const me, TempData* p_td){
    if (me != NULL){
        me->itsTempSensed = p_td;
    }
}

DisplayClient* DisplayClient_create(void)
{
    DisplayClient *me = (DisplayClient*) malloc(sizeof(DisplayClient));
    if (me != NULL){
        //DisplayClient_init(me);
        me->itsDisplayProxy = LCD_create();
    }
    return me;
}

void DisplayClient_destroy(DisplayClient *const me)
{
    if (me != NULL)
    {
        DisplayClient_clean(me);
    }
    free(me);
}

#pragma CODE_SECTION(displayLCDThread, ".TI.ramfunc")
void* displayLCDThread(void *arg0)
{
    //Creates DisplayClient "instance"
    sem_wait(&initDisplayDone);
    //gObj = me;

    /*
    TimerHandle_t lcdOFFBl_tmr = xTimerCreate(
            "LCDBacklightOFF", pdMS_TO_TICKS(LCD_ON_BACKLIGHT_T), pdFALSE,
            (void*) 0, DisplayClient_turnOffLCDbacklight);*/

    //Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);

    while (1)
    {
        sem_wait(&displayData);
        DisplayClient_showInfo(displayClient);

            /*xTimerReset(lcdOFFBl_tmr, 0);

            if (!xTimerIsTimerActive(lcdOFFBl_tmr))
            {
                xTimerStart(lcdOFFBl_tmr, 0);
            }*/
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}

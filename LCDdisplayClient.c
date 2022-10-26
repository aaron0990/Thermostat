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

extern sem_t unlockDisplayThread;
extern sem_t initDisplayDone;

void DisplayClient_turnOffLCDbacklight(DisplayClient *const me)
{
    LCD_noDisplay(me->itsDisplayProxy);
    LCD_noBacklight(me->itsDisplayProxy);
}

void DisplayClient_init(DisplayClient *const me, TempData* readTemp, TempData* targetTemp)
{
    me->itsTempSensed = readTemp;
    me->itsTempTarget = targetTemp;
    me->nextBacklightOffTime = 0;
    me->backlightOnDuration = LCD_ON_BACKLIGHT_T;
    me->flags = SLEEP;
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

void DisplayClient_updateNextBacklightOffTime(DisplayClient *const me, uint32_t currentTime)
{
    if(me != NULL)
    {
        me->nextBacklightOffTime = currentTime + me->backlightOnDuration;
    }
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
    while (1)
    {
        sem_wait(&unlockDisplayThread);
        switch(displayClient->flags)
        {
            case OFF_BACKLIGHT:
                DisplayClient_turnOffLCDbacklight(displayClient);
                break;
            case PRINT_DATA:
                DisplayClient_showInfo(displayClient);
                break;
            default:
                break;
        }
        displayClient->flags = SLEEP;
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}

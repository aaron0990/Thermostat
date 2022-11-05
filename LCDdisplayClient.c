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

extern sem_t initDisplayDone;

void DisplayClient_turnOffLCDbacklight(DisplayClient *const me)
{
    LCD_noDisplay(me->itsDisplayProxy);
    LCD_noBacklight(me->itsDisplayProxy);
}

void DisplayClient_init(DisplayClient *const me)
{
    me->nextBacklightOffTime = 0;
    me->backlightOnDuration = LCD_ON_BACKLIGHT_T;
    LCD_init(me->itsDisplayProxy);
}

void DisplayClient_clean(DisplayClient *const me)
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
void DisplayClient_showInfo(DisplayClient *const me, DCEvent* event)
{
    LCD_backlight(me->itsDisplayProxy);
    LCD_display(me->itsDisplayProxy);

    LCD_clear(me->itsDisplayProxy); //Clear all screen data

    if(event->textR1Length)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to upper line in LCD
        LCD_write(me->itsDisplayProxy, event->textR1, event->textR1Length);
    }
    if(event->textR2Length)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 1); //Move to lower line in LCD
        LCD_write(me->itsDisplayProxy, event->textR2, event->textR2Length);
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
    DCEvent* event;
    while (1)
    {
        xQueueReceive(displayClientEventQueue, event, portMAX_DELAY);
        switch(event->eventType)
        {
            case OFF_BACKLIGHT:
                DisplayClient_turnOffLCDbacklight(displayClient);
                break;
            case PRINT_DATA:
                DisplayClient_showInfo(displayClient, event);
                break;
            default:
                break;
        }
        Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    }
}

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
extern TempData *readTemp;
extern TempData *targetTemp;
extern DS3231Proxy *ds3231hdl;

extern sem_t startStateMachine;

SMState_t *stateMachineCurrState;
StateMachine *stateMachine;

const char *const daysOfWeek[NUM_DOW] = { "Lunes", "Martes", "Miercoles",
                                          "Jueves", "Viernes", "Sabado",
                                          "Domingo" };
const char *const progStatus[NUM_PROG_STATES] = { "OFF", "ON" };

#pragma DATA_SECTION(schedule, ".schedule_data");
#pragma DATA_ALIGN(schedule, 4);
schedule_t schedule;

void StateMachine_init(StateMachine *const me)
{
    //FlashCtl_setWaitState(FLASH_BANK0, 1);
    //FlashCtl_setWaitState(FLASH_BANK1, 1);
    //memcpy(&schedule, &me->schedule, sizeof(schedule_t));
    stateMachineCurrState = &me->stateMachineCurrState;
    me->stateMachineCurrState = IDLE_STATE;
    memset(&me->smEvent, 0, sizeof(SMEvent_t));
    me->dowIdx = 0;
    me->timeSlotIdx = 0;
    me->hour = 0;
    me->minute = 0;
    me->progState = OFF;

    FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);
    memcpy(&me->schedule, (void*) SCHEDULE_FLASH_START, sizeof(schedule_t));
    FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);

    // LUNES
    me->schedule.dowSched[MON].timeSlot[F0].setpointTemp = 19.0;
    me->schedule.dowSched[MON].timeSlot[F0].startHour = 00;
    me->schedule.dowSched[MON].timeSlot[F0].startMin = 00;
    me->schedule.dowSched[MON].timeSlot[F0].endHour = 06;
    me->schedule.dowSched[MON].timeSlot[F0].endMin = 00;

    me->schedule.dowSched[MON].timeSlot[F1].setpointTemp = 21.0;
    me->schedule.dowSched[MON].timeSlot[F1].startHour = 06;
    me->schedule.dowSched[MON].timeSlot[F1].startMin = 00;
    me->schedule.dowSched[MON].timeSlot[F1].endHour = 10;
    me->schedule.dowSched[MON].timeSlot[F1].endMin = 30;

    me->schedule.dowSched[MON].timeSlot[F2].setpointTemp = 19.0;
    me->schedule.dowSched[MON].timeSlot[F2].startHour = 10;
    me->schedule.dowSched[MON].timeSlot[F2].startMin = 30;
    me->schedule.dowSched[MON].timeSlot[F2].endHour = 15;
    me->schedule.dowSched[MON].timeSlot[F2].endMin = 00;

    me->schedule.dowSched[MON].timeSlot[F3].setpointTemp = 21.0;
    me->schedule.dowSched[MON].timeSlot[F3].startHour = 15;
    me->schedule.dowSched[MON].timeSlot[F3].startMin = 00;
    me->schedule.dowSched[MON].timeSlot[F3].endHour = 22;
    me->schedule.dowSched[MON].timeSlot[F3].endMin = 00;

    me->schedule.dowSched[MON].timeSlot[F4].setpointTemp = 19.0;
    me->schedule.dowSched[MON].timeSlot[F4].startHour = 22;
    me->schedule.dowSched[MON].timeSlot[F4].startMin = 00;
    me->schedule.dowSched[MON].timeSlot[F4].endHour = 00;
    me->schedule.dowSched[MON].timeSlot[F4].endMin = 00;

    // MARTES
    me->schedule.dowSched[TUE].timeSlot[F0].setpointTemp = 19.0;
    me->schedule.dowSched[TUE].timeSlot[F0].startHour = 00;
    me->schedule.dowSched[TUE].timeSlot[F0].startMin = 00;
    me->schedule.dowSched[TUE].timeSlot[F0].endHour = 06;
    me->schedule.dowSched[TUE].timeSlot[F0].endMin = 00;

    me->schedule.dowSched[TUE].timeSlot[F1].setpointTemp = 21.0;
    me->schedule.dowSched[TUE].timeSlot[F1].startHour = 06;
    me->schedule.dowSched[TUE].timeSlot[F1].startMin = 00;
    me->schedule.dowSched[TUE].timeSlot[F1].endHour = 10;
    me->schedule.dowSched[TUE].timeSlot[F1].endMin = 30;

    me->schedule.dowSched[TUE].timeSlot[F2].setpointTemp = 19.0;
    me->schedule.dowSched[TUE].timeSlot[F2].startHour = 10;
    me->schedule.dowSched[TUE].timeSlot[F2].startMin = 30;
    me->schedule.dowSched[TUE].timeSlot[F2].endHour = 15;
    me->schedule.dowSched[TUE].timeSlot[F2].endMin = 00;

    me->schedule.dowSched[TUE].timeSlot[F3].setpointTemp = 21.0;
    me->schedule.dowSched[TUE].timeSlot[F3].startHour = 15;
    me->schedule.dowSched[TUE].timeSlot[F3].startMin = 00;
    me->schedule.dowSched[TUE].timeSlot[F3].endHour = 22;
    me->schedule.dowSched[TUE].timeSlot[F3].endMin = 00;

    me->schedule.dowSched[TUE].timeSlot[F4].setpointTemp = 19.0;
    me->schedule.dowSched[TUE].timeSlot[F4].startHour = 22;
    me->schedule.dowSched[TUE].timeSlot[F4].startMin = 00;
    me->schedule.dowSched[TUE].timeSlot[F4].endHour = 00;
    me->schedule.dowSched[TUE].timeSlot[F4].endMin = 00;

    // MIERCOLES
    me->schedule.dowSched[WED].timeSlot[F0].setpointTemp = 19.0;
    me->schedule.dowSched[WED].timeSlot[F0].startHour = 00;
    me->schedule.dowSched[WED].timeSlot[F0].startMin = 00;
    me->schedule.dowSched[WED].timeSlot[F0].endHour = 06;
    me->schedule.dowSched[WED].timeSlot[F0].endMin = 00;

    me->schedule.dowSched[WED].timeSlot[F1].setpointTemp = 21.0;
    me->schedule.dowSched[WED].timeSlot[F1].startHour = 06;
    me->schedule.dowSched[WED].timeSlot[F1].startMin = 00;
    me->schedule.dowSched[WED].timeSlot[F1].endHour = 10;
    me->schedule.dowSched[WED].timeSlot[F1].endMin = 30;

    me->schedule.dowSched[WED].timeSlot[F2].setpointTemp = 19.0;
    me->schedule.dowSched[WED].timeSlot[F2].startHour = 10;
    me->schedule.dowSched[WED].timeSlot[F2].startMin = 30;
    me->schedule.dowSched[WED].timeSlot[F2].endHour = 15;
    me->schedule.dowSched[WED].timeSlot[F2].endMin = 00;

    me->schedule.dowSched[WED].timeSlot[F3].setpointTemp = 21.0;
    me->schedule.dowSched[WED].timeSlot[F3].startHour = 15;
    me->schedule.dowSched[WED].timeSlot[F3].startMin = 00;
    me->schedule.dowSched[WED].timeSlot[F3].endHour = 22;
    me->schedule.dowSched[WED].timeSlot[F3].endMin = 00;

    me->schedule.dowSched[WED].timeSlot[F4].setpointTemp = 19.0;
    me->schedule.dowSched[WED].timeSlot[F4].startHour = 22;
    me->schedule.dowSched[WED].timeSlot[F4].startMin = 00;
    me->schedule.dowSched[WED].timeSlot[F4].endHour = 00;
    me->schedule.dowSched[WED].timeSlot[F4].endMin = 00;

    // JUEVES
    me->schedule.dowSched[THU].timeSlot[F0].setpointTemp = 19.0;
    me->schedule.dowSched[THU].timeSlot[F0].startHour = 00;
    me->schedule.dowSched[THU].timeSlot[F0].startMin = 00;
    me->schedule.dowSched[THU].timeSlot[F0].endHour = 06;
    me->schedule.dowSched[THU].timeSlot[F0].endMin = 00;

    me->schedule.dowSched[THU].timeSlot[F1].setpointTemp = 21.0;
    me->schedule.dowSched[THU].timeSlot[F1].startHour = 06;
    me->schedule.dowSched[THU].timeSlot[F1].startMin = 00;
    me->schedule.dowSched[THU].timeSlot[F1].endHour = 10;
    me->schedule.dowSched[THU].timeSlot[F1].endMin = 30;

    me->schedule.dowSched[THU].timeSlot[F2].setpointTemp = 19.0;
    me->schedule.dowSched[THU].timeSlot[F2].startHour = 10;
    me->schedule.dowSched[THU].timeSlot[F2].startMin = 30;
    me->schedule.dowSched[THU].timeSlot[F2].endHour = 15;
    me->schedule.dowSched[THU].timeSlot[F2].endMin = 00;

    me->schedule.dowSched[THU].timeSlot[F3].setpointTemp = 21.0;
    me->schedule.dowSched[THU].timeSlot[F3].startHour = 15;
    me->schedule.dowSched[THU].timeSlot[F3].startMin = 00;
    me->schedule.dowSched[THU].timeSlot[F3].endHour = 22;
    me->schedule.dowSched[THU].timeSlot[F3].endMin = 00;

    me->schedule.dowSched[THU].timeSlot[F4].setpointTemp = 19.0;
    me->schedule.dowSched[THU].timeSlot[F4].startHour = 22;
    me->schedule.dowSched[THU].timeSlot[F4].startMin = 00;
    me->schedule.dowSched[THU].timeSlot[F4].endHour = 00;
    me->schedule.dowSched[THU].timeSlot[F4].endMin = 00;

    // VIERNES
    me->schedule.dowSched[FRI].timeSlot[F0].setpointTemp = 19.0;
    me->schedule.dowSched[FRI].timeSlot[F0].startHour = 00;
    me->schedule.dowSched[FRI].timeSlot[F0].startMin = 00;
    me->schedule.dowSched[FRI].timeSlot[F0].endHour = 06;
    me->schedule.dowSched[FRI].timeSlot[F0].endMin = 00;

    me->schedule.dowSched[FRI].timeSlot[F1].setpointTemp = 21.0;
    me->schedule.dowSched[FRI].timeSlot[F1].startHour = 06;
    me->schedule.dowSched[FRI].timeSlot[F1].startMin = 00;
    me->schedule.dowSched[FRI].timeSlot[F1].endHour = 10;
    me->schedule.dowSched[FRI].timeSlot[F1].endMin = 30;

    me->schedule.dowSched[FRI].timeSlot[F2].setpointTemp = 19.0;
    me->schedule.dowSched[FRI].timeSlot[F2].startHour = 10;
    me->schedule.dowSched[FRI].timeSlot[F2].startMin = 30;
    me->schedule.dowSched[FRI].timeSlot[F2].endHour = 15;
    me->schedule.dowSched[FRI].timeSlot[F2].endMin = 00;

    me->schedule.dowSched[FRI].timeSlot[F3].setpointTemp = 21.0;
    me->schedule.dowSched[FRI].timeSlot[F3].startHour = 15;
    me->schedule.dowSched[FRI].timeSlot[F3].startMin = 00;
    me->schedule.dowSched[FRI].timeSlot[F3].endHour = 22;
    me->schedule.dowSched[FRI].timeSlot[F3].endMin = 00;

    me->schedule.dowSched[FRI].timeSlot[F4].setpointTemp = 19.0;
    me->schedule.dowSched[FRI].timeSlot[F4].startHour = 22;
    me->schedule.dowSched[FRI].timeSlot[F4].startMin = 00;
    me->schedule.dowSched[FRI].timeSlot[F4].endHour = 00;
    me->schedule.dowSched[FRI].timeSlot[F4].endMin = 00;
}

#pragma CODE_SECTION(stateMachineThread, ".TI.ramfunc")
void* stateMachineThread(void *arg0)
{
sem_wait(&startStateMachine);
stateMachine = malloc(sizeof(StateMachine));
StateMachine_init(stateMachine);
while (1)
{
    //DO NOT DECLARE VARIABLES INSIDE while(1) SCOPE. A HARD FAULT IS RAISED OTHERWISE!
    xQueueReceive(stateMachineEventQueue, &stateMachine->smEvent, portMAX_DELAY);
    //Clear structures
    stateMachine->dcEvt.textR1 = NULL;
    stateMachine->dcEvt.textR1Length = 0;
    stateMachine->dcEvt.textR2 = NULL;
    stateMachine->dcEvt.textR2Length = 0;
    strcpy(stateMachine->text1, "");
    strcpy(stateMachine->text2, "");
    if (stateMachine->stateMachineCurrState == IDLE_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            targetTemp->temperature += 0.5;
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            TempController_updateHeatingState(tempController);
        }
        if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            targetTemp->temperature -= 0.5;
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            TempController_updateHeatingState(tempController);
        }
        if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Ajustes fecha");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_DATE_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == DISPLAY_SET_DATE_STATE)
    {
        if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Ajustes Prog");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
        }
        if (stateMachine->smEvent.eventType == OK_BTN_PRESSED)
        {

            ds3231_read(ds3231hdl, DAY_OF_WEEK, &stateMachine->dowIdx);
            strcpy(stateMachine->text1, "Dia Semana");
            sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = SET_DOW_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == DISPLAY_SET_PROG_STATE)
    {
        if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        if (stateMachine->smEvent.eventType == OK_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Estado prog");
            sprintf(stateMachine->text2, "%s", progStatus[stateMachine->progState]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == SET_DOW_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->dowIdx;
            stateMachine->dowIdx %= NUM_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
            strcpy(stateMachine->text1, "Dia Semana");
            sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);

        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->dowIdx;
            if (stateMachine->dowIdx > (NUM_DOW - 1))
                stateMachine->dowIdx = NUM_DOW - 1; //If me->dowIdx is greater than 6, set it to 6 again.
            strcpy(stateMachine->text1, "Dia Semana");
            sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Ajustes Prog");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            ds3231_set(ds3231hdl, DAY_OF_WEEK, &stateMachine->dowIdx); //Set DOW
            ds3231_read(ds3231hdl, HOUR, &stateMachine->hour); //Read me->hour
            strcpy(stateMachine->text1, "Horas");
            sprintf(stateMachine->text2, "%02d", stateMachine->hour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = SET_HOUR_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == SET_HOUR_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->hour;
            stateMachine->hour %= 24; //If me->hour is greater than 23, set it to 0 again.
            strcpy(stateMachine->text1, "Horas");
            sprintf(stateMachine->text2, "%02d", stateMachine->hour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->hour;
            if (stateMachine->hour > 23)
                stateMachine->hour = 23; //If me->hour is greater than 23, set it to 23 again.
            strcpy(stateMachine->text1, "Horas");
            sprintf(stateMachine->text2, "%02d", stateMachine->hour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Ajustes Prog");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            ds3231_set(ds3231hdl, HOUR, &stateMachine->hour);
            ds3231_read(ds3231hdl, MINUTE, &stateMachine->minute); //Read me->hour
            strcpy(stateMachine->text1, "Minutos");
            sprintf(stateMachine->text2, "%02d", stateMachine->minute);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = SET_MINUTES_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == SET_MINUTES_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->minute;
            stateMachine->minute %= 60; //If me->hour is greater than 59, set it to 0 again.
            strcpy(stateMachine->text1, "Minutos");
            sprintf(stateMachine->text2, "%02d", stateMachine->minute);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->minute;
            if (stateMachine->minute > 59)
                stateMachine->minute = 59; //If me->hour is greater than 59, set it to 59 again.
            strcpy(stateMachine->text1, "Minutos");
            sprintf(stateMachine->text2, "%02d", stateMachine->minute);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            strcpy(stateMachine->text1, "Ajustes Prog");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            ds3231_set(ds3231hdl, MINUTE, &stateMachine->minute);
            strcpy(stateMachine->text1, "Ajustes fecha");
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->stateMachineCurrState = DISPLAY_SET_DATE_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == PROG_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->progState;
            stateMachine->progState %= NUM_PROG_STATES; //If me->dowIdx is greater than 1, set it to 0 again.
            strcpy(stateMachine->text1, "Estado prog");
            sprintf(stateMachine->text2, "%s", progStatus[stateMachine->progState]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->progState;
            if (stateMachine->progState > (NUM_PROG_STATES - 1))
                stateMachine->progState = OFF; //If me->dowIdx is greater than 1, set it to 1 again.
            strcpy(stateMachine->text1, "Estado prog");
            sprintf(stateMachine->text2, "%s", progStatus[stateMachine->progState]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            if (stateMachine->progState == ON) //Schedule is enabled
            {
                ds3231_read(ds3231hdl, DAY_OF_WEEK, &stateMachine->dowIdx);
                strcpy(stateMachine->text1, "Dia Semana");
                sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
                stateMachine->dcEvt.eventType = PRINT_DATA;
                stateMachine->dcEvt.textR1 = stateMachine->text1;
                stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
                stateMachine->dcEvt.textR2 = stateMachine->text2;
                stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
                stateMachine->stateMachineCurrState = PROG_SET_DOW_STATE;

            }
            else    //Schedule is disabled
            {
                strcpy(stateMachine->text1, "Ajustes Prog");
                stateMachine->dcEvt.eventType = PRINT_DATA;
                stateMachine->dcEvt.textR1 = stateMachine->text1;
                stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
                stateMachine->stateMachineCurrState = DISPLAY_SET_PROG_STATE;
            }
        }

    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_DOW_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->dowIdx;
            stateMachine->dowIdx %= NUM_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
            strcpy(stateMachine->text1, "Dia Semana");
            sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);

        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->dowIdx;
            if (stateMachine->dowIdx > (NUM_DOW - 1))
                stateMachine->dowIdx = NUM_DOW - 1; //If me->dowIdx is greater than 6, set it to 6 again.
            strcpy(stateMachine->text1, "Dia Semana");
            sprintf(stateMachine->text2, "%s", daysOfWeek[stateMachine->dowIdx]);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            stateMachine->timeSlotIdx = 0;

            sprintf(stateMachine->text1,
                    "F%d = %02u:%02u-%02u:%02u",
                    stateMachine->timeSlotIdx,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);

            sprintf(stateMachine->text2,
                    "%.1f'C",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);

            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_TIME_SLOT_STATE;
        }

    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_TIME_SLOT_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            ++stateMachine->timeSlotIdx;
            stateMachine->timeSlotIdx %= NUM_TIME_SLOTS_PER_DOW; //If me->dowIdx is greater than 6, set it to 0 again.
            sprintf(stateMachine->text1,
                    "F%d = %02u:%02u-%02u:%02u",
                    stateMachine->timeSlotIdx,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);

            //Show second timeslot (F2)
            sprintf(stateMachine->text2,
                    "%.1f'C",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);

            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            --stateMachine->timeSlotIdx;
            if (stateMachine->timeSlotIdx > (NUM_TIME_SLOTS_PER_DOW - 1))
                stateMachine->timeSlotIdx = NUM_TIME_SLOTS_PER_DOW - 1; //If me->timeSlotIdx is greater than NUM_TIME_SLOTS_PER_DOW-1, set it to NUM_TIME_SLOTS_PER_DOW-1 again.
            sprintf(stateMachine->text1,
                    "F%d = %02u:%02u-%02u:%02u",
                    stateMachine->timeSlotIdx,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);

            //Show second timeslot (F2)
            sprintf(stateMachine->text2,
                    "%.1f'C",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);

            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            sprintf(stateMachine->text1, "Temp deseada F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2,
                    "%.1f",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_SETPOINT_STATE;
        }
    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_SETPOINT_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            float *setpointTemp =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);
            (*setpointTemp) += 0.5;
            sprintf(stateMachine->text1, "Temp deseada F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%.1f", *setpointTemp);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);

        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            float *setpointTemp =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);
            (*setpointTemp) -= 0.5;
            sprintf(stateMachine->text1, "Temp deseada F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%.1f", *setpointTemp);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);

        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            sprintf(stateMachine->text1, "Hora inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2,
                    "%02u",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_START_HOUR_STATE;
        }

    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_START_HOUR_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            uint8_t *startHour =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour);
            ++(*startHour);
            (*startHour) %= 24;
            sprintf(stateMachine->text1, "Hora inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *startHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            uint8_t *startHour =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour);
            --(*startHour);
            if (*startHour > 23)
                *startHour = 23; //If me->hour is greater than 23, set it to 23 again.
            sprintf(stateMachine->text1, "Hora inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *startHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            sprintf(stateMachine->text1, "Minuto inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2,
                    "%02u",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_START_MINUTES_STATE;
        }

    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_START_MINUTES_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            uint8_t *startMin =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin);
            ++(*startMin);
            (*startMin) %= 60;
            sprintf(stateMachine->text1, "Minuto inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *startMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            uint8_t *startMin =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin);
            --(*startMin);
            if (*startMin > 59)
                *startMin = 59; //If me->hour is greater than 59, set it to 59 again.
            sprintf(stateMachine->text1, "Minuto inicio F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *startMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            sprintf(stateMachine->text1, "Hora fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2,
                    "%02u",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_START_MINUTES_STATE;
            stateMachine->stateMachineCurrState = PROG_SET_END_HOUR_STATE;
        }

    }
    else if (stateMachine->stateMachineCurrState == PROG_SET_END_HOUR_STATE)
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            uint8_t *endHour =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour);
            ++(*endHour);
            (*endHour) %= 24;
            sprintf(stateMachine->text1, "Hora fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *endHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            uint8_t *endHour =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour);
            --(*endHour);
            if (*endHour > 23)
                *endHour = 23; //If me->hour is greater than 23, set it to 23 again.
            sprintf(stateMachine->text1, "Hora fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *endHour);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            sprintf(stateMachine->text1, "Minuto fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2,
                    "%02u",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_END_MINUTES_STATE;
        }
    }
    else
    {
        if (stateMachine->smEvent.eventType == INC_BTN_PRESSED)
        {
            uint8_t *endMin =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);
            ++(*endMin);
            (*endMin) %= 60;
            sprintf(stateMachine->text1, "Minuto fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *endMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == DEC_BTN_PRESSED)
        {
            uint8_t *endMin =
                    &(stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);
            --(*endMin);
            if (*endMin > 59)
                *endMin = 59; //If me->hour is greater than 59, set it to 59 again.
            sprintf(stateMachine->text1, "Minuto fin F%d", stateMachine->timeSlotIdx);
            sprintf(stateMachine->text2, "%02u", *endMin);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
        }
        else if (stateMachine->smEvent.eventType == MODE_BTN_PRESSED)
        {
            sprintf(stateMachine->text1, "T. Obj:%.1f'C", targetTemp->temperature);
            sprintf(stateMachine->text2, "T. Act:%.1f'C", readTemp->temperature);
            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = IDLE_STATE;
        }
        else //if me->smEvent.eventType == OK_BTN_PRESSED
        {
            //Update schedule configuration in Flash.
            //memcpy(&schedule, &me->schedule, sizeof(schedule_t));
            FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,
            FLASH_SECTOR31);
            while (!FlashCtl_eraseSector(SCHEDULE_FLASH_START))
                ;
            while (!FlashCtl_programMemory(&stateMachine->schedule,
                                           (void*) SCHEDULE_FLASH_START,
                                           sizeof(schedule_t)))
                ;
            FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,
            FLASH_SECTOR31);

            sprintf(stateMachine->text1,
                    "F%d = %02u:%02u-%02u:%02u",
                    stateMachine->timeSlotIdx,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].startMin,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endHour,
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].endMin);

            sprintf(stateMachine->text2,
                    "%.1f'C",
                    stateMachine->schedule.dowSched[stateMachine->dowIdx].timeSlot[stateMachine->timeSlotIdx].setpointTemp);

            stateMachine->dcEvt.eventType = PRINT_DATA;
            stateMachine->dcEvt.textR1 = stateMachine->text1;
            stateMachine->dcEvt.textR1Length = strlen(stateMachine->text1);
            stateMachine->dcEvt.textR2 = stateMachine->text2;
            stateMachine->dcEvt.textR2Length = strlen(stateMachine->text2);
            stateMachine->stateMachineCurrState = PROG_SET_TIME_SLOT_STATE;
        }

    }
    DisplayClient_updateNextBacklightOffTime(displayClient, rtc->secondsCount);
    xQueueSend(displayClientEventQueue, &stateMachine->dcEvt, 0);

    Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
}
}


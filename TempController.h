/*
 * TempController.h
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#ifndef TEMPCONTROLLER_H_
#define TEMPCONTROLLER_H_

#include "TempSensor.h"
#include <stdio.h>
#include "InterThreadQueues.h"
#include "shared_vars.h"
#include "Keypad.h"
#include "ThreadsArgStruct.h"


typedef struct TempController TempController;

struct TempController{
    TempData targetTemp;
    TempData readTemp;
    //These ones are necessary
    QueueHandle_t qDispConsole;
    QueueHandle_t qTReadToTCtrl;
    QueueHandle_t qKeypadToTCtrl;
    QueueHandle_t qTCtrlToLCD;
};

void TempController_init(TempController *const me);
void TempController_clean(TempController* const me);
TempController* TempController_create(void);
void TempController_destroy(TempController* const me);

#endif /* TEMPCONTROLLER_H_ */

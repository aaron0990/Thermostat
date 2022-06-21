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
    TempSensor* itsTempSensor;
    Keypad* itsKeypad;
    TempData readTemp;
    TempData* targetTemp;

    //These ones are necessary
    QueueHandle_t qDispConsole;
    QueueHandle_t qTReadToTCtrl;
    QueueHandle_t qKeypadToTCtrl;
};

void TempController_init(TempController *const me, TempSensor *const ts, Keypad *const kp);
void TempController_clean(TempController* const me);
TempController* TempController_create(void);
void TempController_destroy(TempController* const me);

#endif /* TEMPCONTROLLER_H_ */

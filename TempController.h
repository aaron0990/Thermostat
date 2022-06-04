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
#include "shared_vars.h"

typedef struct TempController TempController;

struct TempController{
    TempSensor* itsTempSensor;
    TempData* readTemp;
    TempData* targetTemp;
};

void TempController_init(TempController *const me, TempSensor *const ts);
void TempController_clean(TempController* const me);
void TempController_accept(TempController* const me, TempData* sensedTemp);
void TempController_register(TempController* const me); //it calls the subscribe() function of the sensor
TempController* TempController_create(void);
void TempController_destroy(TempController* const me);

//Getters and setters
TempData* TempController_getItsTempData(TempController* const me);
void TempController_setItsTempData(TempController* const me, TempData* p_td);
TempSensor* TempController_getItsTempSensor(TempController* const me);
void TempController_setItsTempSensor(TempController* const me, TempSensor* p_ts);
void TempController_setTargetTemp(TempController* const me, TempData* tgtTmp);

#endif /* TEMPCONTROLLER_H_ */

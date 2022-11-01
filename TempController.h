/*
 * TempController.h
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#ifndef TEMPCONTROLLER_H_
#define TEMPCONTROLLER_H_

#include "shared_vars.h"
#include <stdio.h>
#include "Keypad.h"

typedef struct {
    TempData* targetTemp;
    TempData* readTemp;
    uint8_t heatingOn;
} TempController;

void TempController_init(TempController *const me, TempData* readTemp, TempData* targetTemp);
void TempController_clean(TempController* const me);
void TempController_setReadTemp(TempController *const me, TempData* td);
void TempController_setTargetTemp(TempController *const me, TempData* td);
void TempController_updateHeatingState(TempController *const me);
TempController* TempController_create(void);
void TempController_destroy(TempController* const me);

#endif /* TEMPCONTROLLER_H_ */

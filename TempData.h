/*
 * TempData.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef TEMPDATA_H_
#define TEMPDATA_H_

#include <stdint.h>
#include "shared_vars.h"

typedef struct TempData TempData;
struct TempData
{
    int8_t temperature;
    uint8_t humidity;
};

void TempData_init(TempData *const me);
void TempData_clean(TempData *const me);
TempData* TempData_create(void);
void TempData_destroy(TempData *const me);

#endif /* TEMPDATA_H_ */

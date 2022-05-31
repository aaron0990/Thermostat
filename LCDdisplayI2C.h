/*
 * LCDdisplayI2C.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef LCDDISPLAYI2C_H_
#define LCDDISPLAYI2C_H_

#include <i2c.h>
#include <ti/drivers/I2C.h>
#include <gpio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "shared_vars.h"

#define I2C_MODULE          EUSCI_B0_BASE //Board PINS: SDA->1.6, SCL->1.7
#define SLAVE_ADDR          0x27

typedef struct
{
    I2C_Handle *i2cHandle;
    I2C_Params *i2cParams;
    I2C_Transaction *i2cTransaction;
} DisplayI2C;

extern DisplayI2C* DisplayI2C_create(void);
extern void DisplayI2C_initialize(DisplayI2C *const me);
extern void DisplayI2C_send(DisplayI2C *const me, uint8_t data);

#endif /* LCDDISPLAYI2C_H_ */

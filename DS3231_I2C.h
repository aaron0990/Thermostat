/*
 * DS3231_I2C.h
 *
 *  Created on: 25 oct. 2022
 *      Author: aaron
 */

#ifndef DS3231_I2C_H_
#define DS3231_I2C_H_

#include <i2c.h>
#include <ti/drivers/I2C.h>
#include <gpio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "shared_vars.h"

#define I2C_MODULE          EUSCI_B0_BASE //Board PINS: SDA->1.6, SCL->1.7

typedef struct
{
    I2C_Handle *i2cHandle;
    I2C_Params *i2cParams;
    I2C_Transaction *i2cTransaction;
} DS3231_I2C;

DS3231_I2C* DS3231_I2C_create(void);
void DS3231_I2C_init(DS3231_I2C *const me);
void DS3231_I2C_write_single(DS3231_I2C *const me, uint8_t device_address, uint8_t register_address, uint8_t *data_byte);
void DS3231_I2C_write_multi(DS3231_I2C *const me, uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length);
void DS3231_I2C_read_single(DS3231_I2C *const me, uint8_t device_address, uint8_t register_address, uint8_t *data_byte);
void DS3231_I2C_read_multi(DS3231_I2C *const me, uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length);


#endif /* DS3231_I2C_H_ */

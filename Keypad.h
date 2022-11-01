/*
 * Keypad.h
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#define MAX_SUBSCRIBERS 100

#include "TempData.h"
#include <ti/drivers/GPIO.h>
#include <string.h>
#include <timer_a.h>
#include <interrupt.h>
#include <unistd.h>
#include <gpio.h>
#include <stdlib.h>
#include "shared_vars.h"
#include <sysctl.h>

#define INC_BUTTON_PIN_IDX  0
#define DEC_BUTTON_PIN_IDX  1
#define MODE_BUTTON_PIN_IDX 2
#define OK_BUTTON_PIN_IDX   3

/*Class Keypad*/
typedef struct Keypad Keypad;
struct Keypad
{
    uint8_t padding;
};

/*Constructors and destructors*/
Keypad* Keypad_create(void);
void Keypad_init(Keypad *const me);
void Keypad_clean(Keypad *const me);

/*Operations*/
void Keypad_readTemp(Keypad *const me);
void Keypad_newData(Keypad *const me);
void Keypad_destroy(Keypad *const me);
void Keypad_InterruptHandler(uint_least8_t idx);


#endif /* KEYPAD_H_ */

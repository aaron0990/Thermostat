/*
 * utils.h
 *
 *  Created on: 10 feb. 2022
 *      Author: aaron
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <timer32.h>
#include <cs.h>
#include "shared_vars.h"

/*
 * * NOTE: for seconds and milliseconds resolution, you can use sleep() and usleep() functions from unistd.h library instead.
 */

/* Delay execution for sec seconds. */
void delay(uint32_t sec);

/* Delay execution for usec microseconds. */
void delay_us(uint32_t usec);

#endif /* UTILS_H_ */

/*
 * RTC.h
 *
 *  Created on: 9 sept. 2022
 *      Author: aaron
 */

#ifndef RTC_H_
#define RTC_H_

#include <semaphore.h>

extern sem_t startReadingTemp;

typedef struct RTC RTC;
struct RTC{
    uint32_t secondsCount;
    uint32_t timerHandlers[16];
};

RTC* RTC_C_create(void);
void RTC_C_configure(RTC* const me);
void RTC_C_destroy(RTC* const me);
void RTC_C_init(RTC* const me);
int RTC_C_isTimerExpired(RTC* const me, uint32_t timestamp);
void RTC_C_IRQHandler(uint32_t arg);

#endif /* RTC_H_ */

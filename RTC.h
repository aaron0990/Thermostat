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
    float tempReadPeriod; //In minutes
    uint32_t secondsCount; //When secondsCount == tempReadPeriod*60 -> Fire temp reading
};

void RTC_C_IRQHandler(uint32_t arg);
RTC* RTC_C_create(void);
void RTC_C_configure(RTC* const me, float newTmpReadPeriod);
void RTC_C_destroy(RTC* const me);
void RTC_C_init(RTC* const me);


#endif /* RTC_H_ */

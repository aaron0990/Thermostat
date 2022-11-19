/*ds3231 driver header file - Reza Ebrahimi v1.0*/
#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>
#include "DS3231_I2C.h"
#include "utils.h"

enum options
{
    SECOND,
    MINUTE,
    HOUR,
    DAY_OF_WEEK,
    DATE,
    MONTH,
    YEAR,
    CONTROL,
    CONTROL_STATUS,
    AGING_OFFSET,
    ALARM1,
    ALARM2,
    ALARMS,
    TEMPERATURE,
    TIME,
    ALL
};
enum square_wave
{
    WAVE_OFF, WAVE_1, WAVE_2, WAVE_3, WAVE_4
};
enum run_state
{
    CLOCK_HALT, CLOCK_RUN
};
enum ds3231_registers
{
    DS3231_REGISTER_SECONDS,
    DS3231_REGISTER_MINUTES,
    DS3231_REGISTER_HOURS,
    DS3231_REGISTER_DAY_OF_WEEK,
    DS3231_REGISTER_DATE,
    DS3231_REGISTER_MONTH,
    DS3231_REGISTER_YEAR,
    DS3231_REGISTER_ALARM1_SECONDS,
    DS3231_REGISTER_ALARM1_MINUTES,
    DS3231_REGISTER_ALARM1_HOURS,
    DS3231_REGISTER_ALARM1_DAY_OF_WEEK_OR_DATE,
    DS3231_REGISTER_ALARM2_MINUTES,
    DS3231_REGISTER_ALARM2_HOURS,
    DS3231_REGISTER_ALARM2_DAY_OF_WEEK_OR_DATE,
    DS3231_REGISTER_CONTROL,
    DS3231_REGISTER_CONTROL_STATUS,
    DS3231_REGISTER_AGING_OFFSET,
    DS3231_REGISTER_ALARM2_TEMP_MSB,
    DS3231_REGISTER_ALARM2_TEMP_LSB
};

#define DS3231_I2C_ADDRESS                    0X68

#define FORCE_RESET                           0X00
#define NO_FORCE_RESET                        0X01
#define NO_RESET                              0X02
#define DS3231_IS_RUNNING                     0X01
#define DS3231_IS_STOPPED                     0X00
#define OPERATION_DONE                        0X01
#define OPERATION_FAILED                      0X00
#define DS3231_NOT_INITIALIZED                0X01        /*bit OSF == 1 indicates that the oscillator was stopped*/
#define DS3231_INITIALIZED                    0X00        /*bit OSF == 0 indicates that the oscillator was running before mcu was powered on*/

#define DS3231_BIT_12_24                      0X06
#define DS3231_BIT_CENTURY                    0X07
#define DS3231_BIT_A1M1                       0X07
#define DS3231_BIT_A1M2                       0X07
#define DS3231_BIT_A1M3                       0X07
#define DS3231_BIT_A1M4                       0X07
#define DS3231_BIT_A2M2                       0X07
#define DS3231_BIT_A3M3                       0X07
#define DS3231_BIT_A4M4                       0X07
#define DS3231_BIT_12_24_ALARM1               0X06
#define DS3231_BIT_12_24_ALARM2               0X06
#define DS3231_BIT_DY_DT_ALARM1               0X06
#define DS3231_BIT_DY_DT_ALARM2               0X06
#define DS3231_BIT_A1IE                       0X00
#define DS3231_BIT_A2IE                       0X01
#define DS3231_BIT_INTCN                      0X02
#define DS3231_BIT_RS1                        0X03
#define DS3231_BIT_RS2                        0X04
#define DS3231_BIT_CONV                       0X05
#define DS3231_BIT_BBSQW                      0X06
#define DS3231_BIT_EOSC                       0X07
#define DS3231_BIT_A1F                        0X00
#define DS3231_BIT_A2F                        0X01
#define DS3231_BIT_BSY                        0X02
#define DS3231_BIT_EN32KHZ                    0X03
#define DS3231_BIT_OSF                        0X07

#define DS3231_REGISTER_SECONDS_DEFAULT                       0X00
#define DS3231_REGISTER_MINUTES_DEFAULT                       0X00
#define DS3231_REGISTER_HOURS_DEFAULT                         0X00
#define DS3231_REGISTER_DAY_OF_WEEK_DEFAULT                   0X01
#define DS3231_REGISTER_DATE_DEFAULT                          0X01
#define DS3231_REGISTER_MONTH_DEFAULT                         0X01
#define DS3231_REGISTER_YEAR_DEFAULT                          0X00
#define DS3231_REGISTER_ALARM1_SECONDS_DEFAULT                0X00
#define DS3231_REGISTER_ALARM1_MINUTES_DEFAULT                0X00            
#define DS3231_REGISTER_ALARM1_HOURS_DEFAULT                  0X00
#define DS3231_REGISTER_ALARM1_DAY_OF_WEEK_OR_DATE_DEFAULT    0X00
#define DS3231_REGISTER_ALARM2_MINUTES_DEFAULT                0X00
#define DS3231_REGISTER_ALARM2_HOURS_DEFAULT                  0X00    
#define DS3231_REGISTER_ALARM2_DAY_OF_WEEK_OR_DATE_DEFAULT    0X00
#define DS3231_REGISTER_CONTROL_DEFAULT                       0X1C
#define DS3231_REGISTER_CONTROL_STATUS_DEFAULT                0X00
#define DS3231_REGISTER_AGING_OFFSET_DEFAULT                  0X00

typedef enum
{
    READ_TIME,
} DS3231EventType;

typedef struct
{
    DS3231EventType eventType;
} DS3231Event;

typedef struct
{
    DS3231_I2C *ds3231I2C;
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t dow;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint32_t nextMinuteCheck;
} DS3231Proxy;

extern QueueHandle_t ds3231EventQueue; //Queue used to pass events to the DisplayClient.

DS3231Proxy* ds3231_create(void);
void ds3231_reset(DS3231Proxy *ds3231hdl, uint8_t input);
void ds3231_init(DS3231Proxy *ds3231hdl, uint8_t *data_array,
                 uint8_t run_command, uint8_t reset_state);
void ds3231_init_status_update(DS3231Proxy *ds3231hdl);
uint8_t ds3231_read(DS3231Proxy *ds3231hdl, uint8_t registers,
                    uint8_t *data_array);
uint8_t ds3231_set(DS3231Proxy *ds3231hdl, uint8_t registers,
                   uint8_t *data_array);
uint8_t ds3231_init_status_report(DS3231Proxy *ds3231hdl);
uint8_t ds3231_run_command(DS3231Proxy *ds3231hdl, uint8_t command);
uint8_t ds3231_run_status(DS3231Proxy *ds3231hdl);

void ds3231_updateNextMinuteCheck(DS3231Proxy *ds3231hdl, uint32_t currentTime);

#endif

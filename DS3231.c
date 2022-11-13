/*DS3231 high level driver - Reza Ebrahimi v1.0*/
/*This is MCU independent; there's no need to change the contents of this file. Use low level api to adapt the driver to your MCU of choice*/
#include "DS3231.h"

static void BCD_to_HEX(DS3231Proxy* ds3231hdl, uint8_t *data_array, uint8_t array_length);        /*turns the bcd time values from ds3231 into hex*/
static void HEX_to_BCD(DS3231Proxy* ds3231hdl, uint8_t *data_array, uint8_t array_length);        /*turns the hex numbers into bcd, to be written back into ds3231*/
static void ds3231_data_clone(DS3231Proxy* ds3231hdl, uint8_t option, uint8_t *input_array);        /*clones an array into one of 3 ..._registers_clone[], based on chosen option*/

static uint8_t register_current_value;        /*used to read current values of ds3231 registers*/
static uint8_t register_new_value;        /*used to write new values to ds3231 registers*/
static uint8_t time_registers_clone[7];       /*used for the purpose of not curropting the time settings by reconverting an already converted HEX to BCD array*/
static uint8_t alarm1_registers_clone[4];       /*used for the purpose of not curropting the time settings by reconverting an already converted HEX to BCD array*/
static uint8_t alarm2_registers_clone[3];       /*used for the purpose of not curropting the time settings by reconverting an already converted HEX to BCD array*/
static uint8_t register_default_value[] = {       /*used in reset function, contains default values*/
  DS3231_REGISTER_SECONDS_DEFAULT,
  DS3231_REGISTER_MINUTES_DEFAULT,
  DS3231_REGISTER_HOURS_DEFAULT,
  DS3231_REGISTER_DAY_OF_WEEK_DEFAULT,
  DS3231_REGISTER_DATE_DEFAULT,
  DS3231_REGISTER_MONTH_DEFAULT,
  DS3231_REGISTER_YEAR_DEFAULT,
  DS3231_REGISTER_ALARM1_SECONDS_DEFAULT,
  DS3231_REGISTER_ALARM1_MINUTES_DEFAULT,
  DS3231_REGISTER_ALARM1_HOURS_DEFAULT,
  DS3231_REGISTER_ALARM1_DAY_OF_WEEK_OR_DATE_DEFAULT,
  DS3231_REGISTER_ALARM2_MINUTES_DEFAULT,
  DS3231_REGISTER_ALARM2_HOURS_DEFAULT,
  DS3231_REGISTER_ALARM2_DAY_OF_WEEK_OR_DATE_DEFAULT,
  DS3231_REGISTER_CONTROL_DEFAULT,
  DS3231_REGISTER_CONTROL_STATUS_DEFAULT,
  DS3231_REGISTER_AGING_OFFSET_DEFAULT,
};

DS3231Proxy* ds3231_create(void)
{
    DS3231Proxy* ds3231_proxy = (DS3231Proxy*)malloc(sizeof(DS3231Proxy));
    if (ds3231_proxy != NULL)
    {
        ds3231_proxy->ds3231I2C = DS3231_I2C_create();
    }
    return ds3231_proxy;
}

/*ds3231_init function accepts 3 inputs, data_array[7] is the new time settings,
  run_state commands ds3231 to run or halt (CLOCK_RUN and CLOCK_HALT), and reset_state
  could force-reset ds3231 (FORCE_RESET) or checks if ds3231 is reset beforehand
  (NO_FORCE_RESET)*/
void ds3231_init(DS3231Proxy* ds3231hdl, uint8_t *data_array, uint8_t run_command, uint8_t reset_state)
{
  GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN6); //DS3231 GND
  GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to LOW to power it on
  ds3231hdl->nextMinuteCheck = 0;
  delay_us(300000);
  DS3231_I2C_init(ds3231hdl->ds3231I2C);
  if (((ds3231_init_status_report(ds3231hdl) == DS3231_NOT_INITIALIZED) && (reset_state == NO_FORCE_RESET)) || (reset_state == FORCE_RESET))
  {
    ds3231_reset(ds3231hdl, ALL);
    ds3231_set(ds3231hdl, TIME, data_array);
  }
  ds3231_init_status_update(ds3231hdl);        /*now the device is initialized (DS3231_INITIALIZED)*/
  ds3231_run_command(ds3231hdl, run_command);
  GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to HIGH to power it off
}

/*function to command ds3231 to stop or start updating its time registers, WORKS ONLY WITH BATTERY BACKED DS3231*/
uint8_t ds3231_run_command(DS3231Proxy* ds3231hdl, uint8_t command)
{
  switch (command)
  {
    case CLOCK_RUN:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);
      register_new_value = register_current_value & (~(1 << DS3231_BIT_EOSC));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_new_value);
      return OPERATION_DONE;
    case CLOCK_HALT:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);
      register_new_value = register_current_value | (1 << DS3231_BIT_EOSC);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_new_value);
      return OPERATION_DONE;
    default:
      return OPERATION_FAILED;
  }
}

/*function to check the status of ds3231, whether its running or not. WORKS ONLY WITH BATTERY BACKED DS3231*/
uint8_t ds3231_run_status(DS3231Proxy* ds3231hdl)
{
  DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);
  if ((register_current_value && (1 << DS3231_BIT_EOSC)) == 0)
    return CLOCK_RUN;
  else
    return CLOCK_HALT;
}

/*function to read the oscillator flag OSF and to decide whether it has been reset beforehand or not*/
uint8_t ds3231_init_status_report(DS3231Proxy* ds3231hdl)
{
  DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);
  if (register_current_value & (1 << DS3231_BIT_OSF))
    return DS3231_NOT_INITIALIZED;
  else
    return DS3231_INITIALIZED;
}

/*function to reset the OSF bit (OSF = 0)*/
void ds3231_init_status_update(DS3231Proxy* ds3231hdl)
{
  DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);
  register_new_value = register_current_value & (~(1 << DS3231_BIT_OSF));
  DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_new_value);
}

/*resets the desired register(s), without affecting run_state (RUN_STATE ONLY MAKES SENSE WITH BATTERY-BACKED DS3231*/
void ds3231_reset(DS3231Proxy* ds3231hdl, uint8_t option)
{
  /*data_clone function together with registers_clone static variables, prevent data curroption caused by reconvertig HEX to BCD*/
  ds3231_data_clone(ds3231hdl, TIME, &register_default_value[0]);
  ds3231_data_clone(ds3231hdl, ALARM1, &register_default_value[7]);
  ds3231_data_clone(ds3231hdl, ALARM2, &register_default_value[0X0B]);
  HEX_to_BCD(ds3231hdl, &time_registers_clone[0], 7);
  HEX_to_BCD(ds3231hdl, &alarm1_registers_clone[0], 4);
  HEX_to_BCD(ds3231hdl, &alarm2_registers_clone[0], 3);
  switch (option)
  {
    case SECOND:
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &time_registers_clone[0]);
      break;
    case MINUTE:
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MINUTES, &time_registers_clone[1]);
      break;
    case HOUR:
      time_registers_clone[2] &= (~(1 << DS3231_BIT_12_24));        /*to turn on 24 hours format by default*/
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_HOURS, &time_registers_clone[2]);
      break;
    case DAY_OF_WEEK:
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DAY_OF_WEEK, &time_registers_clone[3]);
      break;
    case DATE:
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DATE, &time_registers_clone[4]);
      break;
    case MONTH:
      time_registers_clone[5] &= (~(1 << DS3231_BIT_CENTURY));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MONTH, &time_registers_clone[5]);
      break;
    case YEAR:
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_YEAR, &time_registers_clone[6]);
      break;
    case CONTROL:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);       /*in order to preserve running state (RUN or HALT)*/
      register_new_value = (register_current_value & (1 << DS3231_BIT_EOSC)) | (register_default_value[0X0E] & (~(1 << DS3231_BIT_EOSC)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_new_value);
      break;
    case CONTROL_STATUS:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);       /*in order to preserve OSF flag*/
      register_new_value = (register_current_value & (1 << DS3231_BIT_OSF)) | (register_default_value[0X0F] & (~(1 << DS3231_BIT_EOSC)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_new_value);
      break;
    case ALARM1:
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_ALARM1_SECONDS, &alarm1_registers_clone[0], 4);
      break;
    case ALARM2:
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_ALARM2_MINUTES, &alarm2_registers_clone[0], 3);
      break;
    case ALARMS:
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_ALARM1_SECONDS, &alarm1_registers_clone[0], 4);
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_ALARM2_MINUTES, &alarm2_registers_clone[0], 3);
      break;
    case AGING_OFFSET:
      register_new_value = DS3231_REGISTER_AGING_OFFSET_DEFAULT;
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_AGING_OFFSET, &register_new_value);
    case TIME:
      time_registers_clone[2] &= (~(1 << DS3231_BIT_12_24));
      time_registers_clone[5] &= (~(1 << DS3231_BIT_CENTURY));
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &time_registers_clone[0], 7);
      break;
    case ALL:
      /*TIME registers reset*/
      time_registers_clone[2] &= (~(1 << DS3231_BIT_12_24));        /*to preserve 24 hours mode*/
      time_registers_clone[5] &= (~(1 << DS3231_BIT_CENTURY));        /*resetting century bit*/
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &time_registers_clone[0], 7);       /*to reset all the TIME registers*/
      /*CONTROL and CONTROL_STATUS registers reset*/
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);       /*in order to preserve OSF flag*/
      register_new_value = (register_current_value & (1 << DS3231_BIT_OSF)) | (register_default_value[0X0F] & (~(1 << DS3231_BIT_EOSC)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_new_value);
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);       /*to preserve run_status, either RUN or HALT*/
      register_new_value = (register_current_value & (1 << DS3231_BIT_EOSC)) | (register_default_value[0X0E] & (~(1 << DS3231_BIT_EOSC)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_new_value);
      /*AGING_OFFSET registers reset*/
      register_new_value = DS3231_REGISTER_AGING_OFFSET_DEFAULT;
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_AGING_OFFSET, &register_new_value);
      break;
    default:
      break;
  }
}

/*function to read internal registers of ds3231, one register at a time or an array of registers*/
uint8_t ds3231_read(DS3231Proxy* ds3231hdl, uint8_t option, uint8_t *data_array)
{
  GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to LOW to power it on
  delay_us(300000);
  switch (option)
  {
    case SECOND:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case MINUTE:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MINUTES, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case HOUR:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_HOURS, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case DAY_OF_WEEK:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DAY_OF_WEEK, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case DATE:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DATE, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case MONTH:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MONTH, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case YEAR:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_YEAR, &register_current_value);
      *data_array = register_current_value;
      BCD_to_HEX(ds3231hdl, data_array, 1);
      break;
    case CONTROL:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);
      *data_array = register_current_value;
      break;
    case CONTROL_STATUS:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);
      *data_array = register_current_value;
      break;
    case AGING_OFFSET:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_AGING_OFFSET, &register_current_value);
      *data_array = register_current_value;
    case TIME:
      DS3231_I2C_read_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, data_array, 7);
      BCD_to_HEX(ds3231hdl, data_array, 7);
      break;
    default:
      return OPERATION_FAILED;
  }
  GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to HIGH to shut it down
  return OPERATION_DONE;
}

/*function to set internal registers of ds3231, one register at a time or an array of registers*/
uint8_t ds3231_set(DS3231Proxy* ds3231hdl, uint8_t option, uint8_t *data_array)
{
  GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to LOW to power it on
  delay_us(300000);
  switch (option)
  {
    case SECOND:
      time_registers_clone[0] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[0], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &time_registers_clone[0]);
      break;
    case MINUTE:
      time_registers_clone[1] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[1], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MINUTES, &time_registers_clone[1]);
      break;
    case HOUR:
      time_registers_clone[2] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[2], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_HOURS, &time_registers_clone[2]);
      break;
    case DAY_OF_WEEK:
      time_registers_clone[3] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[3], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DAY_OF_WEEK, &time_registers_clone[3]);
      break;
    case DATE:
      time_registers_clone[4] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[4], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_DATE, &time_registers_clone[4]);
      break;
    case MONTH:
      time_registers_clone[5] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[5], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_MONTH, &time_registers_clone[5]);
      break;
    case YEAR:
      time_registers_clone[6] = *data_array;
      HEX_to_BCD(ds3231hdl, &time_registers_clone[6], 1);
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_YEAR, &time_registers_clone[6]);
      break;
    case CONTROL:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_current_value);
      register_new_value = (register_current_value & (1 << DS3231_BIT_EOSC)) | (*data_array & (~(1 << DS3231_BIT_EOSC)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL, &register_new_value);
      break;
    case CONTROL_STATUS:
      DS3231_I2C_read_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_current_value);
      register_new_value = (register_current_value & (1 << DS3231_BIT_OSF)) | (*data_array & (~(1 << DS3231_BIT_OSF)));
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_CONTROL_STATUS, &register_new_value);
      break;                                                                                         
    case TIME:
      ds3231_data_clone(ds3231hdl,TIME, data_array);
      HEX_to_BCD(ds3231hdl, &time_registers_clone[0], 7);
      DS3231_I2C_write_multi(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_SECONDS, &time_registers_clone[0], 7);
      break;
    case AGING_OFFSET:
      register_new_value = *data_array;
      DS3231_I2C_write_single(ds3231hdl->ds3231I2C, DS3231_I2C_ADDRESS, DS3231_REGISTER_AGING_OFFSET, &register_new_value);
      break;
    default:
      return OPERATION_FAILED;
  }
  GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN6); //Set to HIGH to power it off
  return OPERATION_DONE;
}

/*to clone the desired data and prevent reconversion of BCD to HEX*/
static void ds3231_data_clone(DS3231Proxy* ds3231hdl, uint8_t option, uint8_t *input_array)
{
  uint8_t counter;
  switch (option)
  {
    case TIME:
      for (counter = 0; counter < 7; counter++)
      {
        time_registers_clone[counter] = input_array[counter];
      }
      break;
    case ALARM1:
      for (counter = 0; counter < 4; counter++)
      {
        alarm1_registers_clone[counter] = input_array[counter];
      }
      break;
    case ALARM2:
      for (counter = 0; counter < 3; counter++)
      {
        alarm2_registers_clone[counter] = input_array[counter];
      }
      break;
    default:
      break;
  }
}

void ds3231_updateNextMinuteCheck(DS3231Proxy* ds3231hdl, uint32_t currentTime)
{
    if (ds3231hdl != NULL)
    {
        ds3231hdl->nextMinuteCheck = currentTime + 60;
    }
}

/*internal function related to this file and not accessible from outside*/
static void BCD_to_HEX(DS3231Proxy* ds3231hdl, uint8_t *data_array, uint8_t array_length)
{
  int8_t index;
  for (index = (array_length - 1); index >= 0; index--)
  {
    data_array[index] = ((data_array[index] >> 4) << 1) + ((data_array[index] >> 4) << 3) + (data_array[index] & 0X0F);
  }
}

/*internal function related to this file and not accessible from outside*/
static void HEX_to_BCD(DS3231Proxy* ds3231hdl, uint8_t *data_array, uint8_t array_length)
{
  uint8_t temporary_value;
  int8_t index;
  for (index = (array_length - 1); index >= 0; index--)
  {
    temporary_value = 0;
    while (((int8_t)data_array[index] - 0X0A) >= 0)
    {
      temporary_value += 0X10;
      data_array[index] -= 0X0A;
    }
    temporary_value += data_array[index];
    data_array[index] = temporary_value;
  }
}

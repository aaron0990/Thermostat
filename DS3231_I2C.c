/*DS3231 low level API - Reza Ebrahimi v1.0*/
#include "DS3231_I2C.h"


DS3231_I2C* DS3231_I2C_create(void)
{
    DS3231_I2C* ds3231_i2c = (DS3231_I2C*)malloc(sizeof(DS3231_I2C));
    return ds3231_i2c;
}

/*function to transmit one byte of data to register_address on ds3231 (device_address: 0X68)*/
void DS3231_I2C_write_single(DS3231_I2C *const me, uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
    uint8_t buf[2] = {register_address, *data_byte};
    me->i2cTransaction->slaveAddress = device_address;
    me->i2cTransaction->writeBuf = buf;
    me->i2cTransaction->writeCount = 2;
    me->i2cTransaction->readCount = 0;
    int8_t ret = 0;
    do
    {
        ret = I2C_transfer(*(me->i2cHandle), me->i2cTransaction);
    }
    while (!ret);

    //I2C_close(*(me->i2cHandle));

    if (ret == I2C_STATUS_TIMEOUT)
    {
        return;
    }
    else if (ret == I2C_STATUS_ERROR)
    {
        return;
    }
}

/*function to transmit an array of data to device_address, starting from start_register_address*/
void DS3231_I2C_write_multi(DS3231_I2C *const me, uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
    uint8_t buf[1];
    buf[0] = start_register_address;
    me->i2cTransaction->slaveAddress = device_address;
    me->i2cTransaction->writeBuf = buf;
    me->i2cTransaction->writeCount = 1;
    me->i2cTransaction->readCount = 0;
    int8_t ret = 0;

    ret = I2C_transfer(*(me->i2cHandle), me->i2cTransaction);

    for (; data_length && ret; data_length--)
    {
        buf[0] = *data_array;
        //me->i2cTransaction->writeBuf = *data_array;
        ret = I2C_transfer(*(me->i2cHandle), me->i2cTransaction);
        ++data_array;
    }
    //I2C_close(*(me->i2cHandle));

}

/*function to read one byte of data from register_address on ds3231*/
void DS3231_I2C_read_single(DS3231_I2C *const me, uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
}

/*function to read an array of data from device_address*/
void DS3231_I2C_read_multi(DS3231_I2C *const me, uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
}

/*function to initialize I2C peripheral in 100khz or 400khz*/
void DS3231_I2C_init(DS3231_I2C *const me)
{
    me->i2cHandle = (I2C_Handle*) malloc(sizeof(I2C_Handle));
    me->i2cParams = (I2C_Params*) malloc(sizeof(I2C_Params));
    me->i2cTransaction = (I2C_Transaction*) malloc(sizeof(I2C_Transaction));

    I2C_Params_init(me->i2cParams);
    me->i2cParams->transferMode = I2C_MODE_BLOCKING;
    me->i2cParams->bitRate = I2C_100kHz;

    //Open I2C bus for usage
    *(me->i2cHandle) = I2C_open(1, me->i2cParams);
}

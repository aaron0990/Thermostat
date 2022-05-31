#include <LCDdisplayI2C.h>

DisplayI2C* DisplayI2C_create(void)
{
    DisplayI2C *displayI2C = (DisplayI2C*) malloc(sizeof(DisplayI2C));
    if (displayI2C != NULL)
    {
        DisplayI2C_initialize(displayI2C);
    }
    return displayI2C;
}

void DisplayI2C_initialize(DisplayI2C *const me)
{

    me->i2cHandle = (I2C_Handle*) malloc(sizeof(I2C_Handle));
    me->i2cParams = (I2C_Params*) malloc(sizeof(I2C_Params));
    me->i2cTransaction = (I2C_Transaction*) malloc(sizeof(I2C_Transaction));

    I2C_Params_init(me->i2cParams);
    me->i2cParams->transferMode = I2C_MODE_BLOCKING;
    me->i2cParams->bitRate = I2C_100kHz;

    //Initialize slave address of transaction
    me->i2cTransaction->slaveAddress = SLAVE_ADDR;
}

void DisplayI2C_send(DisplayI2C *const me, uint8_t data)
{
    //Open I2C bus for usage
    *(me->i2cHandle) = I2C_open(0, me->i2cParams);

    uint8_t buf[1];
    buf[0] = data;
    me->i2cTransaction->slaveAddress = 0x27;
    me->i2cTransaction->writeBuf = buf;
    me->i2cTransaction->writeCount = sizeof(buf);
    me->i2cTransaction->readCount = 0;
    int8_t ret = 0;
    do
    {
        ret = I2C_transfer(*(me->i2cHandle), me->i2cTransaction);
    }
    while (!ret);

    I2C_close(*(me->i2cHandle));

    if (ret == I2C_STATUS_TIMEOUT)
    {
        return;
    }
    else if (ret == I2C_STATUS_ERROR)
    {
        return;
    }

}

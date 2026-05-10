#include "mpu6050.h"
#include <stddef.h>

#define MPU6050_ADDR       0x68
#define MPU6050_ADDR_ALT   0x69
#define MPU6050_WHO_AM_I   0x68

#define REG_WHO_AM_I       0x75
#define REG_PWR_MGMT_1     0x6B
#define REG_SMPLRT_DIV     0x19
#define REG_CONFIG         0x1A
#define REG_GYRO_CONFIG    0x1B
#define REG_ACCEL_CONFIG   0x1C
#define REG_ACCEL_XOUT_H   0x3B
#define REG_TEMP_OUT_H     0x41
#define REG_GYRO_XOUT_H    0x43

static MPU6050_IO_t *pIO = NULL;
static uint8_t mpu_addr = 0;
static uint8_t mpu_ok = 0;

static int8_t read_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
    return pIO->read(mpu_addr, reg, data, len);
}

static int8_t write_byte(uint8_t reg, uint8_t val)
{
    return pIO->write(mpu_addr, reg, &val, 1);
}

static int8_t mpu_try_addr(uint8_t addr)
{
    uint8_t id;
    if (pIO->read(addr, REG_WHO_AM_I, &id, 1) != MPU6050_OK)
        return MPU6050_ERR_I2C;
    if (id != MPU6050_WHO_AM_I)
        return MPU6050_ERR_ID;

    mpu_addr = addr;

    write_byte(REG_PWR_MGMT_1, 0x00);
    pIO->delay_ms(10);

    write_byte(REG_SMPLRT_DIV, 0x07);
    write_byte(REG_CONFIG, 0x06);
    write_byte(REG_GYRO_CONFIG, 0x18);
    write_byte(REG_ACCEL_CONFIG, 0x10);

    {
        uint8_t check;
        if (pIO->read(mpu_addr, REG_PWR_MGMT_1, &check, 1) != MPU6050_OK)
            return MPU6050_ERR_INIT;
        if (check != 0x00)
            return MPU6050_ERR_INIT;
    }

    mpu_ok = 1;
    return MPU6050_OK;
}

int8_t MPU6050_Init(MPU6050_IO_t *io)
{
    pIO = io;
    if (!pIO)
        return MPU6050_ERR_NULL;

    mpu_ok = 0;

    if (mpu_try_addr(MPU6050_ADDR) != MPU6050_OK)
        mpu_try_addr(MPU6050_ADDR_ALT);

    return mpu_ok ? MPU6050_OK : MPU6050_ERR_I2C;
}

int8_t MPU6050_Read(MPU6050_Data_t *data)
{
    uint8_t buf[14];

    if (!pIO || !data)
        return MPU6050_ERR_NULL;

    if (!mpu_ok)
        return MPU6050_ERR_INIT;

    if (read_reg(REG_ACCEL_XOUT_H, buf, 14) != MPU6050_OK)
        return MPU6050_ERR_READ;

    data->ax = (int16_t)((uint16_t)buf[0] << 8) | buf[1];
    data->ay = (int16_t)((uint16_t)buf[2] << 8) | buf[3];
    data->az = (int16_t)((uint16_t)buf[4] << 8) | buf[5];
    data->temp = (int16_t)((uint16_t)buf[6] << 8) | buf[7];
    data->gx = (int16_t)((uint16_t)buf[8] << 8) | buf[9];
    data->gy = (int16_t)((uint16_t)buf[10] << 8) | buf[11];
    data->gz = (int16_t)((uint16_t)buf[12] << 8) | buf[13];

    return MPU6050_OK;
}

float MPU6050_GetAccelX(int16_t raw) { return (float)raw / 4096.0f; }
float MPU6050_GetAccelY(int16_t raw) { return (float)raw / 4096.0f; }
float MPU6050_GetAccelZ(int16_t raw) { return (float)raw / 4096.0f; }
float MPU6050_GetGyroX(int16_t raw)  { return (float)raw / 65.5f; }
float MPU6050_GetGyroY(int16_t raw)  { return (float)raw / 65.5f; }
float MPU6050_GetGyroZ(int16_t raw)  { return (float)raw / 65.5f; }
float MPU6050_GetTemp(int16_t raw)   { return (float)raw / 340.0f + 36.53f; }

#ifndef __MPU6050_H
#define __MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MPU6050_OK               0
#define MPU6050_ERR_NULL       (-1)
#define MPU6050_ERR_I2C        (-2)
#define MPU6050_ERR_ID         (-3)
#define MPU6050_ERR_INIT       (-4)
#define MPU6050_ERR_READ       (-5)

#define MPU6050_GYRO_FS_250   0
#define MPU6050_GYRO_FS_500   1
#define MPU6050_GYRO_FS_1000  2
#define MPU6050_GYRO_FS_2000  3

#define MPU6050_ACCEL_FS_2G   0
#define MPU6050_ACCEL_FS_4G   1
#define MPU6050_ACCEL_FS_8G   2
#define MPU6050_ACCEL_FS_16G  3

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t temp;
} MPU6050_Data_t;

typedef struct {
    int8_t (*read)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    int8_t (*write)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    void (*delay_ms)(uint32_t ms);
} MPU6050_IO_t;

void MPU6050_Init(MPU6050_IO_t *io);
int8_t MPU6050_Read(MPU6050_Data_t *data);
float  MPU6050_GetAccelX(int16_t raw);
float  MPU6050_GetAccelY(int16_t raw);
float  MPU6050_GetAccelZ(int16_t raw);
float  MPU6050_GetGyroX(int16_t raw);
float  MPU6050_GetGyroY(int16_t raw);
float  MPU6050_GetGyroZ(int16_t raw);
float  MPU6050_GetTemp(int16_t raw);

#ifdef __cplusplus
}
#endif

#endif

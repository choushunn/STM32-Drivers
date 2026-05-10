#ifndef __AHT20_BMP280_H
#define __AHT20_BMP280_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define AHT20_BMP280_OK                0
#define AHT20_BMP280_ERR_NULL        (-1)

#define AHT20_BMP280_ERR_AHT20_INIT   (-2)
#define AHT20_BMP280_ERR_AHT20_TRIG   (-3)
#define AHT20_BMP280_ERR_AHT20_READ   (-4)

#define AHT20_BMP280_ERR_BMP280_ADDR  (-5)
#define AHT20_BMP280_ERR_BMP280_ID    (-6)
#define AHT20_BMP280_ERR_BMP280_CALIB (-7)
#define AHT20_BMP280_ERR_BMP280_CFG   (-8)
#define AHT20_BMP280_ERR_BMP280_READ  (-9)

typedef struct {
    int8_t (*read)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    int8_t (*write)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    void (*delay_ms)(uint32_t ms);
} AHT20_BMP280_IO_t;

typedef struct {
    float temp;
    float humidity;
    float pressure;
    uint8_t aht20_ok;
    uint8_t bmp280_ok;
    int8_t  err_aht20;
    int8_t  err_bmp280;
} AHT20_BMP280_Data_t;

void AHT20_BMP280_Init(AHT20_BMP280_IO_t *io);
int8_t AHT20_BMP280_Read(AHT20_BMP280_Data_t *data);
const char *AHT20_BMP280_ErrStr(int8_t err);

#ifdef __cplusplus
}
#endif

#endif

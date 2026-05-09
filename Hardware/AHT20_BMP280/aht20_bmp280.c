#include "aht20_bmp280.h"
#include <stddef.h>

#define AHT20_ADDR      0x38
#define BMP280_ADDR     0x76
#define BMP280_ADDR_ALT 0x77

#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_TRIGGER   0xAC

#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_CALIB_T1     0x88
#define BMP280_REG_CHIP_ID      0xD0
#define BMP280_CHIP_ID_VAL      0x58

static AHT20_BMP280_IO_t *pIO = NULL;
static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t bmp280_ready = 0;
static uint8_t bmp280_addr = 0;

static int8_t read_reg(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return pIO->read(addr, reg, data, len);
}

static int8_t write_reg(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return pIO->write(addr, reg, data, len);
}

static int8_t write_cmd(uint8_t addr, uint8_t *data, uint16_t len)
{
    return pIO->write(addr, 0xFF, data, len);
}

const char* AHT20_BMP280_ErrStr(int8_t err)
{
    if (err == AHT20_BMP280_OK)                   return "OK";
    if (err == AHT20_BMP280_ERR_NULL)             return "NULL";
    if (err == AHT20_BMP280_ERR_AHT20_INIT)       return "AHT20_INIT";
    if (err == AHT20_BMP280_ERR_AHT20_TRIG)       return "AHT20_TRIG";
    if (err == AHT20_BMP280_ERR_AHT20_READ)       return "AHT20_READ";
    if (err == AHT20_BMP280_ERR_BMP280_ADDR)      return "BMP280_ADDR";
    if (err == AHT20_BMP280_ERR_BMP280_ID)        return "BMP280_ID";
    if (err == AHT20_BMP280_ERR_BMP280_CALIB)     return "BMP280_CALIB";
    if (err == AHT20_BMP280_ERR_BMP280_CFG)       return "BMP280_CFG";
    if (err == AHT20_BMP280_ERR_BMP280_READ)      return "BMP280_READ";
    return "UNKNOWN";
}

static int32_t bmp280_compensate_temp(int32_t adc_T, int32_t *t_fine)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t bmp280_compensate_press(int32_t adc_P, int32_t t_fine)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0)
        return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p >> 8);
}

static int8_t bmp280_read_calib(uint8_t addr)
{
    uint8_t buf[24];
    if (read_reg(addr, BMP280_REG_CALIB_T1, buf, 24) != AHT20_BMP280_OK)
        return AHT20_BMP280_ERR_BMP280_CALIB;

    dig_T1 = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    dig_T2 = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8));
    dig_T3 = (int16_t)((uint16_t)buf[4] | ((uint16_t)buf[5] << 8));
    dig_P1 = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
    dig_P2 = (int16_t)((uint16_t)buf[8] | ((uint16_t)buf[9] << 8));
    dig_P3 = (int16_t)((uint16_t)buf[10] | ((uint16_t)buf[11] << 8));
    dig_P4 = (int16_t)((uint16_t)buf[12] | ((uint16_t)buf[13] << 8));
    dig_P5 = (int16_t)((uint16_t)buf[14] | ((uint16_t)buf[15] << 8));
    dig_P6 = (int16_t)((uint16_t)buf[16] | ((uint16_t)buf[17] << 8));
    dig_P7 = (int16_t)((uint16_t)buf[18] | ((uint16_t)buf[19] << 8));
    dig_P8 = (int16_t)((uint16_t)buf[20] | ((uint16_t)buf[21] << 8));
    dig_P9 = (int16_t)((uint16_t)buf[22] | ((uint16_t)buf[23] << 8));
    return AHT20_BMP280_OK;
}

static int8_t bmp280_try_addr(uint8_t addr)
{
    uint8_t id;
    if (read_reg(addr, BMP280_REG_CHIP_ID, &id, 1) != AHT20_BMP280_OK)
        return AHT20_BMP280_ERR_BMP280_ADDR;
    if (id != BMP280_CHIP_ID_VAL)
        return AHT20_BMP280_ERR_BMP280_ID;

    bmp280_addr = addr;

    int8_t ret = bmp280_read_calib(addr);
    if (ret != AHT20_BMP280_OK)
        return ret;

    {
        uint8_t cfg;
        cfg = 0x27;
        if (write_reg(addr, BMP280_REG_CTRL_MEAS, &cfg, 1) != AHT20_BMP280_OK)
            return AHT20_BMP280_ERR_BMP280_CFG;
        cfg = 0xA0;
        if (write_reg(addr, BMP280_REG_CONFIG, &cfg, 1) != AHT20_BMP280_OK)
            return AHT20_BMP280_ERR_BMP280_CFG;
    }

    bmp280_ready = 1;
    return AHT20_BMP280_OK;
}

static int8_t aht20_init(void)
{
    uint8_t cmd[3];
    cmd[0] = AHT20_CMD_INIT;
    cmd[1] = 0x08;
    cmd[2] = 0x00;
    if (write_cmd(AHT20_ADDR, cmd, 3) != AHT20_BMP280_OK)
        return AHT20_BMP280_ERR_AHT20_INIT;
    pIO->delay_ms(10);
    return AHT20_BMP280_OK;
}

void AHT20_BMP280_Init(AHT20_BMP280_IO_t *io)
{
    pIO = io;
    if (!pIO)
        return;

    aht20_init();

    if (bmp280_try_addr(BMP280_ADDR) != AHT20_BMP280_OK)
        bmp280_try_addr(BMP280_ADDR_ALT);
}

int8_t AHT20_BMP280_Read(AHT20_BMP280_Data_t *data)
{
    uint8_t buf[6];
    uint32_t raw;
    int32_t adc_T, adc_P, t_fine;

    if (!pIO || !data)
        return AHT20_BMP280_ERR_NULL;

    data->aht20_ok = 0;
    data->bmp280_ok = 0;
    data->err_aht20 = AHT20_BMP280_OK;
    data->err_bmp280 = AHT20_BMP280_OK;

    {
        uint8_t cmd[3] = {AHT20_CMD_TRIGGER, 0x33, 0x00};
        if (write_cmd(AHT20_ADDR, cmd, 3) != AHT20_BMP280_OK)
        {
            data->err_aht20 = AHT20_BMP280_ERR_AHT20_TRIG;
        }
        else
        {
            pIO->delay_ms(80);

            if (read_reg(AHT20_ADDR, 0xFF, buf, 6) != AHT20_BMP280_OK)
            {
                data->err_aht20 = AHT20_BMP280_ERR_AHT20_READ;
            }
            else
            {
                raw = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | ((uint32_t)buf[3] >> 4);
                data->humidity = (float)raw * 100.0f / 1048576.0f;

                raw = (((uint32_t)(buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5]);
                data->temp = (float)raw * 200.0f / 1048576.0f - 50.0f;

                data->aht20_ok = 1;
            }
        }
    }

    if (bmp280_ready)
    {
        if (read_reg(bmp280_addr, BMP280_REG_PRESS_MSB, buf, 6) != AHT20_BMP280_OK)
        {
            data->err_bmp280 = AHT20_BMP280_ERR_BMP280_READ;
        }
        else
        {
            adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
            adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);

            data->temp = (float)bmp280_compensate_temp(adc_T, &t_fine) / 100.0f;
            data->pressure = (float)bmp280_compensate_press(adc_P, t_fine) / 25600.0f;

            data->bmp280_ok = 1;
        }
    }
    else
    {
        data->err_bmp280 = AHT20_BMP280_ERR_BMP280_ADDR;
    }

    return (data->aht20_ok || data->bmp280_ok) ? AHT20_BMP280_OK : AHT20_BMP280_ERR_NULL;
}

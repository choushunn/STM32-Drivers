#include "tof050f.h"
#include <stddef.h>

#define TOF050F_I2C_ADDR        0x52
#define VL6180X_ID              0xB4

#define REG_ID_MODEL_ID         0x000
#define REG_SYSTEM_INTERRUPT_CONFIG     0x014
#define REG_SYSTEM_INTERRUPT_CLEAR      0x015
#define REG_SYSRANGE_START              0x018
#define REG_RESULT_RANGE_STATUS         0x04D
#define REG_RESULT_RANGE_VAL            0x062

static TOF050F_IO_t *pIO = NULL;

static int8_t write_byte(uint16_t reg, uint8_t data)
{
    return pIO->write_reg(reg, &data, 1);
}

static int8_t read_byte(uint16_t reg, uint8_t *data)
{
    return pIO->read_reg(reg, data, 1);
}

int8_t TOF050F_Init(TOF050F_IO_t *io)
{
    uint8_t id;

    pIO = io;
    if (!pIO)
        return TOF050F_ERR_NULL;

    if (read_byte(REG_ID_MODEL_ID, &id) != TOF050F_OK)
        return TOF050F_ERR_I2C;

    if (id != VL6180X_ID)
        return TOF050F_ERR_I2C;

    write_byte(0x0207, 0x01);
    write_byte(0x0208, 0x01);
    write_byte(0x0096, 0x00);
    write_byte(0x0097, 0xFD);
    write_byte(0x00E3, 0x00);
    write_byte(0x00E4, 0x04);
    write_byte(0x00E5, 0x02);
    write_byte(0x00E6, 0x01);
    write_byte(0x00E7, 0x03);
    write_byte(0x00F5, 0x02);
    write_byte(0x00D9, 0x05);
    write_byte(0x00DB, 0xCE);
    write_byte(0x00DC, 0x07);
    write_byte(0x00DD, 0xF8);
    write_byte(0x009F, 0x00);
    write_byte(0x00A3, 0x3C);
    write_byte(0x00B7, 0x00);
    write_byte(0x00BB, 0x3C);
    write_byte(0x00B2, 0x09);
    write_byte(0x00CA, 0x00);
    write_byte(0x009C, 0x00);
    write_byte(0x00AA, 0x0E);
    write_byte(0x003E, 0x01);
    write_byte(0x001E, 0x31);
    write_byte(0x001B, 0x00);
    write_byte(REG_SYSTEM_INTERRUPT_CONFIG, 0x24);

    return TOF050F_OK;
}

int8_t TOF050F_ReadDistance(uint16_t *dist_mm)
{
    uint8_t status, range;

    if (!pIO || !dist_mm)
        return TOF050F_ERR_NULL;

    if (read_byte(REG_RESULT_RANGE_STATUS, &status) != TOF050F_OK)
        return TOF050F_ERR_I2C;
    if (!(status & 0x01))
        return TOF050F_ERR_I2C;

    if (write_byte(REG_SYSRANGE_START, 0x01) != TOF050F_OK)
        return TOF050F_ERR_I2C;

    pIO->delay_ms(30);

    if (write_byte(REG_SYSTEM_INTERRUPT_CLEAR, 0x07) != TOF050F_OK)
        return TOF050F_ERR_I2C;

    if (read_byte(REG_RESULT_RANGE_VAL, &range) != TOF050F_OK)
        return TOF050F_ERR_I2C;

    *dist_mm = range;
    return TOF050F_OK;
}

#include "dac1220.h"
#include <stddef.h>

#define CMD_WRITE(len, addr)  ((uint8_t)(((len) << 5) | (addr)))
#define CMD_READ(len, addr)   ((uint8_t)(0x80 | ((len) << 5) | (addr)))

#define MB_1BYTE  0
#define MB_2BYTE  1
#define MB_3BYTE  2

static DAC1220_IO_t *pIO = NULL;

static void xfer_frame(uint8_t cmd, const uint8_t *tx, uint8_t *rx, uint8_t len)
{
    pIO->cs(0);

    pIO->sdio_dir(1);
    pIO->send_byte(cmd);

    if (tx)
    {
        pIO->sdio_dir(1);
        for (uint8_t i = 0; i < len; i++)
            pIO->send_byte(tx[i]);
    }
    else if (rx)
    {
        pIO->sdio_dir(0);
        for (uint8_t i = 0; i < len; i++)
            rx[i] = pIO->recv_byte();
    }

    pIO->sdio_dir(1);
    pIO->cs(1);
}

void DAC1220_Init(DAC1220_IO_t *io)
{
    pIO = io;
    if (!pIO) return;

    pIO->cs(1);
    pIO->sdio_dir(1);

    pIO->delay_us(10);

    uint8_t cmr = DAC1220_CMD_RST;
    DAC1220_WriteReg(DAC1220_REG_CMR, &cmr, 1);
    pIO->delay_us(10);

    cmr = DAC1220_CMD_NOP;
    DAC1220_WriteReg(DAC1220_REG_CMR, &cmr, 1);
    pIO->delay_us(10);
}

int8_t DAC1220_WriteReg(uint8_t addr, const uint8_t *data, uint8_t len)
{
    if (!pIO) return DAC1220_ERR_NULL;
    if (!data || len == 0 || len > 3) return DAC1220_ERR_PARAM;

    uint8_t mb = (len == 3) ? MB_3BYTE : (len == 2) ? MB_2BYTE : MB_1BYTE;
    uint8_t cmd = CMD_WRITE(mb, addr);

    xfer_frame(cmd, data, NULL, len);

    return DAC1220_OK;
}

int8_t DAC1220_ReadReg(uint8_t addr, uint8_t *data, uint8_t len)
{
    if (!pIO) return DAC1220_ERR_NULL;
    if (!data || len == 0 || len > 3) return DAC1220_ERR_PARAM;

    uint8_t mb = (len == 3) ? MB_3BYTE : (len == 2) ? MB_2BYTE : MB_1BYTE;
    uint8_t cmd = CMD_READ(mb, addr);

    xfer_frame(cmd, NULL, data, len);

    return DAC1220_OK;
}

int8_t DAC1220_SetValue(uint32_t value)
{
    if (!pIO) return DAC1220_ERR_NULL;
    if (value > DAC1220_VALUE_MAX) return DAC1220_ERR_PARAM;

    uint8_t data[3];
    data[0] = (uint8_t)((value >> 12) & 0xFF);
    data[1] = (uint8_t)((value >> 4) & 0xFF);
    data[2] = (uint8_t)((value & 0x0F) << 4);

    DAC1220_WriteReg(DAC1220_REG_DIR, data, 3);

    return DAC1220_OK;
}

int8_t DAC1220_Calibrate(void)
{
    if (!pIO) return DAC1220_ERR_NULL;

    uint8_t cmr = DAC1220_CMD_CAL;
    DAC1220_WriteReg(DAC1220_REG_CMR, &cmr, 1);

    pIO->delay_us(20000);

    cmr = DAC1220_CMD_NOP;
    DAC1220_WriteReg(DAC1220_REG_CMR, &cmr, 1);

    return DAC1220_OK;
}

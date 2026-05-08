#ifndef __DAC1220_H
#define __DAC1220_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define DAC1220_OK            0
#define DAC1220_ERR_NULL    (-1)
#define DAC1220_ERR_PARAM   (-2)

/* 寄存器地址 */
#define DAC1220_REG_CMR    0x00
#define DAC1220_REG_DIR    0x01
#define DAC1220_REG_OCR    0x02
#define DAC1220_REG_FCR    0x03

/* CMR 命令位 */
#define DAC1220_CMD_NOP     0x00
#define DAC1220_CMD_CAL     0x02
#define DAC1220_CMD_RST     0x04

/* DAC 分辨率 */
#define DAC1220_RES_16BIT    0
#define DAC1220_RES_20BIT    1

/* 20 位 DAC 范围 */
#define DAC1220_VALUE_MIN     0
#define DAC1220_VALUE_MAX    0xFFFFF

typedef struct {
    void (*cs)(uint8_t level);
    void (*sdio_dir)(uint8_t output);
    void (*send_byte)(uint8_t data);
    uint8_t (*recv_byte)(void);
    void (*delay_us)(uint32_t us);
} DAC1220_IO_t;

void DAC1220_Init(DAC1220_IO_t *io);
int8_t DAC1220_WriteReg(uint8_t addr, const uint8_t *data, uint8_t len);
int8_t DAC1220_ReadReg(uint8_t addr, uint8_t *data, uint8_t len);
int8_t DAC1220_SetValue(uint32_t value);
int8_t DAC1220_Calibrate(void);

#ifdef __cplusplus
}
#endif

#endif

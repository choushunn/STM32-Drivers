#ifndef __TOF050F_H
#define __TOF050F_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TOF050F_OK            0
#define TOF050F_ERR_NULL    (-1)
#define TOF050F_ERR_TIMEOUT (-2)
#define TOF050F_ERR_I2C     (-3)

typedef struct {
    int8_t (*read_reg)(uint16_t reg, uint8_t *data, uint16_t len);
    int8_t (*write_reg)(uint16_t reg, uint8_t *data, uint16_t len);
    void (*delay_ms)(uint32_t ms);
} TOF050F_IO_t;

void TOF050F_Init(TOF050F_IO_t *io);
int8_t TOF050F_ReadDistance(uint16_t *dist_mm);

#ifdef __cplusplus
}
#endif

#endif

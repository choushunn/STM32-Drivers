#ifndef __TEMT6000_H
#define __TEMT6000_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TEMT6000_OK            0
#define TEMT6000_ERR_NULL    (-1)

typedef struct {
    uint16_t (*read_adc)(void);
} TEMT6000_IO_t;

typedef struct {
    uint16_t raw;
    float    lux;
} TEMT6000_Data_t;

void TEMT6000_Init(TEMT6000_IO_t *io);
uint16_t TEMT6000_ReadRaw(void);
float    TEMT6000_CalcLux(uint16_t raw);

#ifdef __cplusplus
}
#endif

#endif

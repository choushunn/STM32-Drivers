#ifndef __TURBIDITY_H
#define __TURBIDITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TURBIDITY_OK            0
#define TURBIDITY_ERR_NULL    (-1)

typedef struct {
    uint16_t (*read_adc)(void);
} TURBIDITY_IO_t;

typedef struct {
    uint16_t raw;
    float    turbidity;
} TURBIDITY_Data_t;

void TURBIDITY_Init(TURBIDITY_IO_t *io);
uint16_t TURBIDITY_ReadRaw(void);
float    TURBIDITY_CalcPercent(uint16_t raw);

#ifdef __cplusplus
}
#endif

#endif

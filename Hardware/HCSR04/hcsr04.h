#ifndef __HCSR04_H
#define __HCSR04_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define HCSR04_OK            0
#define HCSR04_ERR_NULL    (-1)
#define HCSR04_ERR_TIMEOUT (-2)
#define HCSR04_ERR_RANGE   (-3)

typedef struct {
    void (*trig)(void);
    uint8_t (*read_echo)(void);
    uint32_t (*get_us)(void);
} HCSR04_IO_t;

void HCSR04_Init(HCSR04_IO_t *io);
int8_t HCSR04_Read(uint32_t *distance_mm);
uint32_t HCSR04_GetPulseUs(void);

#ifdef __cplusplus
}
#endif

#endif

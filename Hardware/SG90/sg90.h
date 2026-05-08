#ifndef __SG90_H
#define __SG90_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define SG90_OK            0
#define SG90_ERR_NULL    (-1)
#define SG90_ERR_RANGE   (-2)

/* 角度范围 */
#define SG90_ANGLE_MIN      0
#define SG90_ANGLE_MAX    180

/* 脉宽范围 (μs) */
#define SG90_PULSE_MIN    500
#define SG90_PULSE_MID   1500
#define SG90_PULSE_MAX   2500

typedef struct {
    void (*set_pulse)(uint16_t us);
    uint32_t (*get_ms)(void);
} SG90_IO_t;

void SG90_Init(SG90_IO_t *io);
int8_t SG90_SetAngle(uint16_t angle);
int8_t SG90_SetAngleSmooth(uint16_t target_angle, uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif

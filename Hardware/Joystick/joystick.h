#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define JOYSTICK_OK            0
#define JOYSTICK_ERR_NULL    (-1)

/* 方向 */
typedef enum {
    JOYSTICK_DIR_CENTER  = 0,
    JOYSTICK_DIR_UP      = 1,
    JOYSTICK_DIR_DOWN    = 2,
    JOYSTICK_DIR_LEFT    = 3,
    JOYSTICK_DIR_RIGHT   = 4,
} JOYSTICK_Dir_t;

/* 数据 */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  btn;
} JOYSTICK_Data_t;

typedef struct {
    uint16_t (*read_x)(void);
    uint16_t (*read_y)(void);
    uint8_t  (*read_btn)(void);
} JOYSTICK_IO_t;

void JOYSTICK_Init(JOYSTICK_IO_t *io);
int8_t JOYSTICK_Read(JOYSTICK_Data_t *data);
JOYSTICK_Dir_t JOYSTICK_GetDir(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif

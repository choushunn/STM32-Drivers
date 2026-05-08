#include "joystick.h"
#include <stddef.h>

/* 中位死区阈值 (ADC 12位, 中心约 2048) */
#define JOYSTICK_DEAD_ZONE     300
#define JOYSTICK_CENTER        2048
#define JOYSTICK_RANGE_LOW     (JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE)
#define JOYSTICK_RANGE_HIGH    (JOYSTICK_CENTER + JOYSTICK_DEAD_ZONE)

static JOYSTICK_IO_t *pIO = NULL;

void JOYSTICK_Init(JOYSTICK_IO_t *io)
{
    pIO = io;
}

int8_t JOYSTICK_Read(JOYSTICK_Data_t *data)
{
    if (!pIO) return JOYSTICK_ERR_NULL;

    data->x = pIO->read_x();
    data->y = pIO->read_y();
    data->btn = pIO->read_btn();

    return JOYSTICK_OK;
}

JOYSTICK_Dir_t JOYSTICK_GetDir(uint16_t x, uint16_t y)
{
    uint8_t x_dir = 0;
    uint8_t y_dir = 0;

    if (x < JOYSTICK_RANGE_LOW)
        x_dir = 1;
    else if (x > JOYSTICK_RANGE_HIGH)
        x_dir = 2;

    if (y < JOYSTICK_RANGE_LOW)
        y_dir = 1;
    else if (y > JOYSTICK_RANGE_HIGH)
        y_dir = 2;

    if (x_dir == 0 && y_dir == 0)
        return JOYSTICK_DIR_CENTER;

    if (x_dir && y_dir)
    {
        uint16_t dx = (x > JOYSTICK_CENTER) ? (x - JOYSTICK_CENTER) : (JOYSTICK_CENTER - x);
        uint16_t dy = (y > JOYSTICK_CENTER) ? (y - JOYSTICK_CENTER) : (JOYSTICK_CENTER - y);

        if (dx >= dy)
            return (x_dir == 1) ? JOYSTICK_DIR_LEFT : JOYSTICK_DIR_RIGHT;
        else
            return (y_dir == 1) ? JOYSTICK_DIR_DOWN : JOYSTICK_DIR_UP;
    }

    if (x_dir)
        return (x_dir == 1) ? JOYSTICK_DIR_LEFT : JOYSTICK_DIR_RIGHT;

    return (y_dir == 1) ? JOYSTICK_DIR_DOWN : JOYSTICK_DIR_UP;
}

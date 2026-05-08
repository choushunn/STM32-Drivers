#include "sg90.h"
#include <stddef.h>

static SG90_IO_t *pIO = NULL;
static uint16_t current_angle = 90;

void SG90_Init(SG90_IO_t *io)
{
    pIO = io;
    current_angle = 90;
    if (pIO)
        pIO->set_pulse(SG90_PULSE_MID);
}

static uint16_t angle_to_pulse(uint16_t angle)
{
    if (angle > SG90_ANGLE_MAX)
        angle = SG90_ANGLE_MAX;
    return SG90_PULSE_MIN + (uint32_t)(angle) * (SG90_PULSE_MAX - SG90_PULSE_MIN) / SG90_ANGLE_MAX;
}

int8_t SG90_SetAngle(uint16_t angle)
{
    if (!pIO) return SG90_ERR_NULL;
    if (angle > SG90_ANGLE_MAX)
        return SG90_ERR_RANGE;

    current_angle = angle;
    pIO->set_pulse(angle_to_pulse(angle));
    return SG90_OK;
}

int8_t SG90_SetAngleSmooth(uint16_t target_angle, uint16_t speed)
{
    if (!pIO) return SG90_ERR_NULL;
    if (target_angle > SG90_ANGLE_MAX)
        return SG90_ERR_RANGE;

    if (!pIO->get_ms)
        return SG90_SetAngle(target_angle);

    if (speed < 1) speed = 1;

    while (current_angle != target_angle)
    {
        uint32_t tick = pIO->get_ms();

        if (current_angle < target_angle)
        {
            if (target_angle - current_angle <= speed)
                current_angle = target_angle;
            else
                current_angle += speed;
        }
        else
        {
            if (current_angle - target_angle <= speed)
                current_angle = target_angle;
            else
                current_angle -= speed;
        }

        pIO->set_pulse(angle_to_pulse(current_angle));

        while (pIO->get_ms() - tick < 20);
    }

    return SG90_OK;
}

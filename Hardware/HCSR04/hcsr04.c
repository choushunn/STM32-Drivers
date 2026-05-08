#include "hcsr04.h"
#include <stddef.h>

#define TIMEOUT_ECHO_HIGH_US  5000
#define TIMEOUT_ECHO_LOW_US  30000
#define TRIG_SETTLE_US       500

static HCSR04_IO_t *pIO = NULL;
static uint32_t hcsr04_debug_pulse_us = 0;
#if HCSR04_FILTER_STRENGTH > 0
static uint32_t ema_pulse;
static uint8_t ema_first = 1;
#endif

void HCSR04_Init(HCSR04_IO_t *io)
{
    pIO = io;
}

int8_t HCSR04_Read(uint32_t *distance_mm)
{
    uint32_t start;
    uint32_t pulse;

    if (!pIO) return HCSR04_ERR_NULL;

    pIO->trig();

    start = pIO->get_us();
    while (pIO->read_echo())
    {
        if (pIO->get_us() - start > TRIG_SETTLE_US)
            return HCSR04_ERR_TIMEOUT;
    }

    start = pIO->get_us();
    while (!pIO->read_echo())
    {
        if (pIO->get_us() - start > TIMEOUT_ECHO_HIGH_US)
            return HCSR04_ERR_TIMEOUT;
    }

    start = pIO->get_us();
    while (pIO->read_echo())
    {
        if (pIO->get_us() - start > TIMEOUT_ECHO_LOW_US)
            return HCSR04_ERR_TIMEOUT;
    }
    pulse = pIO->get_us() - start;
    hcsr04_debug_pulse_us = pulse;

    if (pulse < 100)
        return HCSR04_ERR_RANGE;

#if HCSR04_FILTER_STRENGTH > 0
    if (ema_first)
    {
        ema_pulse = pulse;
        ema_first = 0;
    }
    else
    {
        ema_pulse = (ema_pulse * HCSR04_FILTER_STRENGTH + pulse) / (HCSR04_FILTER_STRENGTH + 1);
    }
    pulse = ema_pulse;
#endif

    *distance_mm = pulse * 10 / 58;

    return HCSR04_OK;
}

uint32_t HCSR04_GetPulseUs(void)
{
    return hcsr04_debug_pulse_us;
}

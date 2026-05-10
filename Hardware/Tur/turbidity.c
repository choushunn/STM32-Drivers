#include "turbidity.h"
#include <stddef.h>

static TURBIDITY_IO_t *pIO = NULL;

void TURBIDITY_Init(TURBIDITY_IO_t *io)
{
    pIO = io;
}

uint16_t TURBIDITY_ReadRaw(void)
{
    if (!pIO)
        return 0;
    return pIO->read_adc();
}

float TURBIDITY_CalcPercent(uint16_t raw)
{
    float turb = 100.0f - (float)raw * 100.0f / 4096.0f;
    if (turb < 0.0f)
        turb = 0.0f;
    if (turb > 100.0f)
        turb = 100.0f;
    return turb;
}

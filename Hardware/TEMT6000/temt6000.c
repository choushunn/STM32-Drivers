#include "temt6000.h"
#include <stddef.h>

#define TEMT6000_LUX_SCALE    1.24f

static TEMT6000_IO_t *pIO = NULL;

void TEMT6000_Init(TEMT6000_IO_t *io)
{
    pIO = io;
}

uint16_t TEMT6000_ReadRaw(void)
{
    if (!pIO)
        return 0;
    return pIO->read_adc();
}

float TEMT6000_CalcLux(uint16_t raw)
{
    return (float)raw * TEMT6000_LUX_SCALE;
}

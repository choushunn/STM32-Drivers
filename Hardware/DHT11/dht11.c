#include "dht11.h"
#include <stddef.h>

static DHT11_IO_t *pIO = NULL;

void DHT11_Init(DHT11_IO_t *io)
{
    pIO = io;
    pIO->set_output();
    pIO->write_high();
}

static int8_t DHT11_ReadBit(void)
{
    uint32_t timeout = 0;

    while (!pIO->read_pin() && timeout < 150)
    {
        timeout++;
        pIO->delay_us(1);
    }
    if (timeout >= 150) return -1;

    pIO->delay_us(30);

    uint8_t val = pIO->read_pin();

    timeout = 0;
    while (val && pIO->read_pin() && timeout < 150)
    {
        timeout++;
        pIO->delay_us(1);
    }

    return (int8_t)val;
}

static int8_t DHT11_ReadByte(uint8_t *byte)
{
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        int8_t bit = DHT11_ReadBit();
        if (bit < 0) return -1;
        data = (data << 1) | (uint8_t)bit;
    }
    *byte = data;
    return 0;
}

int8_t DHT11_ReadData(DHT11_Data_t *data)
{
    uint8_t buf[5];
    uint32_t timeout;

    if (!pIO) return DHT11_ERR_NULL;

    pIO->set_output();
    pIO->write_low();
    pIO->delay_ms(20);
    pIO->write_high();
    pIO->delay_us(30);

    pIO->set_input();

    timeout = 0;
    while (pIO->read_pin() && timeout < 200)
    {
        timeout++;
        pIO->delay_us(1);
    }
    if (timeout >= 200) return DHT11_ERR_START_LOW;

    timeout = 0;
    while (!pIO->read_pin() && timeout < 200)
    {
        timeout++;
        pIO->delay_us(1);
    }
    if (timeout >= 200) return DHT11_ERR_RESP_LOW;

    timeout = 0;
    while (pIO->read_pin() && timeout < 200)
    {
        timeout++;
        pIO->delay_us(1);
    }
    if (timeout >= 200) return DHT11_ERR_RESP_HIGH;

    for (uint8_t i = 0; i < 5; i++)
    {
        if (DHT11_ReadByte(&buf[i]) < 0)
            return DHT11_ERR_READ_BIT;
    }

    for (uint8_t i = 0; i < 5; i++)
        data->raw[i] = buf[i];

    if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) != buf[4])
        return DHT11_ERR_CHECKSUM;

    data->humidity = buf[0] * 10 + buf[1];

    uint8_t temp_int = buf[2] & 0x7F;
    data->temperature = (int16_t)(temp_int * 10 + buf[3]);

    if (buf[2] & 0x80)
        data->temperature = -data->temperature;

    return DHT11_OK;
}

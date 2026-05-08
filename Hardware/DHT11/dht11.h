#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define DHT11_OK               0
#define DHT11_ERR_NULL       (-1)
#define DHT11_ERR_START_LOW  (-2)
#define DHT11_ERR_RESP_LOW   (-3)
#define DHT11_ERR_RESP_HIGH  (-4)
#define DHT11_ERR_READ_BIT   (-5)
#define DHT11_ERR_CHECKSUM   (-6)

typedef struct {
    int16_t temperature;
    uint16_t humidity;
    uint8_t raw[5];
} DHT11_Data_t;

typedef struct {
    void (*set_output)(void);
    void (*set_input)(void);
    void (*write_low)(void);
    void (*write_high)(void);
    uint8_t (*read_pin)(void);
    void (*delay_us)(uint32_t us);
    void (*delay_ms)(uint32_t ms);
} DHT11_IO_t;

void DHT11_Init(DHT11_IO_t *io);
int8_t DHT11_ReadData(DHT11_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif

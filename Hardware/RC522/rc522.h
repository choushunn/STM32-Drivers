#ifndef __RC522_H
#define __RC522_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RC522_OK             0
#define RC522_ERR_NULL     (-1)
#define RC522_ERR_VERSION  (-2)
#define RC522_ERR_TIMEOUT  (-3)
#define RC522_ERR_CRC      (-4)
#define RC522_ERR_COLLISION (-5)
#define RC522_ERR_BCC      (-6)
#define RC522_ERR_TRANS    (-7)
#define RC522_ERR_AUTH     (-8)

#define MAXRLEN 18

typedef struct {
    void (*cs_low)(void);
    void (*cs_high)(void);
    void (*rst_low)(void);
    void (*rst_high)(void);
    void (*spi_transfer)(uint8_t *tx, uint8_t *rx, uint16_t len);
    void (*delay_ms)(uint32_t ms);
} RC522_IO_t;

typedef struct {
    uint8_t uid[4];
    uint8_t sak;
    uint8_t card_type[2];
} RC522_CardInfo_t;

#define PCD_IDLE            0x00
#define PCD_AUTHENT         0x0E
#define PCD_RECEIVE         0x08
#define PCD_TRANSMIT        0x04
#define PCD_TRANSCEIVE      0x0C
#define PCD_RESETPHASE      0x0F
#define PCD_CALCCRC         0x03

#define PICC_REQIDL         0x26
#define PICC_REQALL         0x52
#define PICC_ANTICOLL1      0x93
#define PICC_ANTICOLL2      0x95
#define PICC_ANTICOLL3      0x97
#define PICC_AUTHENT1A      0x60
#define PICC_AUTHENT1B      0x61
#define PICC_READ           0x30
#define PICC_WRITE          0xA0
#define PICC_DECREMENT      0xC0
#define PICC_INCREMENT      0xC1
#define PICC_RESTORE        0xC2
#define PICC_TRANSFER       0xB0
#define PICC_HALT           0x50

#define REG_COMMAND         0x01
#define REG_COMIEN          0x02
#define REG_DIVIEN          0x03
#define REG_COMIRQ          0x04
#define REG_DIVIRQ          0x05
#define REG_ERROR           0x06
#define REG_STATUS1         0x07
#define REG_STATUS2         0x08
#define REG_FIFODATA        0x09
#define REG_FIFOLEVEL       0x0A
#define REG_WATERLEVEL      0x0B
#define REG_CONTROL         0x0C
#define REG_BITFRAMING      0x0D
#define REG_COLL            0x0E

#define REG_MODE            0x11
#define REG_TXMODE          0x12
#define REG_RXMODE          0x13
#define REG_TXCONTROL       0x14
#define REG_TXAUTO          0x15
#define REG_TXSEL           0x16
#define REG_RXSEL           0x17
#define REG_RXTHRESHOLD     0x18
#define REG_DEMOD           0x19
#define REG_MIFARE          0x1C
#define REG_SERIALSPEED     0x1F

#define REG_CRCRESULTM      0x21
#define REG_CRCRESULTL      0x22
#define REG_MODWIDTH         0x24
#define REG_RFCONFIG        0x26
#define REG_GSN             0x27
#define REG_CWGCSP          0x28
#define REG_MODGSP          0x29
#define REG_TMODE           0x2A
#define REG_TPRESCALER      0x2B
#define REG_TRELOADH        0x2C
#define REG_TRELOADL        0x2D
#define REG_TCOUNTERVALH    0x2E
#define REG_TCOUNTERVALL    0x2F

#define REG_VERSION         0x37

void RC522_Init(RC522_IO_t *io);
int8_t RC522_Reset(void);
int8_t RC522_CheckCard(uint8_t *card_type);
int8_t RC522_Anticoll(uint8_t *uid);
int8_t RC522_SelectTag(uint8_t *snr, uint8_t *sak);
int8_t RC522_AuthState(uint8_t auth_mode, uint8_t addr, uint8_t *key, uint8_t *snr);
int8_t RC522_ReadBlock(uint8_t addr, uint8_t *data);
int8_t RC522_WriteBlock(uint8_t addr, uint8_t *data);
int8_t RC522_Halt(void);
void RC522_AntennaOn(void);
void RC522_AntennaOff(void);
uint8_t RC522_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif

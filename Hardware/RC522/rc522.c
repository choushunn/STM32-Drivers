#include "rc522.h"
#include <stddef.h>

static RC522_IO_t *pIO = NULL;

static void WriteRawRC(uint8_t addr, uint8_t val)
{
    uint8_t tx[2] = {(addr << 1) & 0x7E, val};
    uint8_t rx[2];

    pIO->cs_low();
    pIO->spi_transfer(tx, rx, 2);
    pIO->cs_high();
}

static uint8_t ReadRawRC(uint8_t addr)
{
    uint8_t tx[2] = {((addr << 1) & 0x7E) | 0x80, 0x00};
    uint8_t rx[2];

    pIO->cs_low();
    pIO->spi_transfer(tx, rx, 2);
    pIO->cs_high();

    return rx[1];
}

static void SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask);
}

static void ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);
}

static void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
    uint8_t i, n;

    ClearBitMask(REG_DIVIRQ, 0x04);
    WriteRawRC(REG_COMMAND, PCD_IDLE);
    SetBitMask(REG_FIFOLEVEL, 0x80);

    for (i = 0; i < len; i++)
    {
        WriteRawRC(REG_FIFODATA, *(pIndata + i));
    }

    WriteRawRC(REG_COMMAND, PCD_CALCCRC);

    i = 0xFF;
    do
    {
        n = ReadRawRC(REG_DIVIRQ);
        i--;
    } while ((i != 0) && !(n & 0x04));

    pOutData[0] = ReadRawRC(REG_CRCRESULTL);
    pOutData[1] = ReadRawRC(REG_CRCRESULTM);
}

static int8_t PcdComMF522(uint8_t cmd,
                           uint8_t *pIn,
                           uint8_t inLen,
                           uint8_t *pOut,
                           uint16_t *pOutBits)
{
    int8_t status = RC522_ERR_TRANS;
    uint8_t irqEn = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    switch (cmd)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(REG_COMIEN, irqEn | 0x80);
    ClearBitMask(REG_COMIRQ, 0x80);
    WriteRawRC(REG_COMMAND, PCD_IDLE);
    SetBitMask(REG_FIFOLEVEL, 0x80);

    for (i = 0; i < inLen; i++)
    {
        WriteRawRC(REG_FIFODATA, pIn[i]);
    }

    WriteRawRC(REG_COMMAND, cmd);

    if (cmd == PCD_TRANSCEIVE)
    {
        SetBitMask(REG_BITFRAMING, 0x80);
    }

    i = 600;
    do
    {
        n = ReadRawRC(REG_COMIRQ);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));

    ClearBitMask(REG_BITFRAMING, 0x80);

    if (i != 0)
    {
        if (!(ReadRawRC(REG_ERROR) & 0x1B))
        {
            status = RC522_OK;

            if (n & irqEn & 0x01)
            {
                status = RC522_ERR_TIMEOUT;
            }

            if (cmd == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(REG_FIFOLEVEL);
                lastBits = ReadRawRC(REG_CONTROL) & 0x07;

                if (lastBits)
                {
                    *pOutBits = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutBits = n * 8;
                }

                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }

                for (i = 0; i < n; i++)
                {
                    pOut[i] = ReadRawRC(REG_FIFODATA);
                }
            }
        }
        else
        {
            status = RC522_ERR_TRANS;
        }
    }
    else
    {
        status = RC522_ERR_TIMEOUT;
    }

    SetBitMask(REG_CONTROL, 0x80);
    WriteRawRC(REG_COMMAND, PCD_IDLE);

    return status;
}

void RC522_Init(RC522_IO_t *io)
{
    pIO = io;
}

int8_t RC522_Reset(void)
{
    if (!pIO) return RC522_ERR_NULL;

    pIO->rst_high();
    pIO->delay_ms(1);
    pIO->rst_low();
    pIO->delay_ms(1);
    pIO->rst_high();
    pIO->delay_ms(1);

    WriteRawRC(REG_COMMAND, PCD_RESETPHASE);
    pIO->delay_ms(1);

    uint8_t ver = ReadRawRC(REG_VERSION);
    if (ver == 0x00 || ver == 0xFF)
    {
        return RC522_ERR_VERSION;
    }

    WriteRawRC(REG_MODE, 0x3D);
    WriteRawRC(REG_TRELOADL, 30);
    WriteRawRC(REG_TRELOADH, 0);
    WriteRawRC(REG_TMODE, 0x8D);
    WriteRawRC(REG_TPRESCALER, 0x3E);
    WriteRawRC(REG_TXAUTO, 0x40);
    WriteRawRC(REG_RFCONFIG, 0x68);
    WriteRawRC(REG_TXSEL, 0x10);
    WriteRawRC(REG_RXSEL, 0x84);
    WriteRawRC(REG_BITFRAMING, 0x00);
    WriteRawRC(REG_COLL, 0x80);

    RC522_AntennaOn();

    return RC522_OK;
}

uint8_t RC522_GetVersion(void)
{
    return ReadRawRC(REG_VERSION);
}

void RC522_AntennaOn(void)
{
    uint8_t i = ReadRawRC(REG_TXCONTROL);
    if (!(i & 0x03))
    {
        SetBitMask(REG_TXCONTROL, 0x03);
    }
}

void RC522_AntennaOff(void)
{
    ClearBitMask(REG_TXCONTROL, 0x03);
}

int8_t RC522_CheckCard(uint8_t *card_type)
{
    int8_t status;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    ClearBitMask(REG_STATUS2, 0x08);
    WriteRawRC(REG_BITFRAMING, 0x07);
    SetBitMask(REG_TXCONTROL, 0x03);

    buf[0] = PICC_REQALL;

    status = PcdComMF522(PCD_TRANSCEIVE, buf, 1, buf, &unLen);

    if ((status == RC522_OK) && (unLen == 0x10))
    {
        if (card_type != NULL)
        {
            card_type[0] = buf[0];
            card_type[1] = buf[1];
        }
    }
    else
    {
        status = RC522_ERR_TRANS;
    }

    return status;
}

int8_t RC522_Anticoll(uint8_t *uid)
{
    int8_t status;
    uint8_t i;
    uint8_t snr_check = 0;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    ClearBitMask(REG_STATUS2, 0x08);
    WriteRawRC(REG_BITFRAMING, 0x00);
    ClearBitMask(REG_COLL, 0x80);

    buf[0] = PICC_ANTICOLL1;
    buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, buf, 2, buf, &unLen);

    if (status == RC522_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(uid + i) = buf[i];
            snr_check ^= buf[i];
        }

        if (snr_check != buf[i])
        {
            status = RC522_ERR_BCC;
        }
    }

    SetBitMask(REG_COLL, 0x80);
    return status;
}

int8_t RC522_SelectTag(uint8_t *snr, uint8_t *sak)
{
    int8_t status;
    uint8_t i;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    buf[0] = PICC_ANTICOLL1;
    buf[1] = 0x70;
    buf[6] = 0;

    for (i = 0; i < 4; i++)
    {
        buf[i + 2] = *(snr + i);
        buf[6] ^= *(snr + i);
    }

    CalulateCRC(buf, 7, &buf[7]);

    ClearBitMask(REG_STATUS2, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, buf, 9, buf, &unLen);

    if ((status == RC522_OK) && (unLen == 0x18))
    {
        if (sak != NULL)
        {
            *sak = buf[0];
        }
    }
    else
    {
        status = RC522_ERR_TRANS;
    }

    return status;
}

int8_t RC522_AuthState(uint8_t auth_mode, uint8_t addr, uint8_t *key, uint8_t *snr)
{
    int8_t status;
    uint8_t i;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    buf[0] = auth_mode;
    buf[1] = addr;

    for (i = 0; i < 6; i++)
    {
        buf[i + 2] = *(key + i);
    }
    for (i = 0; i < 6; i++)
    {
        buf[i + 8] = *(snr + i);
    }

    status = PcdComMF522(PCD_AUTHENT, buf, 12, buf, &unLen);

    if ((status != RC522_OK) || (!(ReadRawRC(REG_STATUS2) & 0x08)))
    {
        status = RC522_ERR_AUTH;
    }

    return status;
}

int8_t RC522_ReadBlock(uint8_t addr, uint8_t *data)
{
    int8_t status;
    uint8_t i;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    buf[0] = PICC_READ;
    buf[1] = addr;
    CalulateCRC(buf, 2, &buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, buf, 4, buf, &unLen);

    if ((status == RC522_OK) && (unLen == 0x90))
    {
        for (i = 0; i < 16; i++)
        {
            *(data + i) = buf[i];
        }
    }
    else
    {
        status = RC522_ERR_TRANS;
    }

    return status;
}

int8_t RC522_WriteBlock(uint8_t addr, uint8_t *data)
{
    int8_t status;
    uint8_t i;
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    buf[0] = PICC_WRITE;
    buf[1] = addr;
    CalulateCRC(buf, 2, &buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, buf, 4, buf, &unLen);

    if ((status != RC522_OK) || (unLen != 4) || ((buf[0] & 0x0F) != 0x0A))
    {
        status = RC522_ERR_TRANS;
    }

    if (status == RC522_OK)
    {
        for (i = 0; i < 16; i++)
        {
            buf[i] = *(data + i);
        }
        CalulateCRC(buf, 16, &buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, buf, 18, buf, &unLen);

        if ((status != RC522_OK) || (unLen != 4) || ((buf[0] & 0x0F) != 0x0A))
        {
            status = RC522_ERR_TRANS;
        }
    }

    return status;
}

int8_t RC522_Halt(void)
{
    uint16_t unLen;
    uint8_t buf[MAXRLEN];

    if (!pIO) return RC522_ERR_NULL;

    buf[0] = PICC_HALT;
    buf[1] = 0;

    CalulateCRC(buf, 2, &buf[2]);

    PcdComMF522(PCD_TRANSCEIVE, buf, 4, buf, &unLen);

    return RC522_OK;
}

#include "zs040.h"
#include <stddef.h>
#include <string.h>

#define AT_ENTER_MS         200
#define AT_EXIT_MS          100

static ZS040_IO_t *pIO = NULL;

static volatile uint8_t rx_buf[ZS040_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

void ZS040_Init(ZS040_IO_t *io)
{
    pIO = io;
    if (!pIO) return;

    rx_head = 0;
    rx_tail = 0;

    pIO->set_en(0);
    pIO->set_key(0);
}

int8_t ZS040_EnterAtMode(void)
{
    if (!pIO) return ZS040_ERR_NULL;

    rx_head = 0;
    rx_tail = 0;

    pIO->set_key(1);
    pIO->delay_ms(AT_ENTER_MS);

    char resp[32];
    int8_t ret = ZS040_SendAt("AT", resp, sizeof(resp), 1000);

    return ret;
}

int8_t ZS040_ExitAtMode(void)
{
    if (!pIO) return ZS040_ERR_NULL;

    pIO->set_key(0);
    pIO->delay_ms(AT_EXIT_MS);

    return ZS040_OK;
}

int8_t ZS040_SendAt(const char *cmd, char *resp, uint16_t resp_size, uint32_t timeout_ms)
{
    if (!pIO) return ZS040_ERR_NULL;
    if (!cmd || !resp || resp_size < 4) return ZS040_ERR_PARAM;

    rx_head = 0;
    rx_tail = 0;

    uint16_t len = 0;
    while (cmd[len] && len < 64)
        len++;

    pIO->uart_send((const uint8_t *)cmd, len);
    pIO->uart_send((const uint8_t *)"\r\n", 2);

    uint32_t start = pIO->get_ms();
    uint32_t elapsed = 0;
    uint16_t pos = 0;

    while (elapsed < timeout_ms)
    {
        if (rx_tail != rx_head)
        {
            uint8_t byte = rx_buf[rx_tail];
            rx_tail = (rx_tail + 1) % ZS040_RX_BUF_SIZE;

            if (pos < resp_size - 1)
            {
                resp[pos++] = (char)byte;
                resp[pos] = '\0';
            }
        }

        if (strstr(resp, "OK") || strstr(resp, "ok"))
            return ZS040_OK;

        if (strstr(resp, "ERROR") || strstr(resp, "error") || strstr(resp, "Fail"))
            return ZS040_ERR_AT;

        elapsed = pIO->get_ms() - start;
    }

    return ZS040_ERR_TIMEOUT;
}

int8_t ZS040_SetName(const char *name)
{
    if (!pIO) return ZS040_ERR_NULL;
    if (!name) return ZS040_ERR_PARAM;

    char cmd[32];
    uint8_t n = 0;
    while (name[n] && n < 20)
        n++;

    cmd[0] = 'A'; cmd[1] = 'T'; cmd[2] = '+'; cmd[3] = 'N'; cmd[4] = 'A'; cmd[5] = 'M'; cmd[6] = 'E'; cmd[7] = '=';
    for (uint8_t i = 0; i < n; i++)
        cmd[8 + i] = name[i];
    cmd[8 + n] = '\0';

    char resp[32];
    return ZS040_SendAt(cmd, resp, sizeof(resp), 2000);
}

int8_t ZS040_SetPswd(const char *pswd)
{
    if (!pIO) return ZS040_ERR_NULL;
    if (!pswd) return ZS040_ERR_PARAM;

    char cmd[32];
    uint8_t n = 0;
    while (pswd[n] && n < 16)
        n++;

    cmd[0] = 'A'; cmd[1] = 'T'; cmd[2] = '+'; cmd[3] = 'P'; cmd[4] = 'S'; cmd[5] = 'W'; cmd[6] = 'D'; cmd[7] = '=';
    for (uint8_t i = 0; i < n; i++)
        cmd[8 + i] = pswd[i];
    cmd[8 + n] = '\0';

    char resp[32];
    return ZS040_SendAt(cmd, resp, sizeof(resp), 2000);
}

static uint32_t baud_to_reg(uint32_t baud)
{
    if (baud <= 1200)    return 1;
    if (baud <= 2400)    return 2;
    if (baud <= 4800)    return 3;
    if (baud <= 9600)    return 4;
    if (baud <= 19200)   return 5;
    if (baud <= 38400)   return 6;
    if (baud <= 57600)   return 7;
    if (baud <= 115200)  return 8;
    if (baud <= 230400)  return 9;
    if (baud <= 460800)  return 10;
    return 8;
}

int8_t ZS040_SetBaud(uint32_t baud)
{
    if (!pIO) return ZS040_ERR_NULL;

    char cmd[24];
    cmd[0] = 'A'; cmd[1] = 'T'; cmd[2] = '+'; cmd[3] = 'U'; cmd[4] = 'A'; cmd[5] = 'R'; cmd[6] = 'T'; cmd[7] = '=';

    uint32_t reg = baud_to_reg(baud);
    uint8_t p = 8;
    if (reg >= 10)
    {
        cmd[p++] = '0' + (reg / 10);
        reg %= 10;
    }
    cmd[p++] = '0' + reg;
    cmd[p++] = ','; cmd[p++] = '0'; cmd[p++] = ','; cmd[p++] = '0';
    cmd[p] = '\0';

    char resp[32];
    return ZS040_SendAt(cmd, resp, sizeof(resp), 2000);
}

int8_t ZS040_SetRole(ZS040_Role_t role)
{
    if (!pIO) return ZS040_ERR_NULL;

    char cmd[16];
    cmd[0] = 'A'; cmd[1] = 'T'; cmd[2] = '+'; cmd[3] = 'R'; cmd[4] = 'O'; cmd[5] = 'L'; cmd[6] = 'E'; cmd[7] = '=';
    cmd[8] = (role == ZS040_ROLE_MASTER) ? '1' : '0';
    cmd[9] = '\0';

    char resp[32];
    return ZS040_SendAt(cmd, resp, sizeof(resp), 2000);
}

ZS040_State_t ZS040_GetState(void)
{
    if (!pIO) return ZS040_STATE_DISCONNECTED;

    return (pIO->read_state() == 1) ? ZS040_STATE_CONNECTED : ZS040_STATE_DISCONNECTED;
}

void ZS040_PutByte(uint8_t byte)
{
    uint16_t next = (rx_head + 1) % ZS040_RX_BUF_SIZE;
    if (next != rx_tail)
    {
        rx_buf[rx_head] = byte;
        rx_head = next;
    }
}

int8_t ZS040_ReadByte(uint8_t *byte)
{
    if (!byte) return ZS040_ERR_PARAM;
    if (rx_tail == rx_head) return -1;

    *byte = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % ZS040_RX_BUF_SIZE;
    return ZS040_OK;
}

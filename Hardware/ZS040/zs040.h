#ifndef __ZS040_H
#define __ZS040_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码 */
#define ZS040_OK            0
#define ZS040_ERR_NULL    (-1)
#define ZS040_ERR_TIMEOUT (-2)
#define ZS040_ERR_AT      (-3)
#define ZS040_ERR_PARAM   (-4)

/* AT 响应缓冲区大小 */
#define ZS040_RX_BUF_SIZE   128

/* 蓝牙状态 */
typedef enum {
    ZS040_STATE_DISCONNECTED = 0,
    ZS040_STATE_CONNECTED,
} ZS040_State_t;

/* 角色 */
typedef enum {
    ZS040_ROLE_SLAVE  = 0,
    ZS040_ROLE_MASTER = 1,
} ZS040_Role_t;

typedef struct {
    void (*uart_send)(const uint8_t *data, uint16_t len);
    void (*uart_recv_cb)(uint8_t byte);
    void (*set_en)(uint8_t level);
    void (*set_key)(uint8_t level);
    uint8_t (*read_state)(void);
    uint32_t (*get_ms)(void);
    void (*delay_ms)(uint32_t ms);
} ZS040_IO_t;

void ZS040_Init(ZS040_IO_t *io);
int8_t ZS040_EnterAtMode(void);
int8_t ZS040_ExitAtMode(void);
int8_t ZS040_SendAt(const char *cmd, char *resp, uint16_t resp_size, uint32_t timeout_ms);
int8_t ZS040_SetName(const char *name);
int8_t ZS040_SetPswd(const char *pswd);
int8_t ZS040_SetBaud(uint32_t baud);
int8_t ZS040_SetRole(ZS040_Role_t role);
ZS040_State_t ZS040_GetState(void);

#ifdef __cplusplus
}
#endif

#endif

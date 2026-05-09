#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ENCODER_OK            0
#define ENCODER_ERR_NULL    (-1)

typedef enum {
    ENCODER_DIR_NONE  = 0,
    ENCODER_DIR_CW    = 1,
    ENCODER_DIR_CCW   = -1,
} ENCODER_Dir_t;

typedef struct {
    uint8_t (*read_a)(void);
    uint8_t (*read_b)(void);
    uint8_t (*read_btn)(void);
    uint32_t (*get_ms)(void);
} ENCODER_IO_t;

void ENCODER_Init(ENCODER_IO_t *io);
void ENCODER_Process(void);
void ENCODER_ExtiHandler(uint8_t pin_a, uint8_t pin_b);
int32_t ENCODER_GetCount(void);
void ENCODER_SetCount(int32_t count);
ENCODER_Dir_t ENCODER_GetDirection(void);
uint8_t ENCODER_GetBtnPress(void);
uint8_t ENCODER_GetBtnLongPress(void);

#ifdef __cplusplus
}
#endif

#endif

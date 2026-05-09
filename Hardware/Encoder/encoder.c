#include "encoder.h"
#include <stddef.h>

#define ENCODER_DEBOUNCE_MS     20
#define ENCODER_LONG_PRESS_MS   800

static const int8_t encoder_state_table[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0,
};

static ENCODER_IO_t *pIO = NULL;
static int32_t encoder_count = 0;
static uint8_t prev_ab = 0;
static ENCODER_Dir_t last_dir = ENCODER_DIR_NONE;

static uint8_t btn_last = 1;
static uint8_t btn_press_flag = 0;
static uint8_t btn_long_flag = 0;
static uint32_t btn_press_start = 0;
static uint8_t btn_debounce_pending = 0;
static uint32_t btn_debounce_start = 0;

void ENCODER_Init(ENCODER_IO_t *io)
{
    pIO = io;
    if (pIO)
    {
        prev_ab = (pIO->read_a() << 1) | pIO->read_b();
        btn_last = pIO->read_btn();
    }
}

void ENCODER_ExtiHandler(uint8_t pin_a, uint8_t pin_b)
{
    uint8_t curr_ab = (pin_a << 1) | pin_b;
    if (curr_ab != prev_ab)
    {
        int8_t step = encoder_state_table[(prev_ab << 2) | curr_ab];
        if (step != 0)
        {
            encoder_count += step;
            last_dir = (step > 0) ? ENCODER_DIR_CW : ENCODER_DIR_CCW;
        }
        prev_ab = curr_ab;
    }
}

void ENCODER_Process(void)
{
    if (!pIO) return;

    uint8_t btn_curr = pIO->read_btn();

    if (btn_debounce_pending)
    {
        if (pIO->get_ms() - btn_debounce_start >= ENCODER_DEBOUNCE_MS)
        {
            btn_debounce_pending = 0;
            btn_last = btn_curr;

            if (btn_last == 0)
            {
                btn_press_start = pIO->get_ms();
            }
            else
            {
                uint32_t held = pIO->get_ms() - btn_press_start;
                if (held >= ENCODER_LONG_PRESS_MS)
                    btn_long_flag = 1;
                else
                    btn_press_flag = 1;
            }
        }
    }
    else
    {
        if (btn_curr != btn_last)
        {
            btn_debounce_pending = 1;
            btn_debounce_start = pIO->get_ms();
        }
    }
}

int32_t ENCODER_GetCount(void)
{
    return encoder_count;
}

void ENCODER_SetCount(int32_t count)
{
    encoder_count = count;
}

ENCODER_Dir_t ENCODER_GetDirection(void)
{
    ENCODER_Dir_t dir = last_dir;
    last_dir = ENCODER_DIR_NONE;
    return dir;
}

uint8_t ENCODER_GetBtnPress(void)
{
    uint8_t flag = btn_press_flag;
    btn_press_flag = 0;
    return flag;
}

uint8_t ENCODER_GetBtnLongPress(void)
{
    uint8_t flag = btn_long_flag;
    btn_long_flag = 0;
    return flag;
}

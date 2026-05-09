#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * 128×64 I2C OLED 驱动
 * 支持 1.3" SH1106 / 0.96" SSD1306 双控制器
 * 分辨率 128×64, 帧缓冲 1024 字节
 */

#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          (OLED_HEIGHT / 8)

/* 控制器类型 (默认 SSD1306) */
typedef enum {
    OLED_CONTROLLER_SSD1306 = 0,
    OLED_CONTROLLER_SH1106  = 1,
} OLED_Controller_t;

typedef struct {
    void (*write_cmd)(uint8_t cmd);
    void (*write_data)(const uint8_t *data, uint16_t len);
} OLED_IO_t;

void OLED_Init(OLED_IO_t *io);
void OLED_InitEx(OLED_IO_t *io, OLED_Controller_t ctrl);
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_Clear(void);
void OLED_Display(void);

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint8_t x, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void OLED_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void OLED_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen, uint8_t size);
void OLED_SetCursor(uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif

#endif

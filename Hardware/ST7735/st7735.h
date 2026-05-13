#ifndef __ST7735_H
#define __ST7735_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * 1.8" TFT LCD ST7735 驱动
 * 分辨率 128×160, 16-bit RGB565 色彩, SPI 接口
 * 无帧缓冲, 直接操作显示控制器 GRAM
 */

#define ST7735_WIDTH           128
#define ST7735_HEIGHT          160

#define ST7735_BLACK           0x0000
#define ST7735_WHITE           0xFFFF
#define ST7735_RED             0xF800
#define ST7735_GREEN           0x07E0
#define ST7735_BLUE            0x001F
#define ST7735_YELLOW          0xFFE0
#define ST7735_CYAN            0x07FF
#define ST7735_MAGENTA         0xF81F
#define ST7735_GRAY            0x8410
#define ST7735_ORANGE          0xFD20

#define ST7735_OK              0
#define ST7735_ERR_NULL       (-1)

typedef struct {
    void (*write_cmd)(uint8_t cmd);
    void (*write_data)(const uint8_t *data, uint16_t len);
    void (*delay_ms)(uint32_t ms);
} ST7735_IO_t;

void ST7735_Init(ST7735_IO_t *io);
void ST7735_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void ST7735_Fill(uint16_t color);
void ST7735_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void ST7735_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void ST7735_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void ST7735_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void ST7735_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void ST7735_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg, uint8_t size);
void ST7735_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void ST7735_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint16_t color, uint16_t bg, uint8_t size);
void ST7735_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen, uint16_t color, uint16_t bg, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif

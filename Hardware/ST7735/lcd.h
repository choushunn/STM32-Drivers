#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "st7735.h"
#include <stdio.h>

#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define BRED           0xF81F
#define GRED           0xFFE0
#define GBLUE          0x07FF
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define BROWN          0xBC40
#define BRRED          0xFC07
#define GRAY           0x8430
#define DARKBLUE       0x01CF
#define LIGHTBLUE      0x7D7C
#define GRAYBLUE       0x5458

extern ST7735_Object_t st7735_pObj;
extern ST7735_IO_t st7735_pIO;
extern uint32_t st7735_id;

extern uint16_t POINT_COLOR;
extern uint16_t BACK_COLOR;

void LCD_SetBrightness(uint32_t Brightness);
uint32_t LCD_GetBrightness(void);
void LCD_Light(uint32_t Brightness_Dis, uint32_t time);
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);
void LCD_TestPattern(void);

#endif

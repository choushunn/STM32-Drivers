#include "lcd.h"
#include "font.h"
#include "spi.h"
#include "tim.h"

#define LCD_RS_SET      HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_SET)
#define LCD_RS_RESET    HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_RESET)
#define LCD_CS_SET      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_CS_RESET    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)

#define SPI_Drv         (&hspi4)
#define delay_ms        HAL_Delay
#define get_tick        HAL_GetTick

#define LCD_Brightness_timer  &htim1
#define LCD_Brightness_channel TIM_CHANNEL_2

static int32_t lcd_init(void);
static int32_t lcd_gettick(void);
static int32_t lcd_writereg(uint8_t reg, uint8_t* pdata, uint32_t length);
static int32_t lcd_readreg(uint8_t reg, uint8_t* pdata);
static int32_t lcd_senddata(const uint8_t* pdata, uint32_t length);
static int32_t lcd_recvdata(uint8_t* pdata, uint32_t length);

ST7735_IO_t st7735_pIO = {
    lcd_init,
    NULL,
    lcd_writereg,
    lcd_readreg,
    lcd_senddata,
    lcd_recvdata,
    lcd_gettick
};

ST7735_Object_t st7735_pObj;
uint32_t st7735_id;

uint16_t POINT_COLOR = 0xFFFF;
uint16_t BACK_COLOR = BLACK;

void LCD_SetBrightness(uint32_t Brightness) {
    __HAL_TIM_SetCompare(LCD_Brightness_timer, LCD_Brightness_channel, Brightness);
}

uint32_t LCD_GetBrightness(void) {
    return __HAL_TIM_GetCompare(LCD_Brightness_timer, LCD_Brightness_channel);
}

void LCD_Light(uint32_t Brightness_Dis, uint32_t time) {
    uint32_t Brightness_Now = LCD_GetBrightness();
    uint32_t time_now = 0;

    if (Brightness_Now == Brightness_Dis || time == 0) return;

    float k = (float)(Brightness_Now - Brightness_Dis) / (float)(0 - time);
    uint32_t tick = get_tick();

    while (time_now < time) {
        delay_ms(1);
        time_now = get_tick() - tick;
        float set = (float)time_now * k + (float)Brightness_Now;
        LCD_SetBrightness((uint32_t)set);
    }
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode) {
    uint8_t temp, t1;
    uint16_t y0 = y;
    uint16_t x0 = x;
    uint16_t colortemp = POINT_COLOR;
    uint32_t h, w;
    uint16_t count = 0;

    ST7735_GetXSize(&st7735_pObj, &w);
    ST7735_GetYSize(&st7735_pObj, &h);

    num -= ' ';

    uint16_t write[size][size == 12 ? 6 : 8];

    if (!mode) {
        for (t1 = 0; t1 < size; t1++) {
            temp = (size == 12) ? asc2_1206[num][t1] : asc2_1608[num][t1];
            for (uint8_t t = 0; t < 8; t++) {
                POINT_COLOR = (temp & 0x80) ?
                    ((colortemp & 0xFF) << 8) | (colortemp >> 8) :
                    ((BACK_COLOR & 0xFF) << 8) | (BACK_COLOR >> 8);
                write[count][t1 / 2] = POINT_COLOR;
                count = (count + 1) % size;
                temp <<= 1;
                if (++y >= h) { POINT_COLOR = colortemp; return; }
                if (y - y0 == size) { y = y0; if (++x >= w) { POINT_COLOR = colortemp; return; } break; }
            }
        }
    } else {
        for (t1 = 0; t1 < size; t1++) {
            temp = (size == 12) ? asc2_1206[num][t1] : asc2_1608[num][t1];
            for (uint8_t t = 0; t < 8; t++) {
                if (temp & 0x80) {
                    write[count][t1 / 2] = ((POINT_COLOR & 0xFF) << 8) | (POINT_COLOR >> 8);
                }
                count = (count + 1) % size;
                temp <<= 1;
                if (++y >= h) { POINT_COLOR = colortemp; return; }
                if (y - y0 == size) { y = y0; if (++x >= w) { POINT_COLOR = colortemp; return; } break; }
            }
        }
    }
    ST7735_FillRGBRect(&st7735_pObj, x0, y0, (uint8_t*)write, (size == 12) ? 6 : 8, size);
    POINT_COLOR = colortemp;
}

void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p) {
    uint8_t x0 = x;
    width += x;
    height += y;

    while (*p <= '~' && *p >= ' ') {
        if (x >= width) { x = x0; y += size; }
        if (y >= height) break;
        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}

void LCD_TestPattern(void) {
    uint32_t w, h;
    ST7735_GetXSize(&st7735_pObj, &w);
    ST7735_GetYSize(&st7735_pObj, &h);

    uint32_t bar_height = h / 6;
    uint32_t y_pos = 0;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, bar_height, RED);
    LCD_ShowString(0, y_pos, w, bar_height, 12, (uint8_t*)"RED");
    y_pos += bar_height;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, bar_height, GREEN);
    LCD_ShowString(0, y_pos, w, bar_height, 12, (uint8_t*)"GREEN");
    y_pos += bar_height;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, bar_height, BLUE);
    LCD_ShowString(0, y_pos, w, bar_height, 12, (uint8_t*)"BLUE");
    y_pos += bar_height;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, bar_height, YELLOW);
    LCD_ShowString(0, y_pos, w, bar_height, 12, (uint8_t*)"YELLOW");
    y_pos += bar_height;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, bar_height, CYAN);
    LCD_ShowString(0, y_pos, w, bar_height, 12, (uint8_t*)"CYAN");
    y_pos += bar_height;

    ST7735_FillRect(&st7735_pObj, 0, y_pos, w, h - y_pos, MAGENTA);
    LCD_ShowString(0, y_pos, w, h - y_pos, 12, (uint8_t*)"MAGENTA");
}

static int32_t lcd_init(void) {
    HAL_TIMEx_PWMN_Start(LCD_Brightness_timer, LCD_Brightness_channel);
    return ST7735_OK;
}

static int32_t lcd_gettick(void) {
    return HAL_GetTick();
}

static int32_t lcd_writereg(uint8_t reg, uint8_t* pdata, uint32_t length) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_RESET;
    result = HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
    LCD_RS_SET;
    if (length > 0) {
        result += HAL_SPI_Transmit(SPI_Drv, pdata, length, 500);
    }
    LCD_CS_SET;
    return (result > 0) ? -1 : 0;
}

static int32_t lcd_readreg(uint8_t reg, uint8_t* pdata) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_RESET;
    result = HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
    LCD_RS_SET;
    result += HAL_SPI_Receive(SPI_Drv, pdata, 1, 500);
    LCD_CS_SET;
    return (result > 0) ? -1 : 0;
}

static int32_t lcd_senddata(const uint8_t* pdata, uint32_t length) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_SET;
    result = HAL_SPI_Transmit(SPI_Drv, (uint8_t *)pdata, length, 100);
    LCD_CS_SET;
    return (result > 0) ? -1 : 0;
}

static int32_t lcd_recvdata(uint8_t* pdata, uint32_t length) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_SET;
    result = HAL_SPI_Receive(SPI_Drv, pdata, length, 500);
    LCD_CS_SET;
    return (result > 0) ? -1 : 0;
}

#include "oled.h"
#include "oled_font.h"
#include <string.h>

static uint8_t OLED_Buffer[OLED_WIDTH * OLED_PAGES];
static OLED_IO_t *pIO = NULL;
static OLED_Controller_t oled_ctrl = OLED_CONTROLLER_SSD1306;

/* 硬件初始化序列, 复用 SSD1306/SH1106 公用的命令,
   SH1106 跳过 0x20/0x00(设置内存寻址模式) 并使用不同的对比度值 0x7F */
static void oled_init_seq(void)
{
    pIO->write_cmd(0xAE);
    pIO->write_cmd(0xD5);
    pIO->write_cmd(0x80);
    pIO->write_cmd(0xA8);
    pIO->write_cmd(0x3F);
    pIO->write_cmd(0xD3);
    pIO->write_cmd(0x00);
    pIO->write_cmd(0x40);
    pIO->write_cmd(0x8D);
    pIO->write_cmd(0x14);

    if (oled_ctrl != OLED_CONTROLLER_SH1106)
    {
        pIO->write_cmd(0x20);
        pIO->write_cmd(0x00);
    }

    pIO->write_cmd(0xA1);
    pIO->write_cmd(0xC8);
    pIO->write_cmd(0x81);
    pIO->write_cmd((oled_ctrl == OLED_CONTROLLER_SH1106) ? 0x7F : 0xCF);
    pIO->write_cmd(0xD9);
    pIO->write_cmd(0xF1);
    pIO->write_cmd(0xDB);
    pIO->write_cmd(0x40);
    pIO->write_cmd(0xA4);
    pIO->write_cmd(0xA6);
    pIO->write_cmd(0xDA);
    pIO->write_cmd(0x12);
    pIO->write_cmd(0xAF);
}

/* 默认使用 SSD1306 控制器 */
void OLED_Init(OLED_IO_t *io)
{
    OLED_InitEx(io, OLED_CONTROLLER_SSD1306);
}

/* 注册 IO 回调并执行初始化序列 */
void OLED_InitEx(OLED_IO_t *io, OLED_Controller_t ctrl)
{
    pIO = io;
    oled_ctrl = ctrl;

    oled_init_seq();

    OLED_Clear();
    OLED_Display();
}

void OLED_DisplayOn(void)
{
    if (pIO) pIO->write_cmd(0xAF);
}

void OLED_DisplayOff(void)
{
    if (pIO) pIO->write_cmd(0xAE);
}

/* 清空帧缓冲, 不立即刷新到屏幕 */
void OLED_Clear(void)
{
    memset(OLED_Buffer, 0x00, sizeof(OLED_Buffer));
}

/* 将帧缓冲通过 IO 回调逐页发送到 OLED,
   SH1106 需要 2 列偏移, SSD1306 不需要 */
void OLED_Display(void)
{
    if (!pIO) return;

    uint8_t col_ofs = (oled_ctrl == OLED_CONTROLLER_SH1106) ? 2 : 0;

    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        pIO->write_cmd(0xB0 + page);
        pIO->write_cmd(col_ofs & 0x0F);
        pIO->write_cmd(0x10 | ((col_ofs >> 4) & 0x0F));
        pIO->write_data(&OLED_Buffer[page * OLED_WIDTH], OLED_WIDTH);
    }
}

/* 设置或清除 (x,y) 处的像素 */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;

    if (color)
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
    }
    else
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

/* Bresenham 直线算法 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    int16_t dx, dy, sx, sy, err, e2;

    dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    while (1)
    {
        OLED_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

/* 画空心矩形, 由四条直线组成 */
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    OLED_DrawLine(x, y, x + w - 1, y, color);
    OLED_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    OLED_DrawLine(x, y, x, y + h - 1, color);
    OLED_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

/* 填充矩形区域 */
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    for (uint8_t i = 0; i < h; i++)
    {
        for (uint8_t j = 0; j < w; j++)
        {
            OLED_DrawPixel(x + j, y + i, color);
        }
    }
}

/* Bresenham 画圆算法, 利用八对称性只计算 1/8 圆弧 */
void OLED_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    OLED_DrawPixel(x0, y0 + r, color);
    OLED_DrawPixel(x0, y0 - r, color);
    OLED_DrawPixel(x0 + r, y0, color);
    OLED_DrawPixel(x0 - r, y0, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        OLED_DrawPixel(x0 + x, y0 + y, color);
        OLED_DrawPixel(x0 - x, y0 + y, color);
        OLED_DrawPixel(x0 + x, y0 - y, color);
        OLED_DrawPixel(x0 - x, y0 - y, color);
        OLED_DrawPixel(x0 + y, y0 + x, color);
        OLED_DrawPixel(x0 - y, y0 + x, color);
        OLED_DrawPixel(x0 + y, y0 - x, color);
        OLED_DrawPixel(x0 - y, y0 - x, color);
    }
}

/* 先画垂直中轴线, 再用 Bresenham 步进填充两侧 */
void OLED_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    for (int16_t i = y0 - r; i <= y0 + r; i++)
    {
        OLED_DrawPixel(x0, i, color);
    }

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        for (int16_t i = y0 - y; i <= y0 + y; i++)
        {
            OLED_DrawPixel(x0 + x, i, color);
            OLED_DrawPixel(x0 - x, i, color);
        }
        for (int16_t i = y0 - x; i <= y0 + x; i++)
        {
            OLED_DrawPixel(x0 + y, i, color);
            OLED_DrawPixel(x0 - y, i, color);
        }
    }
}

/* 设置 OLED 内部页地址和列地址 */
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    uint8_t col_ofs = (oled_ctrl == OLED_CONTROLLER_SH1106) ? 2 : 0;
    x += col_ofs;
    pIO->write_cmd(0xB0 + y);
    pIO->write_cmd(x & 0x0F);
    pIO->write_cmd(0x10 | ((x >> 4) & 0x0F));
}

/* 在指定位置显示单个字符, size=1 直接操作像素, size>1 用像素块放大 5x7 字库 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size)
{
    if (ch < ' ' || ch > '~')
        ch = ' ';

    if (size == 1)
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            uint8_t data;
            if (i < 5)
                data = Font_5x7[ch - ' '][i];
            else
                data = 0x00;

            for (uint8_t j = 0; j < 8; j++)
            {
                if (data & (1 << j))
                    OLED_DrawPixel(x + i, y + j, 1);
                else
                    OLED_DrawPixel(x + i, y + j, 0);
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            uint8_t data;
            if (i < 5)
                data = Font_5x7[ch - ' '][i];
            else
                data = 0x00;

            for (uint8_t j = 0; j < 8; j++)
            {
                if (data & (1 << j))
                {
                    OLED_FillRect(x + i * size, y + j * size, size, size, 1);
                }
                else
                {
                    OLED_FillRect(x + i * size, y + j * size, size, size, 0);
                }
            }
        }
    }
}

/* 逐字符显示字符串, 超宽自动换行 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    while (*str)
    {
        OLED_ShowChar(x, y, *str, size);
        x += (size == 1) ? 6 : (6 * size);
        str++;
        if (x > OLED_WIDTH - 6)
        {
            x = 0;
            y += (size == 1) ? 8 : (8 * size);
        }
    }
}

/* 工具函数: 原地反转字符数组 */
static void OLED_ReverseStr(char *str, uint8_t len)
{
    for (uint8_t i = 0; i < len / 2; i++)
    {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

/* 将 uint32 转为字符串后显示, len 控制最小宽度补空格 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    char buf[12];
    uint8_t i = 0;

    if (num == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        while (num > 0 && i < sizeof(buf) - 1)
        {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }

    while (i < len)
        buf[i++] = ' ';

    OLED_ReverseStr(buf, i);
    buf[i] = '\0';

    OLED_ShowString(x, y, buf, size);
}

/* 将 float 转为字符串后显示, 支持负数、整数部分和小数部分位数指定 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen, uint8_t size)
{
    char buf[16];
    uint8_t pos = 0;
    uint8_t i;

    if (num < 0)
    {
        buf[pos++] = '-';
        num = -num;
    }

    uint32_t intPart = (uint32_t)num;

    if (intPart == 0)
    {
        buf[pos++] = '0';
    }
    else
    {
        char tmp[12];
        uint8_t tlen = 0;
        while (intPart > 0)
        {
            tmp[tlen++] = '0' + (intPart % 10);
            intPart /= 10;
        }
        for (i = 0; i < tlen; i++)
            buf[pos++] = tmp[tlen - 1 - i];
    }

    if (decLen > 0)
    {
        buf[pos++] = '.';

        float decPartF = num - (float)((uint32_t)num);
        if (decPartF < 0)
            decPartF = 0;

        uint32_t multiplier = 1;
        for (i = 0; i < decLen; i++)
            multiplier *= 10;

        uint32_t decPart = (uint32_t)(decPartF * multiplier + 0.5f);

        for (i = 0; i < decLen; i++)
        {
            buf[pos + decLen - 1 - i] = '0' + (decPart % 10);
            decPart /= 10;
        }
        pos += decLen;
    }

    buf[pos] = '\0';
    OLED_ShowString(x, y, buf, size);
}

/* 显示测试图案, 包含矩形、对角线、圆、字符、数字、浮点数各尺寸 */
void OLED_TestPattern(void)
{
    OLED_Clear();

    /* 画边框 */
    OLED_DrawRect(0, 0, 127, 63, 1);
    OLED_DrawRect(2, 2, 123, 59, 1);

    /* 画对角线 */
    OLED_DrawLine(4, 4, 123, 59, 1);
    OLED_DrawLine(123, 4, 4, 59, 1);

    /* 左上角：画圆 + 填充圆 */
    OLED_DrawCircle(20, 20, 12, 1);
    OLED_FillCircle(58, 20, 12, 1);

    /* 右上角：显示字符 1x ~ 3x */
    OLED_ShowString(70, 4, "ABC", 1);
    OLED_ShowString(70, 16, "ABC", 2);
    OLED_ShowString(70, 36, "ABC", 3);

    /* 底部：显示数字和浮点数 */
    OLED_ShowNum(4, 40, 1234, 4, 1);
    OLED_ShowFloat(40, 40, 3.14f, 2, 2, 1);
    OLED_ShowString(4, 52, "OLED OK!", 1);
    OLED_ShowNum(80, 52, 8888, 4, 1);

    OLED_Display();    
}

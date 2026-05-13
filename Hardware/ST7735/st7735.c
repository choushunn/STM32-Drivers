#include "st7735.h"
#include <stddef.h>

static ST7735_IO_t *pIO = NULL;

static const uint8_t Font_5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00}, {0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50}, {0x00,0x00,0x07,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00}, {0x00,0x41,0x22,0x1C,0x00},
    {0x08,0x2A,0x1C,0x2A,0x08}, {0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08},
    {0x00,0x60,0x60,0x00,0x00}, {0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E},
    {0x00,0x36,0x36,0x00,0x00}, {0x00,0x56,0x36,0x00,0x00},
    {0x00,0x08,0x14,0x22,0x41}, {0x14,0x14,0x14,0x14,0x14},
    {0x41,0x22,0x14,0x08,0x00}, {0x02,0x01,0x51,0x09,0x06},
    {0x32,0x49,0x79,0x41,0x3E}, {0x7E,0x11,0x11,0x11,0x7E},
    {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x01,0x01}, {0x3E,0x41,0x41,0x51,0x32},
    {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40}, {0x7F,0x02,0x04,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46}, {0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F}, {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63}, {0x03,0x04,0x78,0x04,0x03},
    {0x61,0x51,0x49,0x45,0x43}, {0x00,0x00,0x7F,0x41,0x41},
    {0x02,0x04,0x08,0x10,0x20}, {0x41,0x41,0x7F,0x00,0x00},
    {0x04,0x02,0x01,0x02,0x04}, {0x80,0x80,0x80,0x80,0x80},
    {0x00,0x01,0x02,0x04,0x00}, {0x20,0x54,0x54,0x54,0x78},
    {0x7F,0x48,0x44,0x44,0x38}, {0x38,0x44,0x44,0x44,0x20},
    {0x38,0x44,0x44,0x48,0x7F}, {0x38,0x54,0x54,0x54,0x18},
    {0x08,0x7E,0x09,0x01,0x02}, {0x08,0x14,0x54,0x54,0x3C},
    {0x7F,0x08,0x04,0x04,0x78}, {0x00,0x44,0x7D,0x40,0x00},
    {0x20,0x40,0x44,0x3D,0x00}, {0x00,0x7F,0x10,0x28,0x44},
    {0x00,0x41,0x7F,0x40,0x00}, {0x7C,0x04,0x18,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78}, {0x38,0x44,0x44,0x44,0x38},
    {0x7C,0x14,0x14,0x14,0x08}, {0x08,0x14,0x14,0x18,0x7C},
    {0x7C,0x08,0x04,0x04,0x08}, {0x48,0x54,0x54,0x54,0x20},
    {0x04,0x3F,0x44,0x40,0x20}, {0x3C,0x40,0x40,0x20,0x7C},
    {0x1C,0x20,0x40,0x20,0x1C}, {0x3C,0x40,0x30,0x40,0x3C},
    {0x44,0x28,0x10,0x28,0x44}, {0x0C,0x50,0x50,0x50,0x3C},
    {0x44,0x64,0x54,0x4C,0x44}, {0x00,0x08,0x36,0x41,0x00},
    {0x00,0x00,0x77,0x00,0x00}, {0x00,0x41,0x36,0x08,0x00},
    {0x02,0x01,0x02,0x04,0x02}, {0x3C,0x26,0x23,0x26,0x3C},
};

static void ST7735_WriteCmdData(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    pIO->write_cmd(cmd);
    if (len && data)
        pIO->write_data(data, len);
}

void ST7735_Init(ST7735_IO_t *io)
{
    static const uint8_t init_cmds[] = {
        0x01, 0x80, 150,
        0x11, 0x80, 255,
        0xB1, 3, 0x01, 0x2C, 0x2D,
        0xB2, 3, 0x01, 0x2C, 0x2D,
        0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
        0xB4, 1, 0x07,
        0xC0, 3, 0xA2, 0x02, 0x84,
        0xC1, 1, 0xC5,
        0xC2, 2, 0x0A, 0x00,
        0xC3, 2, 0x8A, 0x2A,
        0xC4, 2, 0x8A, 0xEE,
        0xC5, 1, 0x0E,
        0x20, 0x00,
        0x36, 1, 0xC0,
        0x3A, 1, 0x05,
        0x2A, 4, 0x00, 0x02, 0x00, 0x81,
        0x2B, 4, 0x00, 0x01, 0x00, 0xA0,
        0x21, 0x00,
        0x13, 0x00,
        0x11, 0x80, 255,
        0x29, 0x80, 255,
    };

    pIO = io;

    for (uint16_t i = 0; i < sizeof(init_cmds);)
    {
        uint8_t cmd = init_cmds[i++];
        uint8_t len = init_cmds[i++];
        uint8_t delay_ms = (len & 0x80) ? init_cmds[i] : 0;
        len &= 0x7F;

        pIO->write_cmd(cmd);
        if (len > 0)
            pIO->write_data(&init_cmds[i], len);

        i += len;

        if (delay_ms)
            pIO->delay_ms(delay_ms);
    }

    ST7735_Fill(0x0000);
}

void ST7735_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t col[4] = { 0, x0 + 2, 0, x1 + 2 };
    uint8_t row[4] = { 0, y0 + 1, 0, y1 + 1 };

    ST7735_WriteCmdData(0x2A, col, 4);
    ST7735_WriteCmdData(0x2B, row, 4);
    pIO->write_cmd(0x2C);
}

void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color)
{
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;

    ST7735_SetWindow(x, y, x, y);
    uint8_t buf[2] = { color >> 8, color & 0xFF };
    pIO->write_data(buf, 2);
}

void ST7735_Fill(uint16_t color)
{
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    ST7735_SetWindow(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1);

    for (uint32_t i = 0; i < (uint32_t)ST7735_WIDTH * ST7735_HEIGHT; i++)
    {
        pIO->write_data(&hi, 1);
        pIO->write_data(&lo, 1);
    }
}

void ST7735_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;
    if (x + w > ST7735_WIDTH)  w = ST7735_WIDTH - x;
    if (y + h > ST7735_HEIGHT) h = ST7735_HEIGHT - y;

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    ST7735_SetWindow(x, y, x + w - 1, y + h - 1);

    for (uint32_t i = 0; i < (uint32_t)w * h; i++)
    {
        pIO->write_data(&hi, 1);
        pIO->write_data(&lo, 1);
    }
}

void ST7735_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
    ST7735_DrawLine(x, y, x + w - 1, y, color);
    ST7735_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    ST7735_DrawLine(x, y, x, y + h - 1, color);
    ST7735_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void ST7735_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    int16_t dx, dy, sx, sy, err, e2;

    dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    while (1)
    {
        ST7735_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void ST7735_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ST7735_DrawPixel(x0, y0 + r, color);
    ST7735_DrawPixel(x0, y0 - r, color);
    ST7735_DrawPixel(x0 + r, y0, color);
    ST7735_DrawPixel(x0 - r, y0, color);

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

        ST7735_DrawPixel(x0 + x, y0 + y, color);
        ST7735_DrawPixel(x0 - x, y0 + y, color);
        ST7735_DrawPixel(x0 + x, y0 - y, color);
        ST7735_DrawPixel(x0 - x, y0 - y, color);
        ST7735_DrawPixel(x0 + y, y0 + x, color);
        ST7735_DrawPixel(x0 - y, y0 + x, color);
        ST7735_DrawPixel(x0 + y, y0 - x, color);
        ST7735_DrawPixel(x0 - y, y0 - x, color);
    }
}

void ST7735_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    for (int16_t i = y0 - r; i <= y0 + r; i++)
        ST7735_DrawPixel(x0, i, color);

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
            ST7735_DrawPixel(x0 + x, i, color);
            ST7735_DrawPixel(x0 - x, i, color);
        }
        for (int16_t i = y0 - x; i <= y0 + x; i++)
        {
            ST7735_DrawPixel(x0 + y, i, color);
            ST7735_DrawPixel(x0 - y, i, color);
        }
    }
}

void ST7735_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg, uint8_t size)
{
    if (ch < ' ' || ch > '~')
        ch = ' ';

    uint8_t idx = ch - ' ';

    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t data = Font_5x7[idx][i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (data & (1 << j))
            {
                if (size == 1)
                    ST7735_DrawPixel(x + i, y + j, color);
                else
                    ST7735_FillRect(x + i * size, y + j * size, size, size, color);
            }
            else if (bg != color)
            {
                if (size == 1)
                    ST7735_DrawPixel(x + i, y + j, bg);
                else
                    ST7735_FillRect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }

    if (size == 1)
    {
        for (uint8_t j = 0; j < 8; j++)
            ST7735_DrawPixel(x + 5, y + j, bg);
    }
    else
    {
        ST7735_FillRect(x + 5 * size, y, size, 8 * size, bg);
    }
}

void ST7735_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
    while (*str)
    {
        if (x > ST7735_WIDTH - (6 * size))
        {
            x = 0;
            y += (size == 1) ? 8 : (8 * size);
        }
        if (y > ST7735_HEIGHT - (8 * size))
            return;

        ST7735_ShowChar(x, y, *str, color, bg, size);
        x += (size == 1) ? 6 : (6 * size);
        str++;
    }
}

static void ST7735_ReverseStr(char *str, uint8_t len)
{
    for (uint8_t i = 0; i < len / 2; i++)
    {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

void ST7735_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint16_t color, uint16_t bg, uint8_t size)
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

    ST7735_ReverseStr(buf, i);
    buf[i] = '\0';

    ST7735_ShowString(x, y, buf, color, bg, size);
}

void ST7735_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t intLen, uint8_t decLen, uint16_t color, uint16_t bg, uint8_t size)
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

    char tmp[12];
    uint8_t tlen = 0;
    if (intPart == 0)
    {
        tmp[tlen++] = '0';
    }
    else
    {
        while (intPart > 0)
        {
            tmp[tlen++] = '0' + (intPart % 10);
            intPart /= 10;
        }
    }
    while (tlen < intLen)
        tmp[tlen++] = ' ';

    for (i = 0; i < tlen; i++)
        buf[pos++] = tmp[tlen - 1 - i];

    if (decLen > 0)
    {
        buf[pos++] = '.';

        float decPartF = num - (float)((uint32_t)num);
        if (decPartF < 0) decPartF = 0;

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
    ST7735_ShowString(x, y, buf, color, bg, size);
}

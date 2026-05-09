#include "oled.h"
#include "oled_font.h"
#include <string.h>

static uint8_t OLED_Buffer[OLED_WIDTH * OLED_PAGES];
static OLED_IO_t *pIO = NULL;
static OLED_Controller_t oled_ctrl = OLED_CONTROLLER_SSD1306;

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

void OLED_Init(OLED_IO_t *io)
{
    OLED_InitEx(io, OLED_CONTROLLER_SSD1306);
}

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

void OLED_Clear(void)
{
    memset(OLED_Buffer, 0x00, sizeof(OLED_Buffer));
}

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

void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    OLED_DrawLine(x, y, x + w - 1, y, color);
    OLED_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    OLED_DrawLine(x, y, x, y + h - 1, color);
    OLED_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

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

void OLED_SetCursor(uint8_t x, uint8_t y)
{
    uint8_t col_ofs = (oled_ctrl == OLED_CONTROLLER_SH1106) ? 2 : 0;
    x += col_ofs;
    pIO->write_cmd(0xB0 + y);
    pIO->write_cmd(x & 0x0F);
    pIO->write_cmd(0x10 | ((x >> 4) & 0x0F));
}

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

static void OLED_ReverseStr(char *str, uint8_t len)
{
    for (uint8_t i = 0; i < len / 2; i++)
    {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

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
    float decPartF = num - (float)intPart;
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

    buf[pos++] = '.';

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

    while ((pos - (num < 0 ? 1 : 0)) < (intLen + 1 + decLen))
        buf[pos++] = ' ';

    buf[pos] = '\0';

    char final[16];
    uint8_t fpos = 0;

    if (num < 0)
        final[fpos++] = '-';

    for (i = 0; i < intLen + 1 + decLen && buf[i] != '\0'; i++);

    uint8_t start = 0;
    if (pos > intLen + 1 + decLen)
        start = pos - (intLen + 1 + decLen);

    for (i = start; i < pos; i++)
        final[fpos++] = buf[i];
    final[fpos] = '\0';

    OLED_ShowString(x, y, final, size);
}

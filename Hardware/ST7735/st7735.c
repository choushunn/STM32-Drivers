#include "st7735.h"

ST7735_LCD_Drv_t ST7735_LCD_Driver = {
    ST7735_Init,
    ST7735_DeInit,
    ST7735_ReadID,
    ST7735_DisplayOn,
    ST7735_DisplayOff,
    ST7735_SetBrightness,
    ST7735_GetBrightness,
    ST7735_SetOrientation,
    ST7735_GetOrientation,
    ST7735_SetCursor,
    ST7735_DrawBitmap,
    ST7735_FillRGBRect,
    ST7735_DrawHLine,
    ST7735_DrawVLine,
    ST7735_FillRect,
    ST7735_GetPixel,
    ST7735_SetPixel,
    ST7735_GetXSize,
    ST7735_GetYSize,
};

static const uint32_t OrientationTab[4][2] = {
    {0x40U, 0xC0U},
    {0x80U, 0x00U},
    {0x20U, 0x60U},
    {0xE0U, 0xA0U}
};

static void ST7735_GetPanelOffset(ST7735_Object_t *pObj, uint32_t *x_ofs, uint32_t *y_ofs) {
    *x_ofs = 0;
    *y_ofs = 0;

    if (pObj->Config.Type == ST7735_0_9_inch_screen) {
        if (pObj->Config.Panel == HannStar_Panel) {
            if (pObj->Config.Orientation <= ST7735_ORIENTATION_PORTRAIT_ROT180) {
                *x_ofs = 26;
                *y_ofs = 1;
            } else {
                *x_ofs = 1;
                *y_ofs = 26;
            }
        } else {
            if (pObj->Config.Orientation <= ST7735_ORIENTATION_PORTRAIT_ROT180) {
                *x_ofs = 24;
                *y_ofs = 0;
            } else {
                *x_ofs = 1;
                *y_ofs = 24;
            }
        }
    } else if (pObj->Config.Type == ST7735_1_8a_inch_screen && pObj->Config.Panel == BOE_Panel) {
        if (pObj->Config.Orientation <= ST7735_ORIENTATION_PORTRAIT_ROT180) {
            *x_ofs = 2;
            *y_ofs = 1;
        } else {
            *x_ofs = 1;
            *y_ofs = 2;
        }
    }
}

static uint8_t ST7735_GetMADCTL(ST7735_Object_t *pObj, uint8_t idx) {
    uint8_t rgb = (pObj->Config.Panel == HannStar_Panel) ? LCD_BGR : LCD_RGB;
    return (uint8_t)OrientationTab[pObj->Config.Orientation][idx] | rgb;
}

static int32_t ST7735_SetDisplayWindow(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height);
static int32_t ST7735_ReadRegWrap(void *Handle, uint8_t Reg, uint8_t* pData);
static int32_t ST7735_WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint32_t Length);
static int32_t ST7735_SendDataWrap(void *Handle, const uint8_t *pData, uint32_t Length);
static int32_t ST7735_RecvDataWrap(void *Handle, uint8_t *pData, uint32_t Length);
static int32_t ST7735_IO_Delay(ST7735_Object_t *pObj, uint32_t Delay);

int32_t ST7735_RegisterBusIO(ST7735_Object_t *pObj, ST7735_IO_t *pIO) {
    if (!pObj) return ST7735_ERROR;

    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.SendData  = pIO->SendData;
    pObj->IO.RecvData  = pIO->RecvData;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg   = ST7735_ReadRegWrap;
    pObj->Ctx.WriteReg  = ST7735_WriteRegWrap;
    pObj->Ctx.SendData  = ST7735_SendDataWrap;
    pObj->Ctx.RecvData  = ST7735_RecvDataWrap;
    pObj->Ctx.handle    = pObj;

    return (pObj->IO.Init) ? pObj->IO.Init() : ST7735_ERROR;
}

int32_t ST7735_Init(ST7735_Object_t *pObj, uint32_t ColorCoding, ST7735_Ctx_t *pConfig) {
    uint8_t tmp;
    int32_t ret;

    if (!pObj || !pConfig) return ST7735_ERROR;

    pObj->Config = *pConfig;

    tmp = 0x00U;
    ret = st7735_write_reg(&pObj->Ctx, ST7735_SW_RESET, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 120);

    tmp = 0x00U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_SW_RESET, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 120);

    tmp = 0x00U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_SLEEP_OUT, &tmp, 1);

    tmp = 0x01U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_FRAME_RATE_CTRL1, &tmp, 0);
    tmp = 0x2CU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x2DU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x01U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_FRAME_RATE_CTRL2, &tmp, 1);
    tmp = 0x2CU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x2DU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x01U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_FRAME_RATE_CTRL3, &tmp, 1);
    tmp = 0x2CU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x2DU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x01U;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x2CU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x2DU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x07U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_FRAME_INVERSION_CTRL, &tmp, 1);

    tmp = 0xA2U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PWR_CTRL1, &tmp, 1);
    tmp = 0x02U;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = 0x84U;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0xC5U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PWR_CTRL2, &tmp, 1);

    tmp = 0x0AU;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PWR_CTRL3, &tmp, 1);
    tmp = 0x00U;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x8AU;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PWR_CTRL4, &tmp, 1);
    tmp = 0x2AU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x8AU;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PWR_CTRL5, &tmp, 1);
    tmp = 0xEEU;
    ret += st7735_send_data(&pObj->Ctx, &tmp, 1);

    tmp = 0x0EU;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_VCOMH_VCOML_CTRL1, &tmp, 1);

    ret += st7735_write_reg(&pObj->Ctx,
        (pConfig->Panel == HannStar_Panel) ? ST7735_DISPLAY_INVERSION_ON : ST7735_DISPLAY_INVERSION_OFF,
        &tmp, 0);

    ret += st7735_write_reg(&pObj->Ctx, ST7735_COLOR_MODE, (uint8_t*)&ColorCoding, 1);

    const uint8_t pv_gamma[] = {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
                                0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
    tmp = pv_gamma[0];
    ret += st7735_write_reg(&pObj->Ctx, ST7735_PV_GAMMA_CTRL, &tmp, 1);
    for (uint8_t i = 1; i < 16; i++) {
        ret += st7735_send_data(&pObj->Ctx, &pv_gamma[i], 1);
    }

    const uint8_t nv_gamma[] = {0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                                0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
    tmp = nv_gamma[0];
    ret += st7735_write_reg(&pObj->Ctx, ST7735_NV_GAMMA_CTRL, &tmp, 1);
    for (uint8_t i = 1; i < 16; i++) {
        ret += st7735_send_data(&pObj->Ctx, &nv_gamma[i], 1);
    }

    tmp = 0x00U;
    ret += st7735_write_reg(&pObj->Ctx, ST7735_NORMAL_DISPLAY_OFF, &tmp, 1);
    ret += st7735_write_reg(&pObj->Ctx, ST7735_DISPLAY_ON, &tmp, 1);

    ret += ST7735_SetOrientation(pObj, pConfig);

    pObj->IsInitialized = (ret == ST7735_OK) ? 1 : 0;
    return (ret == ST7735_OK) ? ST7735_OK : ST7735_ERROR;
}

int32_t ST7735_DeInit(ST7735_Object_t *pObj) {
    (void)pObj;
    return ST7735_OK;
}

int32_t ST7735_ReadID(ST7735_Object_t *pObj, uint32_t *Id) {
    uint8_t tmp[3];

    if (st7735_read_reg(&pObj->Ctx, ST7735_READ_ID1, &tmp[0]) != ST7735_OK) return ST7735_ERROR;
    if (st7735_read_reg(&pObj->Ctx, ST7735_READ_ID2, &tmp[1]) != ST7735_OK) return ST7735_ERROR;
    if (st7735_read_reg(&pObj->Ctx, ST7735_READ_ID3, &tmp[2]) != ST7735_OK) return ST7735_ERROR;

    *Id = ((uint32_t)tmp[2]) | ((uint32_t)tmp[1] << 8) | ((uint32_t)tmp[0] << 16);
    return ST7735_OK;
}

int32_t ST7735_DisplayOn(ST7735_Object_t *pObj) {
    uint8_t tmp = 0;

    st7735_write_reg(&pObj->Ctx, ST7735_NORMAL_DISPLAY_OFF, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 10);
    st7735_write_reg(&pObj->Ctx, ST7735_DISPLAY_ON, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 10);

    tmp = ST7735_GetMADCTL(pObj, 1);
    st7735_write_reg(&pObj->Ctx, ST7735_MADCTL, &tmp, 1);

    return ST7735_OK;
}

int32_t ST7735_DisplayOff(ST7735_Object_t *pObj) {
    uint8_t tmp = 0;

    st7735_write_reg(&pObj->Ctx, ST7735_NORMAL_DISPLAY_OFF, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 10);
    st7735_write_reg(&pObj->Ctx, ST7735_DISPLAY_OFF, &tmp, 0);
    (void)ST7735_IO_Delay(pObj, 10);

    tmp = ST7735_GetMADCTL(pObj, 1);
    st7735_write_reg(&pObj->Ctx, ST7735_MADCTL, &tmp, 1);

    return ST7735_OK;
}

int32_t ST7735_SetBrightness(ST7735_Object_t *pObj, uint32_t Brightness) {
    (void)pObj;
    (void)Brightness;
    return ST7735_ERROR;
}

int32_t ST7735_GetBrightness(ST7735_Object_t *pObj, uint32_t *Brightness) {
    (void)pObj;
    (void)Brightness;
    return ST7735_ERROR;
}

int32_t ST7735_SetOrientation(ST7735_Object_t *pObj, ST7735_Ctx_t *pConfig) {
    if (!pObj || !pConfig) return ST7735_ERROR;

    pObj->Config.Orientation = pConfig->Orientation;
    pObj->Config.Panel = pConfig->Panel;
    pObj->Config.Type = pConfig->Type;

    if (pConfig->Orientation <= ST7735_ORIENTATION_PORTRAIT_ROT180) {
        if (pConfig->Type == ST7735_0_9_inch_screen) {
            pObj->Config.Width  = ST7735_0_9_WIDTH;
            pObj->Config.Height = ST7735_0_9_HEIGHT;
        } else {
            pObj->Config.Width  = ST7735_1_8_WIDTH;
            pObj->Config.Height = ST7735_1_8_HEIGHT;
        }
    } else {
        if (pConfig->Type == ST7735_0_9_inch_screen) {
            pObj->Config.Width  = ST7735_0_9_HEIGHT;
            pObj->Config.Height = ST7735_0_9_WIDTH;
        } else {
            pObj->Config.Width  = ST7735_1_8_HEIGHT;
            pObj->Config.Height = ST7735_1_8_WIDTH;
        }
    }

    ST7735_SetDisplayWindow(pObj, 0U, 0U, pObj->Config.Width, pObj->Config.Height);

    uint8_t tmp = ST7735_GetMADCTL(pObj, 1);
    st7735_write_reg(&pObj->Ctx, ST7735_MADCTL, &tmp, 1);

    return ST7735_OK;
}

int32_t ST7735_GetOrientation(ST7735_Object_t *pObj, uint32_t *Orientation) {
    *Orientation = pObj->Config.Orientation;
    return ST7735_OK;
}

int32_t ST7735_SetCursor(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos) {
    uint8_t tmp;
    uint32_t x_ofs, y_ofs;

    ST7735_GetPanelOffset(pObj, &x_ofs, &y_ofs);
    Xpos += x_ofs;
    Ypos += y_ofs;

    st7735_write_reg(&pObj->Ctx, ST7735_CASET, &tmp, 0);
    tmp = (uint8_t)(Xpos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Xpos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    st7735_write_reg(&pObj->Ctx, ST7735_RASET, &tmp, 0);
    tmp = (uint8_t)(Ypos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Ypos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    st7735_write_reg(&pObj->Ctx, ST7735_WRITE_RAM, &tmp, 0);

    return ST7735_OK;
}

int32_t ST7735_DrawBitmap(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp) {
    uint32_t index, width, height, y_pos;
    uint8_t pixel_val[2], tmp;
    uint8_t *pbmp;
    uint32_t counter = 0;

    index = (uint32_t)pBmp[10] + ((uint32_t)pBmp[11] << 8) + ((uint32_t)pBmp[12] << 16) + ((uint32_t)pBmp[13] << 24);
    width = (uint32_t)pBmp[18] + ((uint32_t)pBmp[19] << 8) + ((uint32_t)pBmp[20] << 16) + ((uint32_t)pBmp[21] << 24);
    height = (uint32_t)pBmp[22] + ((uint32_t)pBmp[23] << 8) + ((uint32_t)pBmp[24] << 16) + ((uint32_t)pBmp[25] << 24);
    index = (uint32_t)pBmp[2] + ((uint32_t)pBmp[3] << 8) + ((uint32_t)pBmp[4] << 16) + ((uint32_t)pBmp[5] << 24) - index;

    pbmp = pBmp + index;
    y_pos = pObj->Config.Height - Ypos - height;

    if (ST7735_SetDisplayWindow(pObj, Xpos, y_pos, width, height) != ST7735_OK) {
        return ST7735_ERROR;
    }

    tmp = ST7735_GetMADCTL(pObj, 0);
    if (st7735_write_reg(&pObj->Ctx, ST7735_MADCTL, &tmp, 1) != ST7735_OK) {
        return ST7735_ERROR;
    }

    if (ST7735_SetCursor(pObj, Xpos, y_pos) != ST7735_OK) {
        return ST7735_ERROR;
    }

    do {
        pixel_val[0] = *(pbmp + 1);
        pixel_val[1] = *(pbmp);
        if (st7735_send_data(&pObj->Ctx, pixel_val, 2U) != ST7735_OK) {
            return ST7735_ERROR;
        }
        counter += 2U;
        pbmp += 2;
    } while (counter < index);

    tmp = ST7735_GetMADCTL(pObj, 1);
    if (st7735_write_reg(&pObj->Ctx, ST7735_MADCTL, &tmp, 1) != ST7735_OK) {
        return ST7735_ERROR;
    }

    return ST7735_SetDisplayWindow(pObj, 0U, 0U, pObj->Config.Width, pObj->Config.Height);
}

int32_t ST7735_FillRGBRect(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height) {
    uint8_t pdata[640];
    uint8_t *rgb_data = pData;

    if ((Xpos + Width) > pObj->Config.Width || (Ypos + Height) > pObj->Config.Height) {
        return ST7735_ERROR;
    }

    for (uint32_t j = 0; j < Height; j++) {
        if (ST7735_SetCursor(pObj, Xpos, Ypos + j) != ST7735_OK) {
            return ST7735_ERROR;
        }

        for (uint32_t i = 0; i < Width; i++) {
            pdata[2U * i] = (uint8_t)(*(rgb_data));
            pdata[(2U * i) + 1U] = (uint8_t)(*(rgb_data + 1));
            rgb_data += 2;
        }

        if (st7735_send_data(&pObj->Ctx, pdata, 2U * Width) != ST7735_OK) {
            return ST7735_ERROR;
        }
    }

    return ST7735_OK;
}

int32_t ST7735_DrawHLine(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color) {
    uint8_t pdata[640];

    if ((Xpos + Length) > pObj->Config.Width) {
        return ST7735_ERROR;
    }

    if (ST7735_SetCursor(pObj, Xpos, Ypos) != ST7735_OK) {
        return ST7735_ERROR;
    }

    for (uint32_t i = 0; i < Length; i++) {
        pdata[2U * i] = (uint8_t)(Color >> 8);
        pdata[(2U * i) + 1U] = (uint8_t)(Color);
    }

    return st7735_send_data(&pObj->Ctx, pdata, 2U * Length);
}

int32_t ST7735_DrawVLine(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color) {
    if ((Ypos + Length) > pObj->Config.Height) {
        return ST7735_ERROR;
    }

    for (uint32_t counter = 0; counter < Length; counter++) {
        if (ST7735_SetPixel(pObj, Xpos, Ypos + counter, Color) != ST7735_OK) {
            return ST7735_ERROR;
        }
    }

    return ST7735_OK;
}

int32_t ST7735_FillRect(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color) {
    uint8_t tmp;
    uint32_t x_ofs, y_ofs;
    uint32_t total_pixels = Width * Height;
    uint32_t remaining = total_pixels;
    
    uint8_t color_msb = (uint8_t)(Color >> 8);
    uint8_t color_lsb = (uint8_t)(Color);

    uint32_t buf_size = (Width * 2) > 512 ? 512 : (Width * 2);
    uint8_t *pbuf = (uint8_t *)__builtin_alloca(buf_size);
    
    for (uint32_t i = 0; i < buf_size / 2; i++) {
        pbuf[2*i] = color_msb;
        pbuf[2*i + 1] = color_lsb;
    }

    ST7735_GetPanelOffset(pObj, &x_ofs, &y_ofs);
    Xpos += x_ofs;
    Ypos += y_ofs;

    st7735_write_reg(&pObj->Ctx, ST7735_CASET, &tmp, 0);
    tmp = (uint8_t)(Xpos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Xpos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Xpos + Width - 1U) >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Xpos + Width - 1U) & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    st7735_write_reg(&pObj->Ctx, ST7735_RASET, &tmp, 0);
    tmp = (uint8_t)(Ypos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Ypos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Ypos + Height - 1U) >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Ypos + Height - 1U) & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    st7735_write_reg(&pObj->Ctx, ST7735_WRITE_RAM, &tmp, 0);

    uint32_t pixels_per_chunk = buf_size / 2;
    while (remaining > 0) {
        uint32_t send_pixels = (remaining > pixels_per_chunk) ? pixels_per_chunk : remaining;
        st7735_send_data(&pObj->Ctx, pbuf, send_pixels * 2);
        remaining -= send_pixels;
    }

    return ST7735_OK;
}

int32_t ST7735_SetPixel(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Color) {
    uint16_t color = (uint16_t)((uint16_t)Color << 8) | (uint16_t)((uint16_t)(Color >> 8));

    if (Xpos >= pObj->Config.Width || Ypos >= pObj->Config.Height) {
        return ST7735_ERROR;
    }

    if (ST7735_SetCursor(pObj, Xpos, Ypos) != ST7735_OK) {
        return ST7735_ERROR;
    }

    return st7735_send_data(&pObj->Ctx, (uint8_t*)&color, 2);
}

int32_t ST7735_GetPixel(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t *Color) {
    uint8_t pixel_lsb, pixel_msb, tmp;

    ST7735_SetCursor(pObj, Xpos, Ypos);
    st7735_read_reg(&pObj->Ctx, ST7735_READ_RAM, &tmp);
    st7735_recv_data(&pObj->Ctx, &tmp, 1);
    st7735_recv_data(&pObj->Ctx, &pixel_lsb, 1);
    st7735_recv_data(&pObj->Ctx, &pixel_msb, 1);

    *Color = ((uint32_t)(pixel_lsb)) + ((uint32_t)(pixel_msb) << 8);
    return ST7735_OK;
}

int32_t ST7735_GetXSize(ST7735_Object_t *pObj, uint32_t *XSize) {
    *XSize = pObj->Config.Width;
    return ST7735_OK;
}

int32_t ST7735_GetYSize(ST7735_Object_t *pObj, uint32_t *YSize) {
    *YSize = pObj->Config.Height;
    return ST7735_OK;
}

static int32_t ST7735_SetDisplayWindow(ST7735_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height) {
    uint8_t tmp;
    uint32_t x_ofs, y_ofs;

    ST7735_GetPanelOffset(pObj, &x_ofs, &y_ofs);
    Xpos += x_ofs;
    Ypos += y_ofs;

    st7735_write_reg(&pObj->Ctx, ST7735_CASET, &tmp, 0);
    tmp = (uint8_t)(Xpos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Xpos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Xpos + Width - 1U) >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Xpos + Width - 1U) & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    st7735_write_reg(&pObj->Ctx, ST7735_RASET, &tmp, 0);
    tmp = (uint8_t)(Ypos >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)(Ypos & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Ypos + Height - 1U) >> 8U);
    st7735_send_data(&pObj->Ctx, &tmp, 1);
    tmp = (uint8_t)((Ypos + Height - 1U) & 0xFFU);
    st7735_send_data(&pObj->Ctx, &tmp, 1);

    return ST7735_OK;
}

static int32_t ST7735_ReadRegWrap(void *Handle, uint8_t Reg, uint8_t* pData) {
    ST7735_Object_t *pObj = (ST7735_Object_t *)Handle;
    return pObj->IO.ReadReg(Reg, pData);
}

static int32_t ST7735_WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint32_t Length) {
    ST7735_Object_t *pObj = (ST7735_Object_t *)Handle;
    return pObj->IO.WriteReg(Reg, pData, Length);
}

static int32_t ST7735_SendDataWrap(void *Handle, const uint8_t *pData, uint32_t Length) {
    ST7735_Object_t *pObj = (ST7735_Object_t *)Handle;
    return pObj->IO.SendData((uint8_t *)pData, Length);
}

static int32_t ST7735_RecvDataWrap(void *Handle, uint8_t *pData, uint32_t Length) {
    ST7735_Object_t *pObj = (ST7735_Object_t *)Handle;
    return pObj->IO.RecvData(pData, Length);
}

static int32_t ST7735_IO_Delay(ST7735_Object_t *pObj, uint32_t Delay) {
    uint32_t tickstart = pObj->IO.GetTick();
    while ((pObj->IO.GetTick() - tickstart) < Delay);
    return ST7735_OK;
}

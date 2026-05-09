# 128×64 OLED 驱动 (SH1106 / SSD1306)

[![Size](https://img.shields.io/badge/Size-1.3%22%20%7C%200.96%22-lightgrey)]()
[![Controller](https://img.shields.io/badge/Controller-SH1106%20%7C%20SSD1306-brightgreen)]()
[![Interface](https://img.shields.io/badge/Interface-I2C-blue)]()

基于函数指针接口解耦的 128×64 I2C OLED 驱动，同时支持 **1.3 寸 SH1106** 和 **0.96 寸 SSD1306** 控制器，支持绘图、字符和数值显示，可跨平台移植。

## 架构

```
┌─────────────────────────────┐
│   oled.c / oled.h           │  ◀── 纯逻辑层，移植时无需修改
│   绘图 / 字符 / 数值显示      │
├─────────────────────────────┤
│   OLED_IO_t                 │  ◀── 2 个函数指针的接口
│   write_cmd / write_data    │
├─────────────────────────────┤
│   用户代码 (端口适配层)       │  ◀── 移植时只需实现此处
│   I2C / SPI 发送函数         │
└─────────────────────────────┘
```

## 控制器选择

| 参数 | 1.3" 屏 | 0.96" 屏 |
|---|---|---|
| 控制器 | SH1106 | SSD1306 |
| 分辨率 | 128×64 | 128×64 |
| 初始化 | `OLED_InitEx(&io, OLED_CONTROLLER_SH1106)` | `OLED_Init(&io)` (默认) |
| 内部列数 | 132 | 128 |
| 列偏移 | +2 | 0 |

**`OLED_Init()` 默认初始化为 SSD1306（0.96 寸）**，这是最常用的型号。1.3 寸 SH1106 需用 `OLED_InitEx()` 指定 `OLED_CONTROLLER_SH1106`。

## 特性

- **双控制器兼容** — SH1106 (1.3") 和 SSD1306 (0.96")
- **帧缓冲架构** — 先写入缓冲区，一次性刷新屏幕，无闪烁
- **图形绘制** — 点、线、矩形、圆、填充
- **文本显示** — ASCII 字符 (5×7)，支持 1~4 倍缩放

## 文件说明

| 文件 | 说明 |
|---|---|
| `oled.h` | API 声明 + `OLED_IO_t` 接口定义 + 控制器类型枚举 |
| `oled_font.h` | 5×7 ASCII 字库 (95 个可打印字符) |
| `oled.c` | 驱动核心实现 |

## API 参考

### 基础控制

| 函数 | 说明 |
|---|---|
| `OLED_Init(OLED_IO_t *io)` | 初始化 (默认 SH1106) |
| `OLED_InitEx(OLED_IO_t *io, OLED_Controller_t ctrl)` | 初始化 (指定控制器) |
| `OLED_Display()` | 刷新缓冲区到屏幕 |
| `OLED_Clear()` | 清空缓冲区 |
| `OLED_DisplayOn()` | 开启显示 |
| `OLED_DisplayOff()` | 关闭显示 |

### 图形绘制

| 函数 | 说明 |
|---|---|
| `OLED_DrawPixel(x, y, color)` | 画点 |
| `OLED_DrawLine(x1, y1, x2, y2, color)` | 画线 (Bresenham) |
| `OLED_DrawRect(x, y, w, h, color)` | 画矩形框 |
| `OLED_FillRect(x, y, w, h, color)` | 填充矩形 |
| `OLED_DrawCircle(x0, y0, r, color)` | 画圆 |
| `OLED_FillCircle(x0, y0, r, color)` | 填充圆 |

### 文本显示

| 函数 | 说明 |
|---|---|
| `OLED_ShowChar(x, y, ch, size)` | 显示字符 (size: 1~4) |
| `OLED_ShowString(x, y, str, size)` | 显示字符串 |
| `OLED_ShowNum(x, y, num, len, size)` | 显示整数，右对齐 |
| `OLED_ShowFloat(x, y, num, intLen, decLen, size)` | 显示浮点数 |
| `OLED_SetCursor(x, y)` | 设置硬件光标 (直接写) |

### 注意事项

- 坐标范围：x 0~127, y 0~63
- `color` 参数：1 = 点亮, 0 = 熄灭
- `size` 参数：1 = 5×7 像素原始大小，2 = 放大一倍，以此类推
- 调用绘图/文本函数后必须调用 `OLED_Display()` 刷新屏幕
- 帧缓冲占用 128 × 64 / 8 = **1024 字节** RAM

## 在 STM32 HAL 中使用 (I2C)

### 1. CubeMX 配置

- PB6 → **I2C1_SCL**, PB7 → **I2C1_SDA**
- I2C1 Speed Mode: 400kHz (Fast Mode)
- RCC: HSE 外部晶振，系统时钟 72MHz

### 2. 实现 2 个回调函数

```c
/* main.c USER CODE BEGIN 0 */
#include <string.h>

static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 100);
}

static void OLED_WriteData(const uint8_t *data, uint16_t len)
{
    uint8_t buf[129];
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, len + 1, 200);
}

static OLED_IO_t oled_io = {
    .write_cmd = OLED_WriteCmd,
    .write_data = OLED_WriteData,
};
/* USER CODE END 0 */
```

> I2C 地址：**0x78** (8 位写地址) = **0x3C** (7 位地址)

### 3. 初始化

```c
/* 0.96 寸 SSD1306 (默认) */
OLED_Init(&oled_io);

/* 1.3 寸 SH1106 */
OLED_InitEx(&oled_io, OLED_CONTROLLER_SH1106);
```

### 完整 main 示例

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */
    OLED_Init(&oled_io);

    OLED_ShowString(0, 0, "Hello OLED!", 2);
    OLED_ShowNum(0, 24, 1234, 4, 1);
    OLED_ShowFloat(0, 40, 3.14f, 2, 2, 1);
    OLED_Display();
    /* USER CODE END 2 */

    while (1)
    {
    }
}
```

## 移植到其他平台

1. 复制 `oled.h`、`oled_font.h`、`oled.c` 到新项目
2. 实现 `OLED_IO_t` 的两个回调函数（支持 I2C 或 SPI）
3. 调用 `OLED_Init()` 或 `OLED_InitEx()` 初始化

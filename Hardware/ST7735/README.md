# ST7735 LCD 驱动

ST7735 是一款用于驱动 0.96 英寸 TFT LCD 屏幕的驱动芯片，支持 SPI 接口通信。

## 硬件接线

### 引脚连接

| STM32 引脚 | LCD 引脚 | 功能说明 |
| :--- | :--- | :--- |
| PE11 | CS | 片选信号（低电平有效） |
| PE13 | DC/RS | 数据/命令选择（高电平=数据，低电平=命令） |
| PE10 | LED | 背光控制（PWM） |
| SPI4_SCK (PE12) | SCK | SPI 时钟 |
| SPI4_MOSI (PE6) | SDA | SPI 数据输出 |

### 电源连接

| LCD 引脚 | 连接 |
| :--- | :--- |
| VCC | 3.3V |
| GND | GND |

## CubeMX 配置

### SPI4 配置

1. **模式**: Master (主机模式)
2. **方向**: 1-Line (单向传输)
3. **数据宽度**: 8 Bits
4. **时钟极性**: Low
5. **时钟相位**: 1 Edge
6. **NSS**: Software (软件控制)
7. **波特率预分频器**: 2 (60MHz)
8. **NSS Pulse Mode**: Disable

### TIM1 配置（背光控制）

1. **模式**: PWM Generation CH2
2. **通道 2N**: 启用互补输出
3. **自动重装载值**: 1000-1
4. **比较值**: 根据需要调整（控制亮度）
5. **OCNPolarity**: Low

### GPIO 配置

| 引脚 | 模式 | 上拉/下拉 | 速度 |
| :--- | :--- | :--- | :--- |
| PE11 | Output Push-Pull | No Pull | Very High |
| PE13 | Output Push-Pull | No Pull | Very High |
| PE10 | Alternate Function Push-Pull | No Pull | Very High |

## 驱动文件结构

```
Hardware/ST7735/
├── font.h          # 字体定义
├── lcd.c           # LCD 上层接口函数
├── lcd.h           # LCD 头文件
├── logo_128_160.c  # Logo 数据 (128x160)
├── logo_160_80.c   # Logo 数据 (160x80)
├── st7735.c        # ST7735 核心驱动
├── st7735.h        # ST7735 头文件
├── st7735_reg.c    # 寄存器操作函数
└── st7735_reg.h    # 寄存器定义
```

## 使用示例

### 初始化代码

```c
#include "lcd.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    MX_SPI4_Init();
    MX_TIM1_Init();
    MX_GPIO_Init();
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

    ST7735_Ctx_t ST7735Ctx;
    ST7735Ctx.Orientation = ST7735_ORIENTATION_LANDSCAPE_ROT180;
    ST7735Ctx.Panel = HannStar_Panel;
    ST7735Ctx.Type = ST7735_0_9_inch_screen;

    ST7735_RegisterBusIO(&st7735_pObj, &st7735_pIO);
    ST7735_LCD_Driver.Init(&st7735_pObj, ST7735_FORMAT_RBG565, &ST7735Ctx);
    ST7735_LCD_Driver.ReadID(&st7735_pObj, &st7735_id);

    LCD_SetBrightness(100);
    
    // 显示测试图案
    LCD_TestPattern();
    
    while (1) {
        // 主循环
    }
}
```

### 常用函数

```c
// 设置亮度 (0-1000)
LCD_SetBrightness(uint32_t Brightness);

// 显示字符串
LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);

// 填充矩形
ST7735_LCD_Driver.FillRect(&st7735_pObj, x, y, width, height, color);

// 绘制水平线
ST7735_LCD_Driver.DrawHLine(&st7735_pObj, x, y, length, color);

// 绘制垂直线
ST7735_LCD_Driver.DrawVLine(&st7735_pObj, x, y, length, color);

// 设置像素
ST7735_LCD_Driver.SetPixel(&st7735_pObj, x, y, color);

// 获取屏幕尺寸
ST7735_LCD_Driver.GetXSize(&st7735_pObj, &width);
ST7735_LCD_Driver.GetYSize(&st7735_pObj, &height);
```

### 显示方向

```c
// 支持的显示方向
ST7735_ORIENTATION_PORTRAIT         // 竖屏
ST7735_ORIENTATION_PORTRAIT_ROT180  // 竖屏旋转180度
ST7735_ORIENTATION_LANDSCAPE        // 横屏
ST7735_ORIENTATION_LANDSCAPE_ROT180 // 横屏旋转180度
```

### 颜色定义

```c
WHITE          0xFFFF
BLACK          0x0000
BLUE           0x001F
RED            0xF800
GREEN          0x07E0
YELLOW         0xFFE0
CYAN           0x7FFF
MAGENTA        0xF81F
```

## 屏幕规格

### 0.96 英寸屏幕 (ST7735_0_9_inch_screen)

- 分辨率: 80 x 160 (竖屏) / 160 x 80 (横屏)
- 颜色深度: 65K (16-bit RGB565)
- 接口: SPI (3线/4线)

## 注意事项

1. **SPI 速度**: 建议使用 60MHz 以获得最佳性能
2. **背光控制**: 使用 TIM1_CH2N 的 PWM 输出控制背光亮度
3. **屏幕校准**: 根据实际屏幕面板类型选择 `HannStar_Panel` 或 `BOE_Panel`
4. **电源**: LCD 需要稳定的 3.3V 电源供应

## 项目集成

在 MDK-ARM 项目中需要添加以下文件：

- `st7735.c`
- `st7735_reg.c`
- `lcd.c`

并确保 Include Path 包含 `Hardware/ST7735` 目录。
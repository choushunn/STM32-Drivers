# SH1106 OLED 驱动 (STM32F103C8T6)

基于 STM32 HAL 库的 0.96 寸 I2C OLED 驱动，支持 SH1106 / SSD1306 控制器。采用分层架构，驱动核心与硬件平台完全解耦，方便移植。

## 架构

```
┌─────────────────────────────┐
│      oled.h / oled.c        │  ◀── 纯逻辑层，零硬件依赖
│   绘图/字符/数值显示核心       │      移植时无需修改
├─────────────────────────────┤
│      OLED_IO_t 接口          │  ◀── 2个函数指针
│   write_cmd / write_data     │        
├─────────────────────────────┤
│   main.c (端口适配层)        │  ◀── 移植时只需重写此处
│   HAL_I2C_Master_Transmit    │
└─────────────────────────────┘
```

## 硬件接线

| OLED (SH1106) | STM32F103C8T6 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 (I2C1_SCL) |
| SDA | PB7 (I2C1_SDA) |

I2C 地址: **0x78** (7位地址 0x3C)

## DHT11 接线

| DHT11 | STM32F103C8T6 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DATA | **PB0** |

注意：DHT11 数据线到 VCC 需接 **4.7kΩ ~ 10kΩ 上拉电阻**。

## 驱动架构

所有硬件驱动均采用函数指针接口解耦：

### OLED (I2C)

```
OLED_IO_t {
    write_cmd(cmd);       // 发送命令
    write_data(buf, len); // 发送数据
}
```

### DHT11 (单总线)

```
DHT11_IO_t {
    set_output();   // 引脚设为推挽输出
    set_input();    // 引脚设为上拉输入
    write_low();    // 引脚输出低电平
    write_high();   // 引脚输出高电平
    read_pin();     // 读取引脚电平 (0/1)
    delay_us(us);   // 微秒延时
    delay_ms(ms);   // 毫秒延时
}
```

## 项目结构

```
F103C8T6-LCD/
├── Core/                      # CubeMX 生成的 HAL 代码
│   ├── Inc/
│   └── Src/                   # main.c, usart.c 等
├── Hardware/                  # 硬件驱动层 (跨平台)
│   ├── OLED/                  #   SH1106 OLED 驱动
│   │   ├── oled.h
│   │   ├── oled_font.h
│   │   └── oled.c
│   └── DHT11/                 #   DHT11 温湿度传感器驱动
│       ├── dht11.h
│       └── dht11.c
├── Drivers/
├── MDK-ARM/
└── README.md
```

## 文件说明

| 文件 | 内容 | 跨平台 |
|---|---|---|
| `Hardware/OLED/oled.h` | OLED API + `OLED_IO_t` 接口 | 通用 |
| `Hardware/OLED/oled_font.h` | 5×7 ASCII 字库 | 通用 |
| `Hardware/OLED/oled.c` | OLED 驱动核心 | 通用 |
| `Hardware/DHT11/dht11.h` | DHT11 API + `DHT11_IO_t` 接口 | 通用 |
| `Hardware/DHT11/dht11.c` | DHT11 单总线时序驱动 | 通用 |
| `Core/Src/main.c` | 端口适配层 + 示例代码 | 按平台修改 |

## 快速开始

### 1. 添加文件到工程

复制驱动文件到你的项目对应目录，然后在 Keil 中：

- **添加源文件**：右键项目分组 → Add Existing Files → 选择 `Hardware/OLED/oled.c`
- **添加头文件路径**：Project → Options → C/C++ → Include Paths → 添加 `.\Hardware\OLED`
- **头文件引用**：
  ```c
  #include "oled.h"                     // 如果在 Include Paths 中添加了路径
  // 或
  #include "../Hardware/OLED/oled.h"    // 使用相对路径
  ```

### 2. 实现 IO 接口

```c
#include "oled.h"
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
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, len + 1, 100);
}

static OLED_IO_t oled_io = {
    .write_cmd = OLED_WriteCmd,
    .write_data = OLED_WriteData,
};
```

### 3. 初始化与使用

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    OLED_Init(&oled_io);

    OLED_ShowString(0, 0, "Hello World!", 1);
    OLED_ShowString(0, 16, "STM32 SH1106", 2);
    OLED_Display();

    while (1)
    {
    }
}
```

## API 参考

### 基础控制

| 函数 | 说明 |
|---|---|
| `OLED_Init(OLED_IO_t *io)` | 初始化 OLED，传入 IO 接口 |
| `OLED_Display()` | 将缓冲区内容刷新到屏幕 |
| `OLED_Clear()` | 清空缓冲区 |
| `OLED_DisplayOn()` | 开启显示 |
| `OLED_DisplayOff()` | 关闭显示 |

### 图形绘制

| 函数 | 说明 |
|---|---|
| `OLED_DrawPixel(x, y, color)` | 画点 (color: 0/1) |
| `OLED_DrawLine(x1, y1, x2, y2, color)` | 画线 (Bresenham 算法) |
| `OLED_DrawRect(x, y, w, h, color)` | 画矩形框 |
| `OLED_FillRect(x, y, w, h, color)` | 填充矩形 |
| `OLED_DrawCircle(x0, y0, r, color)` | 画圆 |
| `OLED_FillCircle(x0, y0, r, color)` | 填充圆 |

### 文本显示

| 函数 | 说明 |
|---|---|
| `OLED_ShowChar(x, y, ch, size)` | 显示字符 (size: 1-4) |
| `OLED_ShowString(x, y, str, size)` | 显示字符串 |
| `OLED_ShowNum(x, y, num, len, size)` | 显示整数 (len: 位数) |
| `OLED_ShowFloat(x, y, num, intLen, decLen, size)` | 显示浮点数 |
| `OLED_SetCursor(x, y)` | 设置光标位置 |

### 注意事项

- 横坐标 x 范围: 0~127，纵坐标 y 范围: 0~63
- `OLED_ShowChar` 中 size=1 为 5×7 像素，size=2 为放大一倍 (10×14)
- 调用 `OLED_Show*` 系列函数后需调用 `OLED_Display()` 才能刷新到屏幕
- 图形绘制函数直接操作缓冲区，同样需要 `OLED_Display()` 刷新

## 移植到其他平台

`oled.h`、`oled_font.h`、`oled.c` 三文件是纯 C 代码，仅依赖 `<stdint.h>` 和 `<string.h>`，可移植到任何平台：

1. 复制三个文件到新项目
2. 在用户代码中实现 `OLED_IO_t` 的两个函数
3. 调用 `OLED_Init(&my_io)` 初始化

### SPI 接口示例

```c
static void My_WriteCmd(uint8_t cmd)
{
    // CS=0; DC=0; SPI发送cmd; CS=1
}

static void My_WriteData(const uint8_t *data, uint16_t len)
{
    // CS=0; DC=1; SPI发送len字节data; CS=1
}

OLED_IO_t io = {My_WriteCmd, My_WriteData};
OLED_Init(&io);
```

## 其他

本项目基于 STM32CubeMX 生成，使用 Keil MDK-ARM v5 工具链，STM32 HAL 库。

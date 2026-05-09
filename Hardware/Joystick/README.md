# Joystick 模拟摇杆驱动

[![ADC](https://img.shields.io/badge/ADC-12bit%20x2-blue)]()
[![Direction](https://img.shields.io/badge/Direction-5%20Way-brightgreen)]()
[![Interface](https://img.shields.io/badge/Interface-ADC%20%2B%20GPIO-orange)]()

基于函数指针接口解耦的双轴模拟摇杆驱动，适用于 PS2 手柄摇杆模块或类似的双轴按键摇杆，支持 XY 轴 ADC 采样、按键检测和方向判别，可跨平台移植。

## 架构

```
┌─────────────────────────────────────┐
│   joystick.c / joystick.h           │  ◀── 纯逻辑层，移植时无需修改
│   采样 / 方向判别 / 按键消抖          │
├─────────────────────────────────────┤
│   JOYSTICK_IO_t                     │  ◀── 3 个函数指针的接口
│   read_x / read_y / read_btn        │
├─────────────────────────────────────┤
│   用户代码 (硬件适配层)               │  ◀── 移植时只需实现此处
│   ADC 读取 / GPIO 读取               │
└─────────────────────────────────────┘
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `joystick.h` | API 声明 + `JOYSTICK_IO_t` 接口定义 + 方向枚举 + 数据结构 |
| `joystick.c` | 驱动核心实现（方向判别 + 死区处理） |

## API 参考

### 数据结构

```c
/* ADC 原始值 (12bit, 0~4095) + 按键状态 */
typedef struct {
    uint16_t x;        /* X轴 ADC 值 */
    uint16_t y;        /* Y轴 ADC 值 */
    uint8_t  btn;      /* 按键状态: 0=按下, 1=释放 */
} JOYSTICK_Data_t;

/* 方向枚举 */
typedef enum {
    JOYSTICK_DIR_CENTER  = 0,   /* 居中 */
    JOYSTICK_DIR_UP      = 1,   /* 上 */
    JOYSTICK_DIR_DOWN    = 2,   /* 下 */
    JOYSTICK_DIR_LEFT    = 3,   /* 左 */
    JOYSTICK_DIR_RIGHT   = 4,   /* 右 */
} JOYSTICK_Dir_t;
```

### IO 接口

`JOYSTICK_IO_t` 定义了 3 个回调函数，用户需根据硬件平台实现：

| 回调 | 返回值 | 说明 |
|---|---|---|
| `read_x` | `uint16_t` | 读取 X 轴 ADC 值 (0~4095) |
| `read_y` | `uint16_t` | 读取 Y 轴 ADC 值 (0~4095) |
| `read_btn` | `uint8_t` | 读取按键 (0=按下, 1=释放) |

### 核心函数

| 函数 | 说明 |
|---|---|
| `JOYSTICK_Init(JOYSTICK_IO_t *io)` | 注册 IO 回调 |
| `JOYSTICK_Read(JOYSTICK_Data_t *data)` | 读取当前摇杆数据 (x, y, btn) |
| `JOYSTICK_GetDir(uint16_t x, uint16_t y)` | 根据 x/y 值判别方向 |

### 错误码

| 宏 | 值 | 说明 |
|---|---|---|
| `JOYSTICK_OK` | 0 | 操作成功 |
| `JOYSTICK_ERR_NULL` | -1 | IO 回调未初始化 |

## 方向判别逻辑

- **死区阈值**: 300（中心点 2048 ± 300）
- 中心区 (`1748~2348`): 判为 `CENTER`
- X/Y 同时偏离时: 比较偏移量，取偏移大者为最终方向
- 仅单轴偏离: 直接按偏移方向判定

```
         UP
     ─────────
     │        │
LEFT │ CENTER │ RIGHT
     │        │
     ─────────
        DOWN
```

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

| 引脚 | 功能 | 配置 |
|---|---|---|
| PA0 (VRX) | ADC1_CH0 | 模拟输入, 单次转换 |
| PA1 (VRY) | ADC2_CH1 | 模拟输入, 单次转换 |
| PA2 (SW) | GPIO | 上拉输入 |

### 2. 实现 3 个回调函数

```c
/* main.c USER CODE BEGIN 0 */
#include "joystick.h"

static uint16_t JOYSTICK_ReadX(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    return HAL_ADC_GetValue(&hadc1);
}

static uint16_t JOYSTICK_ReadY(void)
{
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 10);
    return HAL_ADC_GetValue(&hadc2);
}

static uint8_t JOYSTICK_ReadBtn(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET) ? 0 : 1;
}

static JOYSTICK_IO_t joystick_io = {
    .read_x  = JOYSTICK_ReadX,
    .read_y  = JOYSTICK_ReadY,
    .read_btn = JOYSTICK_ReadBtn,
};
/* USER CODE END 0 */
```

### 3. 初始化与使用

```c
/* USER CODE BEGIN 2 */
JOYSTICK_Init(&joystick_io);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    JOYSTICK_Data_t data;

    if (JOYSTICK_Read(&data) == JOYSTICK_OK)
    {
        JOYSTICK_Dir_t dir = JOYSTICK_GetDir(data.x, data.y);

        /* data.x   — X轴 ADC 原始值 (0~4095) */
        /* data.y   — Y轴 ADC 原始值 (0~4095) */
        /* data.btn — 按键状态 (0=按下)       */
        /* dir      — 方向 (CENTER/UP/DOWN/LEFT/RIGHT) */
    }

    HAL_Delay(100);
}
/* USER CODE END WHILE */
```

## 移植到其他平台

1. 复制 `joystick.h`、`joystick.c` 到新项目
2. 实现 `JOYSTICK_IO_t` 的三个回调函数（ADC 读取 x2 + GPIO 读取 x1）
3. 调用 `JOYSTICK_Init()` 注册回调

## 设计细节

- **死区处理** — 中心区 ±300 阈值，避免摇杆回中时方向抖动
- **对角线仲裁** — 同时偏离时取偏移量大的一方，保证方向语义唯一
- **平台无关** — 通过函数指针解耦，不依赖任何 MCU 或 RTOS

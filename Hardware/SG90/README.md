# SG90 舵机驱动

[![PWM](https://img.shields.io/badge/PWM-50Hz%20Servo-blue)]()
[![Angle](https://img.shields.io/badge/Angle-0~180deg-green)]()
[![Interface](https://img.shields.io/badge/Interface-PWM%20%2B%20Timer-orange)]()

基于函数指针接口解耦的 SG90 微型舵机 PWM 驱动，支持 0~180° 角度控制和平滑移动功能，可跨平台移植。

## 架构

```
┌─────────────────────────────────────┐
│   sg90.c / sg90.h                   │  ◀── 纯逻辑层，移植时无需修改
│   角度→脉宽转换 / 平滑移动控制        │
├─────────────────────────────────────┤
│   SG90_IO_t                         │  ◀── 2 个函数指针的接口
│   set_pulse / get_ms                │
├─────────────────────────────────────┤
│   用户代码 (硬件适配层)               │  ◀── 移植时只需实现此处
│   PWM 输出 / 毫秒定时器              │
└─────────────────────────────────────┘
```

## 工作原理

SG90 舵机通过 PWM 信号控制角度：
- **PWM 频率**: 50Hz（周期 20ms）
- **脉宽范围**: 500µs ~ 2500µs
- **角度范围**: 0° ~ 180°

| 角度 | 脉宽 | 说明 |
|------|------|------|
| 0° | 500µs | 最小角度 |
| 90° | 1500µs | 中位 |
| 180° | 2500µs | 最大角度 |

角度到脉宽的线性转换公式：
```
pulse(µs) = 500 + angle × (2500 - 500) / 180
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `sg90.h` | API 声明 + `SG90_IO_t` 接口定义 + 错误码 + 脉宽常量 |
| `sg90.c` | 驱动核心实现（角度控制 + 平滑移动） |

## API 参考

### IO 接口

`SG90_IO_t` 定义了 2 个回调函数：

| 回调 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `set_pulse` | `uint16_t us` - 脉宽（µs） | 无 | 设置 PWM 脉冲宽度 |
| `get_ms` | 无 | `uint32_t` | 获取毫秒时间戳 |

### 核心函数

| 函数 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `SG90_Init(SG90_IO_t *io)` | IO 接口指针 | 无 | 初始化驱动，设置初始角度为 90° |
| `SG90_SetAngle(uint16_t angle)` | `angle` - 目标角度（0~180） | `int8_t` | 设置舵机角度 |
| `SG90_SetAngleSmooth(uint16_t target, uint16_t speed)` | `target` - 目标角度，`speed` - 步长 | `int8_t` | 平滑移动到目标角度 |

### 错误码

| 宏 | 值 | 说明 |
|---|---|---|
| `SG90_OK` | 0 | 操作成功 |
| `SG90_ERR_NULL` | -1 | IO 回调未初始化 |
| `SG90_ERR_RANGE` | -2 | 角度超出范围（>180°） |

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

| 配置项 | 设置 |
|---|---|
| 定时器 | TIM3 |
| 通道 | CH1 (PA6) |
| 模式 | PWM Generation CH1 |
| 预分频器 | 71 (72MHz / 72 = 1MHz) |
| 自动重装载值 | 19999 (1MHz / 20000 = 50Hz) |
| GPIO | PA6 设为 Alternate Function Push-Pull |

### 2. 实现回调函数

```c
/* main.c USER CODE BEGIN 0 */
#include "sg90.h"

static void SG90_SetPulse(uint16_t us)
{
    /* 计算比较值: us × 1MHz / 1000000 = us */
    /* TIM3 定时器时钟 1MHz，1µs = 1 tick */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, us);
}

static uint32_t SG90_GetMs(void)
{
    return HAL_GetTick();
}

static SG90_IO_t sg90_io = {
    .set_pulse = SG90_SetPulse,
    .get_ms    = SG90_GetMs,
};
/* USER CODE END 0 */
```

### 3. 初始化

```c
/* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  /* 启动 PWM */
SG90_Init(&sg90_io);                        /* 初始化舵机驱动 */
/* USER CODE END 2 */
```

### 4. 使用示例

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    /* 直接设置角度 */
    SG90_SetAngle(0);    /* 转到 0° */
    HAL_Delay(1000);
    
    SG90_SetAngle(90);   /* 转到 90° */
    HAL_Delay(1000);
    
    SG90_SetAngle(180);  /* 转到 180° */
    HAL_Delay(1000);
    
    /* 平滑移动（每步 5°，约 36 步从 0°→180°） */
    SG90_SetAngleSmooth(0, 5);    /* 从当前位置平滑转到 0° */
    HAL_Delay(500);
    
    SG90_SetAngleSmooth(180, 5);  /* 平滑转到 180° */
    HAL_Delay(500);
}
/* USER CODE END WHILE */
```

## 平滑移动说明

`SG90_SetAngleSmooth()` 函数实现匀速平滑移动：

| 参数 | 说明 |
|---|---|
| `target_angle` | 目标角度（0~180） |
| `speed` | 每 20ms 步进的角度（默认 1，越大越快） |

- 内部使用 20ms 间隔逐步调整角度
- 若未提供 `get_ms` 回调，自动降级为直接设置角度
- `speed` 参数小于 1 时自动修正为 1

## 硬件接线

| SG90 引脚 | STM32 引脚 | 说明 |
|---|---|---|
| VCC | 5V | 舵机电源（需外部供电，电流较大） |
| GND | GND | 接地 |
| SIGNAL | PA6 | PWM 控制信号 |

> **注意**: SG90 工作电流较大（最大约 200mA），建议使用外部电源或单独的 5V 电源模块，避免从开发板取电。

## 移植到其他平台

1. 复制 `sg90.h`、`sg90.c` 到新项目
2. 实现 `SG90_IO_t` 的两个回调函数
3. 调用 `SG90_Init()` 初始化驱动

| 回调 | 实现说明 |
|---|---|
| `set_pulse(us)` | 设置 PWM 比较值，产生指定宽度的脉冲 |
| `get_ms()` | 返回单调递增的毫秒时间戳 |

## 设计细节

- **防越界保护** — 角度超出 0~180° 范围时自动截断并返回错误码
- **平台无关** — 通过函数指针解耦，不依赖任何 MCU 或 RTOS
- **平滑移动** — 支持匀速步进控制，避免机械冲击
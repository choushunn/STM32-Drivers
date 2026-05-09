# EC11 旋转编码器驱动

[![Type](https://img.shields.io/badge/Type-EC11%20Encoder-brightgreen)]()
[![Interface](https://img.shields.io/badge/Interface-GPIO%20EXTI-orange)]()
[![Features](https://img.shields.io/badge/Features-Rotation%20%2B%20Button-blue)]()

基于函数指针接口解耦的 EC11 旋转编码器驱动，采用 **EXTI 硬件中断** 实时捕获正交脉冲，支持旋转方向判别、脉冲计数、按键检测（短按/长按），可跨平台移植。

## 架构

```
┌──────────────────────────────────────────────┐
│   encoder.c / encoder.h                      │  ◀── 纯逻辑层，移植时无需修改
│   正交解码 + 按键消抖 + 脉冲计数              │
├──────────────────────────────────────────────┤
│   ENCODER_IO_t                              │  ◀── 4 个函数指针的接口
│   read_a / read_b / read_btn / get_ms       │
├──────────────────────────────────────────────┤
│   用户代码 (硬件适配层)                       │  ◀── 移植时只需实现此处
│   中断配置 / GPIO 读取 / HAL_GetTick        │
└──────────────────────────────────────────────┘
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `encoder.h` | API 声明 + `ENCODER_IO_t` 接口定义 + 方向枚举 |
| `encoder.c` | 驱动核心实现（正交解码 + 按键消抖 + 脉冲计数） |

## 硬件连接

| EC11 引脚 | MCU 引脚 | 功能 | 配置 |
|---|---|---|---|
| A (CLK) | PA0 | 正交编码 A 相 | EXTI 双边沿中断，上拉输入 |
| B (DT) | PA1 | 正交编码 B 相 | EXTI 双边沿中断，上拉输入 |
| SW (按键) | PA2 | 内部按键 | 普通输入，上拉 |
| + | 3.3V | 电源 | - |
| GND | GND | 地 | - |

> ⚠️ **中断优先级**：EXTI0/EXTI1 需优先级配置合理（本项目设为 0 和 1），确保旋转时不被其他中断长时间阻塞。

## 实现方式

本驱动采用 **EXTI 硬件中断 + 正交状态表** 的实时解码方案：

```
PA0 边沿 ──▶ EXTI0_ISR
                    │
PA1 边沿 ──▶ EXTI1_ISR
                    │
      ┌─────────────┘
      ▼
  ENCODER_ExtiHandler(pin_a, pin_b)
      │
      ▼
  正交状态表查表 ──▶ 计数 ±1 + 方向标记
```

### 正交解码原理

将 AB 两相电平组合成 2bit 状态值 `(A<<1)|B`，每次中断读取当前状态并与上一状态查表：

```c
static const int8_t encoder_state_table[16] = {
     0,  1, -1,  0,    // 00→00, 00→01, 00→10, 00→11
    -1,  0,  0,  1,    // 01→00, 01→01, 01→10, 01→11
     1,  0,  0, -1,    // 10→00, 10→01, 10→10, 10→11
     0, -1,  1,  0,    // 11→00, 11→01, 11→10, 11→11
};
```

- **`+1`** = 顺时针 (CW)
- **`-1`** = 逆时针 (CCW)
- **`0`** = 无效跳变（抖动或非法状态）

### 为什么用中断而非轮询

| 方式 | 延迟 | 脉冲丢失 | CPU 占用 |
|------|------|---------|---------|
| 轮询 (HAL_Delay 50ms) | ~50ms | 严重（EC11 快速旋转 <10ms 脉冲） | 高 |
| EXTI 中断 | <1μs | 零丢失 | 极低（仅边沿触发时执行） |

## API 参考

### IO 接口

`ENCODER_IO_t` 定义了 4 个回调函数，用户需根据硬件平台实现：

| 回调 | 返回值 | 说明 |
|---|---|---|
| `read_a` | `uint8_t` | 读取编码器 A 相电平 (0/1) |
| `read_b` | `uint8_t` | 读取编码器 B 相电平 (0/1) |
| `read_btn` | `uint8_t` | 读取按键 (0=按下, 1=释放) |
| `get_ms` | `uint32_t` | 获取系统毫秒时间戳 |

### 核心函数

| 函数 | 说明 |
|---|---|
| `ENCODER_Init(ENCODER_IO_t *io)` | 注册 IO 回调，记录初始 AB 状态 |
| `ENCODER_ExtiHandler(pin_a, pin_b)` | **中断中调用**，传入当前 AB 电平，执行正交解码 |
| `ENCODER_Process(void)` | **主循环中调用**，处理按键消抖和长短按判定 |
| `ENCODER_GetCount(void)` | 获取当前脉冲计数值 |
| `ENCODER_SetCount(int32_t count)` | 设置脉冲计数值（清零或预设） |
| `ENCODER_GetDirection(void)` | 获取最后旋转方向（一次性读取，读后清除） |
| `ENCODER_GetBtnPress(void)` | 获取短按标志（一次性读取） |
| `ENCODER_GetBtnLongPress(void)` | 获取长按标志（一次性读取，≥800ms） |

### 方向枚举

```c
typedef enum {
    ENCODER_DIR_NONE  = 0,   /* 无旋转 */
    ENCODER_DIR_CW    = 1,   /* 顺时针 (ClockWise) */
    ENCODER_DIR_CCW   = -1,  /* 逆时针 (CounterClockWise) */
} ENCODER_Dir_t;
```

### 错误码

| 宏 | 值 | 说明 |
|---|---|---|
| `ENCODER_OK` | 0 | 操作成功 |
| `ENCODER_ERR_NULL` | -1 | IO 回调未初始化 |

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

| 引脚 | 功能 | 配置 |
|---|---|---|
| PA0 | EXTI0 | GPIO_INPUT, 上拉 (代码中重配为 IT_RISING_FALLING) |
| PA1 | EXTI1 | GPIO_INPUT, 上拉 (代码中重配为 IT_RISING_FALLING) |
| PA2 | GPIO | 上拉输入 |

在 NVIC 中使能 `EXTI0_IRQn`、`EXTI1_IRQn`。

### 2. 实现 IO 回调

```c
/* main.c USER CODE BEGIN 0 */
#include "encoder.h"

static uint8_t ENCODER_ReadA(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint8_t ENCODER_ReadB(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint8_t ENCODER_ReadBtn(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET) ? 0 : 1;
}

static uint32_t ENCODER_GetMs(void)
{
    return HAL_GetTick();
}

static ENCODER_IO_t encoder_io = {
    .read_a   = ENCODER_ReadA,
    .read_b   = ENCODER_ReadB,
    .read_btn = ENCODER_ReadBtn,
    .get_ms   = ENCODER_GetMs,
};
/* USER CODE END 0 */
```

### 3. 初始化 + 配置 EXTI

```c
/* USER CODE BEGIN 2 */
{
    GPIO_InitTypeDef gpio = {0};

    /* PA0 / PA1: EXTI 双边沿触发 */
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA2: 普通上拉输入 */
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 使能 EXTI1 NVIC */
    HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    ENCODER_Init(&encoder_io);
}
/* USER CODE END 2 */
```

### 4. 中断服务函数

```c
/* USER CODE BEGIN Includes */
#include "encoder.h"
/* USER CODE END Includes */

void EXTI0_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        ENCODER_ExtiHandler(
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0),
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        ENCODER_ExtiHandler(
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0),
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
```

### 5. 主循环

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    ENCODER_Process();          /* 按键消抖处理 */

    int32_t count = ENCODER_GetCount();
    if (count > old_count)
        /* 顺时针旋转 */;
    else if (count < old_count)
        /* 逆时针旋转 */;
    old_count = count;

    if (ENCODER_GetBtnPress())
        /* 短按 */;

    if (ENCODER_GetBtnLongPress())
        /* 长按 */;
}
/* USER CODE END WHILE */
```

## 按键消抖时序

```
按键按下 ──┐
           │  20ms 消抖
           ▼  ┌────────────┐
读取电平 0 ──▶│ 确认按下    │── 记录按下时间
              └────────────┘
按键释放 ──┐
           │  20ms 消抖
           ▼  ┌──────────────────────────────┐
读取电平 1 ──▶│ 计算保持时间                   │
              │ t ≥ 800ms → 长按标志           │
              │ t < 800ms → 短按标志           │
              └──────────────────────────────┘
```

## 移植到其他平台

1. 复制 `encoder.h`、`encoder.c` 到新项目
2. 在 `stm32f1xx_it.c` 中添加 `#include "encoder.h"`
3. 实现 `ENCODER_IO_t` 的四个回调函数
4. 在 PA0/PA1 引脚配置 EXTI 双边沿触发中断
5. 中断 ISR 中调用 `ENCODER_ExtiHandler()` 传入 AB 电平
6. 主循环周期调用 `ENCODER_Process()` 处理按键

## 设计细节

- **EC11 正交波形** — AB 两相相位差 90°，旋转一周约 20 个脉冲（具体取决于型号）
- **状态表解码** — 4 状态 × 4 状态 = 16 项查表，单次 ISR 执行 < 1μs
- **中断零丢失** — 任何边沿变化立即捕获，EC11 快速旋转无丢失
- **按键消抖** — 20ms 硬件消抖，区分短按（<800ms）和长按（≥800ms）
- **脉冲叠加** — `encoder_count` 为有符号 32 位整数，正负双向累加
- **平台无关** — 通过函数指针解耦，不依赖任何 MCU 或 RTOS

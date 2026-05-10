# TEMT6000 环境光传感器驱动

[![Type](https://img.shields.io/badge/Type-Analog%20Light%20Sensor-yellow)]()
[![Interface](https://img.shields.io/badge/Interface-ADC-blue)]()
[![Range](https://img.shields.io/badge/Range-0~1000%2B%20lux-lightgrey)]()

基于函数指针接口解耦的 TEMT6000 环境光传感器驱动。TEMT6000 是一款模拟输出的光电晶体管，输出电流与环境光照强度成线性关系，通过 ADC 采样和换算得到 Lux 值。

## 架构

```
┌──────────────────────────────────────────────┐
│   temt6000.c / temt6000.h                   │  ◀── 纯逻辑层，移植时无需修改
│   ADC 采样 + Lux 换算                        │
├──────────────────────────────────────────────┤
│   TEMT6000_IO_t                             │  ◀── 1 个函数指针的接口
│   read_adc                                  │
├──────────────────────────────────────────────┤
│   用户代码 (硬件适配层)                       │  ◀── 移植时只需实现此处
│   ADC 初始化 + 读取                          │
└──────────────────────────────────────────────┘
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `temt6000.h` | API 声明 + `TEMT6000_IO_t` 接口定义 |
| `temt6000.c` | 驱动核心实现（读取 + 换算） |

## 硬件连接

| TEMT6000 | MCU 引脚 | 说明 |
|---|---|---|
| OUT | PA3 | ADC1_CH3 模拟输入 |
| VCC | 3.3V | 电源 |
| GND | GND | 地 |

> TEMT6000 模块通常带有 10kΩ 下拉电阻，输出电压 = 集电极电流 × 10kΩ。

## 工作原理

TEMT6000 是一款 NPN 光电晶体管，集电极电流与光照强度成正比：

```
光照 → 光电流 I = k × LUX  (k ≈ 6.5µA / 100 lux)
     → Vout = I × 10kΩ = k × LUX × 10,000
     → LUX = Vout / 0.065V × 100
     → LUX ≈ ADC_value × 3.3 / 4096 / 0.65 × 100 ≈ ADC_value × 1.24
```

## API 参考

### IO 接口

`TEMT6000_IO_t` 定义了 1 个回调函数：

| 回调 | 返回值 | 说明 |
|---|---|---|
| `read_adc` | `uint16_t` | 读取 ADC 原始值 (12bit, 0~4095) |

### 核心函数

| 函数 | 说明 |
|---|---|
| `TEMT6000_Init(io)` | 注册 IO 回调 |
| `TEMT6000_ReadRaw(void)` | 读取 ADC 原始值 |
| `TEMT6000_CalcLux(raw)` | 将 ADC 原始值换算为 Lux 值 |

## 在 STM32 HAL 中使用

### 1. ADC 配置

| 外设 | 引脚 | 配置 |
|---|---|---|
| ADC1 | PA3 (CH3) | 单次转换, 软件触发, 12bit 右对齐 |

### 2. ADC 初始化代码

```c
ADC_HandleTypeDef hadc1;

/* 在 SystemClock_Config 中添加 ADC 时钟 */
RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

/* HAL_ADC_MspInit 回调 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &gpio);
}

/* 初始化 ADC1_CH3 */
hadc1.Instance = ADC1;
hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
hadc1.Init.ContinuousConvMode = DISABLE;
hadc1.Init.DiscontinuousConvMode = DISABLE;
hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
hadc1.Init.NbrOfConversion = 1;
HAL_ADC_Init(&hadc1);

ADC_ChannelConfTypeDef sConfig = {0};
sConfig.Channel = ADC_CHANNEL_3;
sConfig.Rank = ADC_REGULAR_RANK_1;
sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);
```

### 3. 实现 IO 回调

```c
/* USER CODE BEGIN Includes */
#include "temt6000.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */
static uint16_t TEMT6000_ReadADC(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    return HAL_ADC_GetValue(&hadc1);
}

static TEMT6000_IO_t temt_io = {
    .read_adc = TEMT6000_ReadADC,
};
/* USER CODE END 0 */
```

### 4. 初始化与使用

```c
/* USER CODE BEGIN 2 */
TEMT6000_Init(&temt_io);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    uint16_t raw = TEMT6000_ReadRaw();
    float lux = TEMT6000_CalcLux(raw);

    printf("Light: %u raw, %.1f lux\r\n", raw, lux);

    HAL_Delay(200);
}
/* USER CODE END WHILE */
```

## OLED 显示布局

```
        == TEMT6000 ==
Raw:   2048
Lux:  1012.3
 ┌────────────────────────────┐
 ████████████████████          │
 └────────────────────────────┘
        LIGHT SENSOR
```

- Raw: ADC 原始值 (0~4095)
- Lux: 换算后的光照强度 (0~1000+ lux)
- 进度条: 0~1000 lux 满量程

## 移植到其他平台

1. 复制 `temt6000.h`、`temt6000.c` 到新项目
2. 实现 `TEMT6000_IO_t` 的一个回调函数（ADC 读取）
3. 配置 ADC 引脚为模拟输入
4. 调用 `TEMT6000_Init()` → 周期调用 `TEMT6000_ReadRaw()`

## 设计细节

- **最小接口** — 仅需 1 个函数指针（read_adc），是项目中最简单的驱动
- **线性换算** — Lux = Raw × 3.3 / 4096 / 0.65 × 100 ≈ Raw × 1.24
- **平台无关** — 通过函数指针解耦，不依赖任何 MCU 或 RTOS

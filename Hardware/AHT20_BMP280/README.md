# AHT20+BMP280 温湿度气压计驱动

[![Interface](https://img.shields.io/badge/Interface-I2C-blue)]()
[![Sensor1](https://img.shields.io/badge/Sensor1-AHT20%20(T%2BH)-brightgreen)]()
[![Sensor2](https://img.shields.io/badge/Sensor2-BMP280%20(P%2BT)-orange)]()

基于函数指针接口解耦的 AHT20 + BMP280 双传感器组合驱动。AHT20 提供温湿度，BMP280 提供气压和温度，共用同一 I2C 总线。

## 架构

```
┌──────────────────────────────────────────────┐
│   aht20_bmp280.c / aht20_bmp280.h           │  ◀── 纯逻辑层，移植时无需修改
│   AHT20 温湿度 + BMP280 气压                 │
├──────────────────────────────────────────────┤
│   AHT20_BMP280_IO_t                         │  ◀── 3 个函数指针的接口
│   read(dev, reg, buf, len)                  │
│   write(dev, reg, buf, len)                 │
│   delay_ms(ms)                              │
├──────────────────────────────────────────────┤
│   用户代码 (硬件适配层)                       │  ◀── 移植时只需实现此处
│   I2C 读写 / HAL_Delay                      │
└──────────────────────────────────────────────┘
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `aht20_bmp280.h` | API 声明 + `AHT20_BMP280_IO_t` 接口定义 + 错误码枚举 |
| `aht20_bmp280.c` | 驱动核心实现（AHT20 初始化/触发/读取 + BMP280 校准/补偿/读取） |

## 硬件连接

| AHT20+BMP280 模块 | MCU 引脚 | 功能 |
|---|---|---|
| SCL | PB10 | I2C2_SCL |
| SDA | PB11 | I2C2_SDA |
| VCC | 3.3V | 电源 |
| GND | GND | 地 |

> 两个传感器在同一 I2C 总线上，AHT20 地址 `0x38`，BMP280 地址 `0x76` 或 `0x77`。

## 错误码系统

每个错误码对应一个明确的失败原因，便于调试：

| 错误码 | 值 | 含义 |
|---|---|---|
| `AHT20_BMP280_OK` | 0 | 操作成功 |
| `AHT20_BMP280_ERR_NULL` | -1 | IO 回调未初始化 |
| `AHT20_BMP280_ERR_AHT20_INIT` | -2 | AHT20 初始化命令发送失败 |
| `AHT20_BMP280_ERR_AHT20_TRIG` | -3 | AHT20 触发测量命令发送失败 |
| `AHT20_BMP280_ERR_AHT20_READ` | -4 | AHT20 读取测量结果失败 |
| `AHT20_BMP280_ERR_BMP280_ADDR` | -5 | BMP280 地址无应答（0x76 / 0x77 均无响应） |
| `AHT20_BMP280_ERR_BMP280_ID` | -6 | BMP280 芯片 ID 校验失败（期望 0x58） |
| `AHT20_BMP280_ERR_BMP280_CALIB` | -7 | BMP280 校准系数读取失败 |
| `AHT20_BMP280_ERR_BMP280_CFG` | -8 | BMP280 模式配置写入失败 |
| `AHT20_BMP280_ERR_BMP280_READ` | -9 | BMP280 测量值读取失败 |

通过 `AHT20_BMP280_ErrStr(err)` 可将错误码转换为可读字符串，如 `AHT20_INIT`、`BMP280_ID`、`BMP280_ADDR` 等。

## API 参考

### IO 接口

`AHT20_BMP280_IO_t` 定义了 3 个回调函数：

| 回调 | 参数 | 说明 |
|---|---|---|
| `read(dev_addr, reg, data, len)` | dev=I2C地址, reg=寄存器, data=接收缓冲, len=长度 | 从设备指定寄存器读若干字节 |
| `write(dev_addr, reg, data, len)` | dev=I2C地址, reg=寄存器, data=发送数据, len=长度 | 向设备指定寄存器写若干字节 |
| `delay_ms(ms)` | ms=毫秒数 | 阻塞延时 |

> `reg=0xFF` 时视为无寄存器地址的命令帧，直接发送裸数据（AHT20 协议需要）。

### 数据结构

```c
typedef struct {
    float temp;            /* 温度 (°C) */
    float humidity;        /* 湿度 (%RH) */
    float pressure;        /* 气压 (hPa) */
    uint8_t aht20_ok;      /* AHT20 读取成功标志 */
    uint8_t bmp280_ok;     /* BMP280 读取成功标志 */
    int8_t  err_aht20;     /* AHT20 错误码 */
    int8_t  err_bmp280;    /* BMP280 错误码 */
} AHT20_BMP280_Data_t;
```

### 核心函数

| 函数 | 说明 |
|---|---|
| `AHT20_BMP280_Init(io)` | 初始化 AHT20 + 探测并初始化 BMP280（自动尝试 0x76 和 0x77） |
| `AHT20_BMP280_Read(data)` | 读取 AHT20 温湿度 + BMP280 气压温度 |
| `AHT20_BMP280_ErrStr(err)` | 将错误码转换为可读字符串 |

### 数据流

```
AHT20: 发送触发命令(0xAC 0x33 0x00)
       → 等待 80ms 转换
       → 读 6 字节 → 解析温度/湿度

BMP280: 探测地址(0x76/0x77)
        → 读芯片 ID (0xD0) 校验 0x58
        → 读校准系数 (0x88~0xA1, 24 字节)
        → 配置模式 (0xF4=0x27, 0xF5=0xA0)
        → 周期性读 (0xF7~0xFC) 6 字节 → 补偿计算
```

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

| 外设 | 引脚 | 配置 |
|---|---|---|
| I2C2 | PB10(SCL), PB11(SDA) | 100kHz 标准模式 |
| USART1 | PA9(TX), PA10(RX) | 115200-8N1 (调试输出) |

### 2. 实现 IO 回调

```c
/* USER CODE BEGIN Includes */
#include "aht20_bmp280.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */
static int8_t SENSOR_Read(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (reg != 0xFF)
    {
        if (HAL_I2C_Mem_Read(&hi2c2, dev_addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_AHT20_READ;
    }
    else
    {
        if (HAL_I2C_Master_Receive(&hi2c2, dev_addr << 1, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_AHT20_READ;
    }
    return AHT20_BMP280_OK;
}

static int8_t SENSOR_Write(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (reg != 0xFF)
    {
        uint8_t buf[9];
        buf[0] = reg;
        for (uint16_t i = 0; i < len; i++)
            buf[1 + i] = data[i];
        if (HAL_I2C_Master_Transmit(&hi2c2, dev_addr << 1, buf, len + 1, 100) != HAL_OK)
            return AHT20_BMP280_ERR_AHT20_READ;
    }
    else
    {
        if (HAL_I2C_Master_Transmit(&hi2c2, dev_addr << 1, data, len, 100) != HAL_OK)
            return AHT20_BMP280_ERR_AHT20_READ;
    }
    return AHT20_BMP280_OK;
}

static void SENSOR_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

static AHT20_BMP280_IO_t sensor_io = {
    .read = SENSOR_Read,
    .write = SENSOR_Write,
    .delay_ms = SENSOR_Delay,
};
/* USER CODE END 0 */
```

### 3. 初始化

```c
/* USER CODE BEGIN 2 */
MX_I2C2_Init();
AHT20_BMP280_Init(&sensor_io);
/* USER CODE END 2 */
```

### 4. 主循环

```c
/* USER CODE BEGIN WHILE */
uint32_t last_tick = 0;
while (1)
{
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    if (HAL_GetTick() - last_tick >= 1000)
    {
        AHT20_BMP280_Data_t data;
        last_tick = HAL_GetTick();
        AHT20_BMP280_Read(&data);

        if (data.aht20_ok)
        {
            printf("AHT20 T:%.1fC H:%.1f%%\r\n", data.temp, data.humidity);
        }
        else
        {
            printf("AHT20 ERR: %s\r\n", AHT20_BMP280_ErrStr(data.err_aht20));
        }

        if (data.bmp280_ok)
        {
            printf("BMP280 P:%.1fhPa T:%.1fC\r\n", data.pressure, data.temp);
        }
        else
        {
            printf("BMP280 ERR: %s\r\n", AHT20_BMP280_ErrStr(data.err_bmp280));
        }
    }
}
/* USER CODE END 3 */
```

## AHT20 协议说明

```
主机发送:  0xBE 0x08 0x00           ← 初始化 (仅上电首次)
主机发送:  0xAC 0x33 0x00           ← 触发测量
等待 80ms
从机返回:  status H1 H2 H3 T1 T2    ← 6 字节数据

湿度 = (H1:H2:H3) / 2^20 × 100%    (20 位)
温度 = (T1:T2) / 2^20 × 200 - 50   (20 位)
```

## BMP280 协议说明

```
读 ID:     0xD0 → 0x58              ← 校验芯片存在
读校准:    0x88~0x9F (24 字节)       ← 温度+气压补偿系数
配置:      0xF4 = 0x27 (普通模式)
           0xF5 = 0xA0 (1s 滤波)
读数据:    0xF7~0xFC (6 字节)
           P = P_MSB:P_LSB:P_XLSB   (20 位)
           T = T_MSB:T_LSB:T_XLSB   (20 位)

温度补偿后 = (T_raw - T1) × T2 / 2^10 + ...  (详见补偿公式)
气压补偿后 = 复杂多项式，依赖温度 fine 值
```

## 调试技巧

| 现象 | 可能原因 |
|---|---|
| AHT20 正常，BMP280 显示 `BMP280_ADDR` | BMP280 不在 0x76 或 0x77，检查模块供电或接线 |
| BMP280 显示 `BMP280_ID` | 地址能通信但不是 BMP280（读 ID 不是 0x58），检查模块型号 |
| AHT20 显示 `AHT20_TRIG` | I2C 通信失败，检查上拉电阻或接线 |
| AHT20 显示 `AHT20_READ` | 触发成功但读取超时，检查 `delay_ms(80)` 是否足够 |

## 移植到其他平台

1. 复制 `aht20_bmp280.h`、`aht20_bmp280.c` 到新项目
2. 实现 `AHT20_BMP280_IO_t` 的三个回调函数（I2C 读写 + 延时）
3. 调用 `AHT20_BMP280_Init()` → 周期调用 `AHT20_BMP280_Read()`

## 设计细节

- **双传感器融合** — AHT20 测湿度和温度，BMP280 测气压和温度，BMP280 温度精度更高（补偿公式）
- **自动地址探测** — 初始化时自动尝试 BMP280 的 0x76 和 0x77 两个地址
- **细粒度错误码** — 每个失败点都有唯一错误码，配合 `AHT20_BMP280_ErrStr()` 快速定位
- **BMP280 校准补偿** — 完整实现温度/气压补偿公式（int32/int64 精度），输出 0.01°C/0.01hPa 分辨率
- **平台无关** — 通过函数指针解耦，不依赖任何 MCU 或 RTOS

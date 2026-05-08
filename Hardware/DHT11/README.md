# DHT11 温湿度传感器驱动

[![API](https://img.shields.io/badge/API-7%20callbacks-yellowgreen)]()
[![Protocol](https://img.shields.io/badge/Protocol-Single--bus-brightgreen)]()
[![Check](https://img.shields.io/badge/Check-CRC%20sum-orange)]()

基于函数指针接口解耦的 DHT11 单总线驱动，支持 **校验和检测** 和 **符号温度**，可跨平台移植。

## 架构

```
┌──────────────────────────────┐
│   dht11.c / dht11.h          │  ◀── 纯逻辑层，移植时无需修改
│   单总线时序 / 数据解析 / 校验  │
├──────────────────────────────┤
│   DHT11_IO_t                 │  ◀── 7 个函数指针的接口
│   set_output / set_input     │
│   write_low / write_high     │
│   read_pin / delay_us/ms     │
├──────────────────────────────┤
│   用户代码 (端口适配层)        │  ◀── 移植时只需实现此处
│   GPIO / 延时函数             │
└──────────────────────────────┘
```

## 特性

- **单总线协议** — 完整的 DHT11 通信时序，含起始信号和响应检测
- **校验和验证** — 自动校验 5 字节数据，防止误读
- **符号温度** — 支持零下温度（bit7 为符号位）
- **超时保护** — 每个时序阶段都有超时退出，防止死锁

## 数据格式

DHT11 一次传输 40 bit (5 byte) 数据：

| 字节 | 内容 |
|---|---|
| buf[0] | 湿度整数部分 (8bit) |
| buf[1] | 湿度小数部分 (8bit，通常为 0) |
| buf[2] | 温度整数部分 (8bit, bit7 = 符号位) |
| buf[3] | 温度小数部分 (8bit，通常为 0) |
| buf[4] | 校验和 = (buf[0] + ... + buf[3]) & 0xFF |

温度换算：

```
温度 = (buf[2] & 0x7F) . buf[3]   // 单位 °C
若 buf[2] & 0x80，则为负值

湿度 = buf[0] . buf[1]             // 单位 %RH
```

## 错误码

| 宏 | 值 | 含义 |
|---|---|---|
| `DHT11_OK` | 0 | 读取成功 |
| `DHT11_ERR_NULL` | -1 | IO 函数指针为空 |
| `DHT11_ERR_START_LOW` | -2 | 起始信号后总线未拉低 (无响应) |
| `DHT11_ERR_RESP_LOW` | -3 | 响应低电平超时 |
| `DHT11_ERR_RESP_HIGH` | -4 | 响应高电平超时 |
| `DHT11_ERR_READ_BIT` | -5 | 读取数据位时超时 |
| `DHT11_ERR_CHECKSUM` | -6 | 校验和不匹配 |

## API 参考

| 函数 | 说明 |
|---|---|
| `DHT11_Init(DHT11_IO_t *io)` | 初始化驱动，置位总线空闲高电平 |
| `DHT11_ReadData(DHT11_Data_t *data)` | 读取温湿度数据，成功返回 0 |

### 数据结构

```c
typedef struct {
    int16_t temperature;    // 温度，单位 0.1°C (例: 235 = 23.5°C)
    uint16_t humidity;      // 湿度，单位 0.1% (例: 655 = 65.5%)
    uint8_t raw[5];         // 原始 5 字节数据 (用于调试)
} DHT11_Data_t;
```

## 文件说明

| 文件 | 说明 |
|---|---|
| `dht11.h` | API 声明 + `DHT11_IO_t` / `DHT11_Data_t` 定义 + 错误码 |
| `dht11.c` | 单总线时序驱动 + 数据解析 + 校验和验证 |

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

- PB12 配置为 **GPIO_Output**，初始 High，无上拉
- RCC: HSE 外部晶振，系统时钟 72MHz

### 2. 实现 7 个回调函数

```c
/* main.c USER CODE BEGIN PD */
#define DHT11_PIN       12
#define DHT11_PORT      GPIOB
#define DHT11_CRH_SHIFT ((DHT11_PIN - 8) * 4)
/* USER CODE END PD */

/* main.c USER CODE BEGIN PV */
static DHT11_Data_t dht11_data;
/* USER CODE END PV */

/* main.c USER CODE BEGIN 0 */
static void DHT11_SetOutput(void)
{
    DHT11_PORT->CRH &= ~(0xF << DHT11_CRH_SHIFT);
    DHT11_PORT->CRH |= (0x7 << DHT11_CRH_SHIFT);  /* PP 50MHz */
}

static void DHT11_SetInput(void)
{
    DHT11_PORT->CRH &= ~(0xF << DHT11_CRH_SHIFT);
    DHT11_PORT->CRH |= (0x8 << DHT11_CRH_SHIFT);  /* Input + PU/PD */
    DHT11_PORT->ODR |= (1U << DHT11_PIN);          /* 内部上拉 */
}

static void DHT11_WriteLow(void)
{
    DHT11_PORT->BSRR = (1U << (DHT11_PIN + 16));  /* BRx */
}

static void DHT11_WriteHigh(void)
{
    DHT11_PORT->BSRR = (1U << DHT11_PIN);          /* BSx */
}

static uint8_t DHT11_ReadPin(void)
{
    return (DHT11_PORT->IDR >> DHT11_PIN) & 1;
}

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < us * 72);
}

static void DHT11_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static DHT11_IO_t dht11_io = {
    .set_output = DHT11_SetOutput,
    .set_input  = DHT11_SetInput,
    .write_low  = DHT11_WriteLow,
    .write_high = DHT11_WriteHigh,
    .read_pin   = DHT11_ReadPin,
    .delay_us   = DHT11_DelayUs,
    .delay_ms   = DHT11_DelayMs,
};
/* USER CODE END 0 */
```

> PB12 属于高位引脚 (CRH)，`DHT11_CRH_SHIFT` 计算该引脚在 CRH 寄存器中的偏移。若改用低位引脚 (PA0~PA7)，需改为 CRL 和对应移位。

### 3. 初始化 DWT 和 DHT11

DWT 提供微秒级延时，需在 `USER CODE BEGIN 2` 中初始化：

```c
/* USER CODE BEGIN 2 */
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

DHT11_Init(&dht11_io);

/* 首次读取丢弃 (DHT11 首次返回上次测量值) */
HAL_Delay(1000);
DHT11_ReadData(&dht11_data);
HAL_Delay(100);
/* USER CODE END 2 */
```

### 4. 在主循环中读取

```c
/* USER CODE BEGIN WHILE */
uint32_t last_dht11 = 0;
while (1)
{
    uint32_t now = HAL_GetTick();

    if (now - last_dht11 >= 2000)           /* DHT11 采样周期 1s，建议 ≥ 2s */
    {
        last_dht11 = now;

        if (DHT11_ReadData(&dht11_data) == DHT11_OK)
        {
            int16_t t = dht11_data.temperature;  /* 235 = 23.5°C */
            uint16_t h = dht11_data.humidity;    /* 655 = 65.5% */
        }
    }
    /* USER CODE END WHILE */
}
```

### 完整示例 (配合 OLED)

```c
/* USER CODE BEGIN 2 */
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

OLED_Init(&oled_io);
DHT11_Init(&dht11_io);

HAL_Delay(1000);
DHT11_ReadData(&dht11_data);
HAL_Delay(100);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
uint32_t last_dht11 = 0;
while (1)
{
    uint32_t now = HAL_GetTick();

    if (now - last_dht11 >= 2000)
    {
        last_dht11 = now;

        if (DHT11_ReadData(&dht11_data) == DHT11_OK)
        {
            int16_t t = dht11_data.temperature;
            uint16_t h = dht11_data.humidity;

            char buf[16];
            snprintf(buf, sizeof(buf), "Temp: %d.%d C", t / 10, t % 10);
            OLED_ShowString(0, 20, buf, 1);

            snprintf(buf, sizeof(buf), "Humi: %d.%d %%", h / 10, h % 10);
            OLED_ShowString(0, 36, buf, 1);
        }

        OLED_Display();
    }
    /* USER CODE END WHILE */
}
```

### 注意事项

- 读取间隔建议 **≥ 2 秒**（DHT11 采样周期 1 秒）
- **首次读取数据是上一次测量值**，建议丢弃
- 如果持续返回 `ERR_CHECKSUM`，检查 **上拉电阻** 和接线
- DWT 延时 `us × 72` 基于 72MHz 时钟，修改系统时钟需同步调整

## 移植到其他平台

1. 复制 `dht11.h`、`dht11.c` 到新项目
2. 实现 `DHT11_IO_t` 的 7 个回调函数
3. 调用 `DHT11_Init()` 初始化，`DHT11_ReadData()` 读取数据

| 回调 | 功能 | 实现说明 |
|---|---|---|
| `set_output()` | 切换 GPIO 为推挽输出 | 用于主机拉低总线发起通信 |
| `set_input()` | 切换 GPIO 为浮空/上拉输入 | 用于释放总线，检测从机信号 |
| `write_low()` | 引脚输出低电平 | 拉低总线 |
| `write_high()` | 引脚输出高电平 | 拉高总线 |
| `read_pin()` | 读取引脚电平 | 返回 0 或 1 |
| `delay_us(us)` | 微秒级阻塞延时 | 精度要求 ±1µs，建议使用 DWT/CoreTimer |
| `delay_ms(ms)` | 毫秒级阻塞延时 | 可使用 HAL_Delay 或 RTOS 延时 |

# HC-SR04 超声波测距驱动

[![API](https://img.shields.io/badge/API-3%20callbacks-yellowgreen)]()
[![Filter](https://img.shields.io/badge/Filter-EMA%20smoothing-brightgreen)]()

基于函数指针接口解耦的 HC-SR04 超声波测距驱动，支持 **EMA 滤波** 和 **16 位计数器溢出补偿**，可跨平台移植。

## 架构

```
┌──────────────────────────────┐
│   hcsr04.c / hcsr04.h        │  ◀── 纯逻辑层，移植时无需修改
│   脉冲测量 / 距离换算 / 滤波   │
├──────────────────────────────┤
│   HCSR04_IO_t                │  ◀── 3 个函数指针的接口
│   trig / read_echo / get_us  │
├──────────────────────────────┤
│   用户代码 (端口适配层)        │  ◀── 移植时只需实现此处
│   GPIO / 微秒计数器           │
└──────────────────────────────┘
```

## 工作原理

1. MCU 向 Trig 引脚发送 **10µs 以上** 的高电平脉冲
2. 模块自动发射 8 个 40kHz 超声波脉冲
3. Echo 引脚被拉高，等待回波
4. 收到回波后 Echo 拉低，**高电平持续时间与距离成正比**
5. MCU 测量 Echo 高电平宽度，换算距离

```
距离(mm) = 脉冲宽度(µs) × 10 / 58
```

## 滤波器

驱动内置 **EMA (指数移动平均)** 滤波，通过头文件宏配置：

```c
/* 0 = 关闭, 1~7 = 滤波强度（越大越平滑，响应越慢） */
#define HCSR04_FILTER_STRENGTH  3
```

| 强度 | 公式 | 新值占比 | 适用场景 |
|------|------|---------|---------|
| 0 | 无滤波 | 100% | 原始值，用于调试 |
| 1 | `(ema + pulse) / 2` | 50% | 轻量平滑，快速响应 |
| 3 | `(ema × 3 + pulse) / 4` | 25% | 日常使用，兼顾平滑与响应 |
| 7 | `(ema × 7 + pulse) / 8` | 12.5% | 高平滑，适合静态测距 |

## 16 位计数器溢出补偿

驱动依赖 `get_us()` 返回微秒级时间戳。若使用 16 位硬件定时器（如 STM32 TIM3），溢出后差值计算会得到错误结果。建议在 `get_us()` 实现中扩展为 32 位：

```c
static uint16_t last_cnt;
static uint32_t overflow_cnt;

static uint32_t HCSR04_GetUs(void)
{
    uint16_t cnt = TIM3->CNT;
    if (cnt < last_cnt) overflow_cnt++;
    last_cnt = cnt;
    return (overflow_cnt << 16) | cnt;
}
```

## 错误码

| 宏 | 值 | 含义 |
|---|---|---|
| `HCSR04_OK` | 0 | 测量成功 |
| `HCSR04_ERR_NULL` | -1 | IO 函数指针为空 |
| `HCSR04_ERR_TIMEOUT` | -2 | 回波超时 (超出量程或无物体) |
| `HCSR04_ERR_RANGE` | -3 | 物体过近 (< 2cm) |

## API 参考

| 函数 | 说明 |
|---|---|
| `HCSR04_Init(HCSR04_IO_t *io)` | 初始化驱动 |
| `HCSR04_Read(uint32_t *distance_mm)` | 测量距离 (mm)，成功返回 0 |
| `HCSR04_GetPulseUs(void)` | 获取上次回波脉冲宽度 (µs)，用于调试 |

## 文件说明

| 文件 | 说明 |
|---|---|
| `hcsr04.h` | API 声明 + `HCSR04_IO_t` 接口定义 + 滤波配置 + 错误码 |
| `hcsr04.c` | 脉冲测量 + 距离换算 + EMA 滤波实现 |

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

- Trig 引脚设为 **GPIO_Output**，初始 Low
- Echo 引脚设为 **GPIO_Input**，下拉
- 系统时钟 72MHz

### 2. 实现回调函数

```c
/* main.c USER CODE BEGIN PD */
#define HCSR04_TRIG_PORT   GPIOA
#define HCSR04_TRIG_PIN    GPIO_PIN_7
#define HCSR04_ECHO_PORT   GPIOA
#define HCSR04_ECHO_PIN    GPIO_PIN_6
/* USER CODE END PD */

/* main.c USER CODE BEGIN PV */
static uint32_t hcsr04_distance;
/* 使用 16 位定时器时需添加溢出补偿变量 */
static uint16_t hcsr04_last_cnt;
static uint32_t hcsr04_overflow_cnt;
/* USER CODE END PV */

/* main.c USER CODE BEGIN 0 */
static void HCSR04_Trig(void)
{
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_SET);
    uint32_t _t = TIM3->CNT;
    while ((TIM3->CNT - _t) < 15);   /* ~10µs @ 72MHz, 1µs/tick */
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);
}

static uint8_t HCSR04_ReadEcho(void)
{
    return (HAL_GPIO_ReadPin(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN)
            == GPIO_PIN_SET) ? 1 : 0;
}

static uint32_t HCSR04_GetUs(void)
{
    /* 16 位 → 32 位扩展，补偿溢出 */
    uint16_t cnt = TIM3->CNT;
    if (cnt < hcsr04_last_cnt) hcsr04_overflow_cnt++;
    hcsr04_last_cnt = cnt;
    return (hcsr04_overflow_cnt << 16) | cnt;
}

static HCSR04_IO_t hcsr04_io = {
    .trig      = HCSR04_Trig,
    .read_echo = HCSR04_ReadEcho,
    .get_us    = HCSR04_GetUs,
};
/* USER CODE END 0 */
```

### 3. 初始化

```c
/* USER CODE BEGIN 2 */
HAL_TIM_Base_Start(&htim3);      /* 启动 TIM3 提供微秒时基 */
HCSR04_Init(&hcsr04_io);
/* USER CODE END 2 */
```

### 4. 在主循环中读取

```c
/* USER CODE BEGIN WHILE */
uint32_t last_hcsr04 = 0;
while (1)
{
    uint32_t now = HAL_GetTick();

    if (now - last_hcsr04 >= 200)       /* 200ms 间隔 */
    {
        last_hcsr04 = now;

        if (HCSR04_Read(&hcsr04_distance) == HCSR04_OK)
        {
            /* hcsr04_distance: 单位 mm，例: 345 = 34.5cm */
        }
    }
    /* USER CODE END WHILE */
}
```

### 注意事项

- 测量间隔建议 ≥ 60ms（模块最大响应周期）
- 最小有效距离约 2cm（< 2cm 返回 `ERR_RANGE`）
- 最大量程约 400cm（超出返回 `ERR_TIMEOUT`）
- `get_us()` 必须返回单调递增的微秒时间戳，支持回绕
- 若用 DWT 实现 `get_us()`，返回 `DWT->CYCCNT / 72` 即可，无溢出问题

## 移植到其他平台

1. 复制 `hcsr04.h`、`hcsr04.c` 到新项目
2. 实现 `HCSR04_IO_t` 的 3 个回调函数
3. 调用 `HCSR04_Init()` 初始化，`HCSR04_Read()` 读取距离

| 回调 | 功能 | 实现说明 |
|---|---|---|
| `trig()` | 触发测量 | Trig 引脚输出 10µs+ 高电平脉冲 |
| `read_echo()` | 读 Echo 引脚 | 返回 0 或 1 |
| `get_us()` | 获取微秒时间戳 | 单调递增，32 位以上宽度，支持回绕 |

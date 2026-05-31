# MFRC522 RFID 读卡模块驱动

[![API](https://img.shields.io/badge/API-12%20functions-yellowgreen)]()
[![Protocol](https://img.shields.io/badge/Protocol-SPI-brightgreen)]()
[![Check](https://img.shields.io/badge/Check-CRC16-orange)]()

基于函数指针接口解耦的 MFRC522 SPI 驱动，支持 **ISO 14443A** 寻卡、防碰撞、选卡、认证、读写扇区，可跨平台移植。

## 架构

```
┌──────────────────────────────────────┐
│   rc522.c / rc522.h                  │  ◀── 纯逻辑层，移植时无需修改
│   寄存器读写 / ISO14443A 协议 / CRC   │
├──────────────────────────────────────┤
│   RC522_IO_t                         │  ◀── 6 个函数指针的接口
│   cs_low / cs_high / rst_low / rst_high│
│   spi_transfer / delay_ms            │
├──────────────────────────────────────┤
│   用户代码 (端口适配层)                │  ◀── 移植时只需实现此处
│   SPI 外设 / GPIO 控制 / 延时函数      │
└──────────────────────────────────────┘
```

## 特性

- **ISO 14443A 协议** — 完整的寻卡 → 防碰撞 → 选卡 → 认证 → 读写流程
- **CRC16 硬件计算** — 利用 MFRC522 内置 CRC 协处理器
- **多卡防碰撞** — 支持场内多卡环境下的 UID 读取
- **Mifare Classic 读写** — 支持扇区认证（KeyA/KeyB）和 16 字节数据块读写
- **超时保护** — 每个通信阶段都有超时退出，防止死锁
- **零 HAL 依赖** — 纯 C 实现，仅依赖 `<stdint.h>` 和 `<stddef.h>`

## 错误码

| 宏 | 值 | 含义 |
|---|---|---|
| `RC522_OK` | 0 | 操作成功 |
| `RC522_ERR_NULL` | -1 | IO 函数指针为空 |
| `RC522_ERR_VERSION` | -2 | 芯片版本读取失败（SPI 通信异常或芯片未正确复位） |
| `RC522_ERR_TIMEOUT` | -3 | 等待卡片响应超时 |
| `RC522_ERR_CRC` | -4 | CRC 计算超时 |
| `RC522_ERR_COLLISION` | -5 | 防碰撞检测到多卡冲突 |
| `RC522_ERR_BCC` | -6 | UID 校验字节 (BCC) 不匹配 |
| `RC522_ERR_TRANS` | -7 | 数据传输错误（协议错误、奇偶校验错误等） |
| `RC522_ERR_AUTH` | -8 | 扇区认证失败 |

## API 参考

| 函数 | 说明 |
|---|---|
| `RC522_Init(RC522_IO_t *io)` | 初始化驱动，绑定 IO 接口 |
| `RC522_Reset()` | 硬件复位 + 寄存器初始化，返回 `RC522_OK` 或 `RC522_ERR_VERSION` |
| `RC522_GetVersion()` | 读取芯片版本寄存器 (0x37)，正常值为 0x91 或 0x92 |
| `RC522_CheckCard(uint8_t *card_type)` | 寻卡，成功时 `card_type[0..1]` 为 ATQA 应答 |
| `RC522_Anticoll(uint8_t *uid)` | 防碰撞，读取 4 字节 UID |
| `RC522_SelectTag(uint8_t *snr, uint8_t *sak)` | 选卡，`sak` 返回选中应答字节 |
| `RC522_AuthState(uint8_t auth_mode, uint8_t addr, uint8_t *key, uint8_t *snr)` | 认证扇区，`auth_mode` 为 `PICC_AUTHENT1A` 或 `PICC_AUTHENT1B` |
| `RC522_ReadBlock(uint8_t addr, uint8_t *data)` | 读取 16 字节数据块 |
| `RC522_WriteBlock(uint8_t addr, uint8_t *data)` | 写入 16 字节数据块 |
| `RC522_Halt()` | 使卡片进入休眠状态 |
| `RC522_AntennaOn()` | 开启天线发射 |
| `RC522_AntennaOff()` | 关闭天线发射 |

### 接口结构

```c
typedef struct {
    void (*cs_low)(void);           // 片选拉低
    void (*cs_high)(void);          // 片选拉高
    void (*rst_low)(void);          // 复位拉低
    void (*rst_high)(void);         // 复位拉高
    void (*spi_transfer)(uint8_t *tx, uint8_t *rx, uint16_t len);  // SPI 全双工传输
    void (*delay_ms)(uint32_t ms);  // 毫秒延时
} RC522_IO_t;
```

### 卡片类型识别

`RC522_CheckCard()` 成功后，`card_type` 的 ATQA 值可判断卡片类型：

| ATQA | 卡片类型 |
|---|---|
| 0x0400 | Mifare Classic S50 (1K) |
| 0x0200 | Mifare Classic S70 (4K) |
| 0x4400 | Mifare Ultralight |
| 0x0800 | Mifare Pro |
| 0x4403 | Mifare DESFire |

## 文件说明

| 文件 | 说明 |
|---|---|
| `rc522.h` | API 声明 + `RC522_IO_t` / `RC522_CardInfo_t` 定义 + 错误码 + 寄存器/命令宏 |
| `rc522.c` | SPI 寄存器读写 + ISO 14443A 协议 + CRC16 计算 + 防碰撞/认证/读写 |

## 在 STM32 HAL 中使用

### 1. CubeMX 配置

- SPI1: Master, 8-bit, CPOL=Low, CPHA=1Edge, MSB First, BaudRate Prescaler=32 (2.25MHz)
- PA3: GPIO_Output (CS)，初始 High
- PA4: GPIO_Output (RST)，初始 High
- PA5: SPI1_SCK
- PA6: SPI1_MISO
- PA7: SPI1_MOSI

### 2. 实现 6 个回调函数

```c
/* main.c USER CODE BEGIN 0 */
extern SPI_HandleTypeDef hspi1;

static void RC522_CS_Low(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

static void RC522_CS_High(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

static void RC522_RST_Low(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

static void RC522_RST_High(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

static void RC522_SPITransfer(uint8_t *tx, uint8_t *rx, uint16_t len)
{
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, len, HAL_MAX_DELAY);
}

static void RC522_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static RC522_IO_t rc522_io = {
    .cs_low       = RC522_CS_Low,
    .cs_high      = RC522_CS_High,
    .rst_low      = RC522_RST_Low,
    .rst_high     = RC522_RST_High,
    .spi_transfer = RC522_SPI_Transfer,
    .delay_ms     = RC522_DelayMs,
};
/* USER CODE END 0 */
```

### 3. 初始化并读卡

```c
/* USER CODE BEGIN 2 */
RC522_Init(&rc522_io);

if (RC522_Reset() != RC522_OK)
{
    // 初始化失败，检查 SPI 接线和 RST 引脚
    Error_Handler();
}
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
uint8_t card_type[2];
uint8_t uid[4];
uint8_t sak;

while (1)
{
    if (RC522_CheckCard(card_type) == RC522_OK)
    {
        if (RC522_Anticoll(uid) == RC522_OK)
        {
            if (RC522_SelectTag(uid, &sak) == RC522_OK)
            {
                // 卡片读取成功，uid[0..3] 为 4 字节 UID
                // 可继续调用 RC522_AuthState + RC522_ReadBlock 读取扇区数据
            }
            RC522_Halt();
        }
    }

    HAL_Delay(500);
    /* USER CODE END WHILE */
}
```

### 4. 读写 Mifare Classic 扇区

```c
uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // 默认密钥
uint8_t block_addr = 4;   // 扇区 1 的第 0 块
uint8_t read_buf[16];
uint8_t write_buf[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

// 认证扇区
if (RC522_AuthState(PICC_AUTHENT1A, block_addr, key, uid) == RC522_OK)
{
    // 读取数据
    if (RC522_ReadBlock(block_addr, read_buf) == RC522_OK)
    {
        // read_buf[0..15] 为 16 字节数据
    }

    // 写入数据
    if (RC522_WriteBlock(block_addr, write_buf) == RC522_OK)
    {
        // 写入成功
    }
}
```

### 注意事项

- MFRC522 SPI 时钟最高 10MHz，建议使用 **Prescaler=32** (2.25MHz) 保证稳定性
- **RST 引脚必须正确控制** — 初始化时先拉低再拉高，芯片才能正常工作
- 寻卡间隔建议 **≥ 500ms**，避免频繁轮询
- 扇区尾块 (块 3/7/11/...) 存储密钥和权限位，**切勿随意写入**
- `card_type` 为 ATQA 应答，仅在 `CheckCard` 成功后有效

## 移植到其他平台

1. 复制 `rc522.h`、`rc522.c` 到新项目
2. 实现 `RC522_IO_t` 的 6 个回调函数
3. 调用 `RC522_Init()` 绑定接口，`RC522_Reset()` 初始化芯片

| 回调 | 功能 | 实现说明 |
|---|---|---|
| `cs_low()` | 片选拉低 | 选中 MFRC522，开始 SPI 事务 |
| `cs_high()` | 片选拉高 | 释放 MFRC522，结束 SPI 事务 |
| `rst_low()` | 复位拉低 | 硬件复位 MFRC522 |
| `rst_high()` | 复位拉高 | 退出硬件复位，芯片正常工作 |
| `spi_transfer(tx, rx, len)` | SPI 全双工传输 | 发送 `tx[0..len-1]`，接收存入 `rx[0..len-1]` |
| `delay_ms(ms)` | 毫秒级阻塞延时 | 可使用 HAL_Delay 或 RTOS 延时 |

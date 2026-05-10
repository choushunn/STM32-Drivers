# STM32F103C8T6 硬件驱动库

基于 STM32 HAL 库的模块化硬件驱动集合。每个驱动通过函数指针结构体与 MCU 硬件解耦，驱动代码不包含任何 HAL 或 CMSIS 头文件引用。

## 目录结构

```
F103-Driver-Lab/
├── F103-Driver-Lab.ioc          STM32CubeMX 项目配置文件
├── Core/                        CubeMX 生成的 HAL 初始化代码
├── MDK-ARM/                     Keil MDK 项目文件与编译输出
├── Drivers/                     CubeMX 生成的 STM32 HAL + CMSIS 库 
├── Hardware/                    硬件驱动层，每驱动独立子目录
│   ├── AHT20_BMP280/            温湿度+气压传感器 (I2C)
│   ├── DAC1220/                 20 位 DAC (3线 SDIO)
│   ├── DHT11/                   温湿度传感器 (单总线)
│   ├── Encoder/                 旋转编码器 (正交解码 GPIO)
│   ├── HCSR04/                  超声波测距 (GPIO+定时器)
│   ├── Joystick/                模拟摇杆 (双路 ADC+GPIO)
│   ├── MPU6050/                 6 轴惯性测量单元 (I2C)
│   ├── OLED/                    128x64 OLED 显示屏 (I2C)
│   ├── SG90/                    舵机 (PWM)
│   ├── TEMT6000/                环境光传感器 (ADC)
│   ├── TOF050F/                 ToF 激光测距 (I2C)
│   └── ZS040/                   蓝牙串口模块 (UART)
├── .gitignore                   Git 忽略规则
├── README.md                    本文件
└── TODOLIST.md                  驱动开发进度清单
```

`Core/`、`MDK-ARM/`、`Drivers/` 由 CubeMX 生成和管理。`Hardware/` 为手动编写的驱动代码。

## 驱动设计

### IO 抽象层

每个驱动定义一个 `{NAME}_IO_t` 结构体，包含若干函数指针。驱动逻辑只通过这些指针调用底层硬件，不直接引用任何寄存器或 HAL API。

各驱动的 IO 接口：

| 驱动 | IO 接口字段 | 依赖的硬件资源 |
|------|------------|---------------|
| OLED | `write_cmd`, `write_data` | I2C 主模式 |
| DHT11 | `set_output`, `set_input`, `write_low`, `write_high`, `read_pin`, `delay_us`, `delay_ms` | GPIO + 微秒定时器 |
| HCSR04 | `trig`, `read_echo`, `get_us` | GPIO + 微秒定时器 |
| Joystick | `read_x`, `read_y`, `read_btn` | ADC 双通道 + GPIO |
| Encoder | `read_a`, `read_b`, `read_btn`, `get_ms` | GPIO + 毫秒定时器 |
| SG90 | `set_pulse`, `get_ms` | PWM 输出 + 毫秒定时器 |
| DAC1220 | `cs`, `sdio_dir`, `send_byte`, `recv_byte`, `delay_us` | GPIO bit-bang SPI + 微秒定时器 |
| ZS040 | `uart_send`, `set_en`, `set_key`, `read_state`, `get_ms`, `delay_ms` | UART + GPIO + 毫秒定时器 |
| MPU6050 | `read(dev_addr, reg, data, len)`, `write(...)`, `delay_ms` | I2C 主模式 |
| AHT20_BMP280 | `read(dev_addr, reg, data, len)`, `write(...)`, `delay_ms` | I2C 主模式 |
| TOF050F | `read_reg(reg16, data, len)`, `write_reg(...)`, `delay_ms` | I2C 主模式 |
| TEMT6000 | `read_adc` | ADC 单通道 |

### 驱动约束

- 每个驱动仅引用 `<stdint.h>`、`<stddef.h>`、`<string.h>` 等 C 标准库头文件
- 不引用 `main.h`、`stm32f1xx_hal.h` 或任何 CMSIS 头文件
- 内部状态通过 `static` 文件作用域变量隐藏，对外不可见
- 所有 API 函数返回 `int8_t`：`0` 表示正常，负值表示错误码
- 初始化接口接受 `{NAME}_IO_t*` 参数，在内部保存为 `static` 指针
- 不包含引脚号、外设实例等硬件配置，全部通过 IO 回调注入

### 目录内文件约定

每个 `Hardware/<name>/` 目录包含：

- `<name>.c` — 驱动实现
- `<name>.h` — 公开 API 和 `{NAME}_IO_t` 结构体定义
- `README.md` — 硬件接线说明和 CubeMX 配置步骤（可选）
- `*.pdf` — 芯片数据手册（可选）

## 工具链

| 组件 | 工具 | 版本 |
|------|------|------|
| MCU | STM32F103C8T6 | - |
| 时钟 | HSE 8MHz → PLL x9 → SYSCLK 72MHz | - |
| HAL 库生成 | STM32CubeMX | v6.x (推荐) |
| HAL 框架 | STM32Cube FW_F1 | v1.8.x |
| IDE/编译器 | Keil MDK-ARM (Armcc v5 / Armclang v6) | v5.x |
| 调试器 | ST-Link/V2 (SWD) | - |
| 串口调试 | USART1 @ 115200-8-N-1, PA9(TX) PA10(RX) | - |
| 蓝牙串口 | USART2, PA2(TX) PA3(RX) | - |
| I2C 外设 | I2C1 @ 400kHz, PB6(SCL) PB7(SDA) | - |
| 版本控制 | Git | - |

## 首次使用

1. 安装 STM32CubeMX v6.x
2. 安装 Keil MDK-ARM v5.x
3. 克隆仓库：

   ```bash
   git clone <repo-url>
   cd F103-Driver-Lab
   ```

4. 用 CubeMX 打开 `F103-Driver-Lab.ioc`，点击 **Project → Generate Code**，生成 `Drivers/` 目录
5. 用 Keil MDK 打开 `MDK-ARM/F103-Driver-Lab.uvprojx`，编译并下载

## 硬件接线

各驱动的引脚连接说明见其 `Hardware/<name>/README.md`。公用总线：

| 总线 | 引脚 | 挂载设备 |
|------|------|---------|
| I2C1 | PB6(SCL) PB7(SDA) | OLED, MPU6050, AHT20_BMP280, TOF050F |
| USART2 | PA2(TX) PA3(RX) | ZS040 (JDY-31-SPP) |
| USART1 | PA9(TX) PA10(RX) | 调试日志输出 |
| TIM3 | CH1-PA6, CH2-PA7 | SG90 舵机 PWM |

## 驱动清单

| 驱动 | 目录 | 通信方式 | 依赖 IO 回调数 |
|------|------|---------|---------------|
| OLED 显示屏 | `Hardware/OLED/` | I2C | 2 |
| DHT11 温湿度 | `Hardware/DHT11/` | 单总线 GPIO | 7 |
| HC-SR04 超声波 | `Hardware/HCSR04/` | GPIO + 定时器 | 3 |
| 模拟摇杆 | `Hardware/Joystick/` | ADC + GPIO | 3 |
| 旋转编码器 | `Hardware/Encoder/` | GPIO 中断 | 4 |
| SG90 舵机 | `Hardware/SG90/` | PWM | 2 |
| DAC1220 数模转换 | `Hardware/DAC1220/` | 3线 SDIO | 5 |
| ZS040 蓝牙 | `Hardware/ZS040/` | UART + GPIO | 7 |
| MPU6050 姿态 | `Hardware/MPU6050/` | I2C | 3 |
| AHT20+BMP280 环境 | `Hardware/AHT20_BMP280/` | I2C | 3 |
| TOF050F 测距 | `Hardware/TOF050F/` | I2C | 3 |
| TEMT6000 光照 | `Hardware/TEMT6000/` | ADC | 1 |

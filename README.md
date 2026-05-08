# STM32F103C8T6 多传感器显示终端

[![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-blue)]()
[![Framework](https://img.shields.io/badge/Framework-HAL-green)]()
[![IDE](https://img.shields.io/badge/IDE-Keil%20MDK-orange)]()

基于 STM32 HAL 库的多传感器采集与显示项目，支持 **DHT11 温湿度**、**HC-SR04 超声波测距**、**模拟摇杆** 和 **旋转编码器** 输入，数据通过 **OLED** 屏幕展示。驱动层全部采用函数指针接口解耦，可跨平台移植。

## 硬件接线

| 外设 | 引脚 | 协议 |
|---|---|---|
| OLED | PB6 (SCL) / PB7 (SDA) | I2C (地址 0x78) |
| DHT11 | PB12 | 单总线 (需 4.7kΩ 上拉) |
| HC-SR04 Trig | PA7 | GPIO 输出 |
| HC-SR04 Echo | PA6 | TIM3_CH1 (输入捕获) |
| USART1 | PA9 (TX) / PA10 (RX) | 115200bps |

## 项目结构

```
F103C8T6-LCD/
├── Core/                        # CubeMX HAL 代码
│   ├── Inc/                     #   头文件
│   └── Src/                     #   main.c 及外设初始化
├── Hardware/                    # 硬件驱动层 (跨平台通用)
│   ├── OLED/                    #   SH1106/SSD1306 OLED 驱动
│   ├── DHT11/                   #   DHT11 温湿度传感器驱动
│   ├── HCSR04/                  #   HC-SR04 超声波测距驱动
│   ├── Joystick/                #   模拟摇杆驱动 (双轴ADC + 按键)
│   └── Encoder/                 #   旋转编码器驱动 (正交解码 + 按键)
├── Drivers/                     # STM32 HAL 库 (CubeMX 生成)
├── MDK-ARM/                     # Keil 工程文件
└── F103C8T6-LCD.ioc             # CubeMX 配置文件
```

## 驱动一览

| 驱动 | 接口函数数 | 关键特性 | 文档 |
|---|---|---|---|
| [OLED](Hardware/OLED/README.md) | 2 个 | 绘图 / 字符 / 数值显示, SH1106 + SSD1306 | [README](Hardware/OLED/README.md) |
| [DHT11](Hardware/DHT11/README.md) | 7 个 | 单总线时序, 校验和检测 | [README](Hardware/DHT11/README.md) |
| [HC-SR04](Hardware/HCSR04/README.md) | 3 个 | EMA 滤波, 16位溢出补偿, 超时保护 | [README](Hardware/HCSR04/README.md) |
| [Joystick](Hardware/Joystick/joystick.h) | 3 个 | 双轴ADC + 按键, 方向判别 + 死区 | — |
| [Encoder](Hardware/Encoder/encoder.h) | 4 个 | 正交解码, 按键消抖, 长短按识别 | — |

## 驱动设计特点

所有硬件驱动遵循同一设计模式：

- **接口解耦** — 通过 `*_IO_t` 函数指针结构体分离逻辑与硬件
- **纯 C 实现** — 零依赖，无操作系统要求
- **跨平台移植** — 只需实现几个回调函数即可移植到任意平台
- **CubeMX 友好** — 用户代码全部位于 `USER CODE BEGIN/END` 区域内

## 工具链

- **MCU**: STM32F103C8T6 @ 72MHz (HSE 8MHz + PLL x9)
- **框架**: STM32CubeMX + HAL 库
- **IDE**: Keil MDK-ARM v5 / v6
- **调试**: USART1 串口日志输出

## 快速开始

1. 使用 CubeMX 打开 `F103C8T6-LCD.ioc` 确认引脚配置
2. 用 Keil MDK 打开 `MDK-ARM/F103C8T6-LCD.uvprojx`
3. 编译下载到 STM32F103C8T6 开发板
4. 上电后 OLED 显示各传感器数据

各驱动详细用法见对应 `Hardware/*/README.md`。

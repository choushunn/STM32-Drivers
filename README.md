# STM32F103C8T6 驱动验证项目

[![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-blue)]()
[![Framework](https://img.shields.io/badge/Framework-HAL-green)]()
[![IDE](https://img.shields.io/badge/IDE-Keil%20MDK-orange)]()

基于 STM32 HAL 库的硬件驱动开发与验证项目。每个驱动独立封装，通过函数指针接口与硬件解耦，可直接移植到任意平台。

## 项目结构

```
F103-Driver-Lab/
├── Core/                        # CubeMX 自动生成的 HAL 代码
│   ├── Inc/                     #   头文件
│   └── Src/                     #   main.c 及外设初始化
├── Hardware/                    # 硬件驱动层 (跨平台通用，手动编写)
│   ├── OLED/                    #   1.3/0.96寸 128×64 I2C OLED (SH1106/SSD1306)
│   ├── DHT11/                   #   DHT11 温湿度传感器 (单总线)
│   ├── HCSR04/                  #   HC-SR04 超声波测距 (GPIO + 定时器)
│   ├── Joystick/                #   模拟摇杆 (双轴 ADC + 按键)
│   ├── Encoder/                 #   旋转编码器 (正交解码 + 按键)
│   ├── SG90/                    #   SG90 舵机 (PWM)
│   ├── DAC1220/                 #   DAC1220 20位 DAC (3线 SDIO)
│   └── ZS040/                   #   ZS-040/HC-05 蓝牙模块 (UART + AT 指令)
├── MDK-ARM/                     # Keil 工程文件
├── F103-Driver-Lab.ioc           # CubeMX 配置文件
├── .gitignore                   # 版本忽略规则
├── README.md                    # 本文件
└── TODOLIST.md                  # 驱动开发进度清单
```

## 首次使用

本项目的 `.gitignore` 排除了 `Drivers/` 目录（STM32 HAL 驱动库）
**首次克隆后需执行以下步骤**：

1. 安装 [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
2. 打开 `F103-Driver-Lab.ioc`
3. 点击 **Project → Generate Code**（无需修改任何配置，直接生成即可）
4. CubeMX 会自动生成 `Drivers/` 目录（HAL 库 + CMSIS）
5. 用 Keil MDK 打开 `MDK-ARM/F103-Driver-Lab.uvprojx` 编译

> 如果 CubeMX 版本不同导致 HAL 库有差异，同样执行一次 **Generate Code** 即可自动更新。

## 已实现驱动

| 驱动 | 状态 | 接口数 | 关键特性 |
|---|---|---|---|
| [OLED](Hardware/OLED/README.md) | ✅ 已验证 | 2 | SH1106+SSD1306, 128×64, 绘图/字符/数值 |
| [DHT11](Hardware/DHT11/README.md) | ✅ 已验证 | 7 | 单总线时序, 校验和检测 |
| [HC-SR04](Hardware/HCSR04/README.md) | ✅ 已验证 | 3 | EMA 滤波, 16位溢出补偿, 超时保护 |
| [Joystick](Hardware/Joystick/) | 📝 待验证 | 3 | 双轴 ADC, 方向判别 + 死区 |
| [Encoder](Hardware/Encoder/) | 📝 待验证 | 4 | 正交解码, 按键消抖, 长短按 |
| [SG90](Hardware/SG90/) | 📝 待验证 | 1~2 | PWM 控制, 平滑转动, 范围检查 |
| [DAC1220](Hardware/DAC1220/) | 📝 待验证 | 5 | 20 位精度, 3线 SDIO, 自校准 |
| [ZS040](Hardware/ZS040/) | 📝 待验证 | 7 | 蓝牙透传, AT 指令配置, 状态检测 |

> ✅ 已验证 — 代码已通过硬件测试
> 📝 待验证 — 代码已编写，尚未连接硬件验证

各驱动的 **硬件接线说明**、**CubeMX 配置方法** 和 **API 参考** 见对应 `Hardware/<name>/README.md`。

## 驱动设计特点

所有硬件驱动遵循同一设计模式：

- **接口解耦** — 通过 `*_IO_t` 函数指针结构体分离逻辑与硬件
- **纯 C 实现** — 零依赖，无操作系统要求
- **跨平台移植** — 只需实现几个回调函数即可移植到任意平台
- **CubeMX 友好** — 用户代码全部位于 `USER CODE BEGIN/END` 区域内

## 驱动开发规范

新增驱动需满足以下要求：

1. 在 `Hardware/<name>/` 下创建 `{name}.h` + `{name}.c`
2. 通过 `{NAME}_IO_t` 结构体抽象 IO 层
3. 所有内部状态使用 `static` 隐藏
4. 返回 `int8_t` 错误码（`0`=成功, `负数`=错误）
5. 无硬编码引脚号，全部由回调传入
6. 需编写 `README.md` 说明硬件接线和 CubeMX 配置步骤

## 工具链

- **MCU**: STM32F103C8T6 @ 72MHz (HSE 8MHz + PLL x9)
- **框架**: STM32CubeMX + HAL 库
- **IDE**: Keil MDK-ARM v5 / v6
- **调试**: USART1 串口日志输出 @ 115200bps

## 快速开始

1. 安装 CubeMX，打开 `F103-Driver-Lab.ioc` 执行 **Generate Code** 生成 HAL 库
2. 用 Keil MDK 打开 `MDK-ARM/F103-Driver-Lab.uvprojx`，编译下载
3. 各驱动详细用法见对应 `Hardware/*/README.md`

完整开发清单见 [TODOLIST.md](TODOLIST.md)。

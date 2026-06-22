# STM32F103C8T6 舵机通信桥接固件

SO-ARM100 六自由度机械臂的下位机固件。接收 NUC 上位机（ROS2）串口命令，转换为 LX-16A 总线舵机协议，驱动 6 个舵机实现多关节联动。

## 架构

```
NUC (ROS2) ──USB── CP2102 ──UART(9600)── STM32 ──UART2(115200,半双工)── Yeahbot/TTL ── LX-16A ×6
```

## 功能

- USART1 中断接收 NUC 下发的 0x55 0x55 多舵机控制帧
- 拆帧转换：多舵机帧 → 单舵机 LX-16A 协议帧（0x03→0x01 + 校验和）
- USART2 半双工模式驱动舵机总线
- PC13 LED 状态指示（启动闪 3 次 / 收到数据亮 500ms）

## 协议转换

| | NUC → STM32 (USART1, 9600bps) | STM32 → 舵机 (USART2, 115200bps) |
|---|---|---|
| 帧头 | 0x55 0x55 | 0x55 0x55 |
| 长度/ID | 长度 (N×3+5) | 舵机 ID |
| 指令 | 0x03 (多舵机移动) | 0x01 (位置写入) |
| 帧尾 | 无 | 校验和 1B (~sum) |

## 硬件

| 物料 | 说明 |
|------|------|
| STM32F103C8T6 最小系统板 | 主控芯片 |
| CP2102 USB-TTL | NUC 串口通信 |
| Yeahbot 调试板 (TTL+ZX 模式) | 半双工信号合并 |
| LX-16A 串行总线舵机 ×6 | 执行器 |

## 开发环境

- IDE: Keil MDK v5
- 库: STM32 标准外设库 (Standard Peripheral Library)
- 烧录: ST-Link (SWD)

## 文件结构

```
├── USER/main.c              # 主程序 + USART1 中断接收
├── USER/system_stm32f10x.c  # 系统时钟初始化
├── SYSTEM/uart/uart.c       # USART1 初始化 (PA9/PA10, 9600bps)
├── SYSTEM/delay/delay.c     # μs/ms 延时
├── servo/servo.c            # 核心：USART2 初始化 + 协议转换
└── CORE/                    # CMSIS 内核 + 启动文件
```

## 上层项目

本固件对应的 ROS2 上位机项目：SO-ARM100 机械臂完整控制系统（Level 0~3）

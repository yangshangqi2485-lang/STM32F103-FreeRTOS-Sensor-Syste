本项目基于 STM32CubeMX 生成 CMake 工程。

使用前，请使用 CubeMX 重新生成代码。

# 基于 FreeRTOS 的手持多传感器采集与显示系统

## 📌 项目简介
本项目基于 STM32F103C8T6 微控制器与 FreeRTOS 实时操作系统，搭建了一套手持传感器数据采集与显示系统。实现了 NTC 温度采集与 VL53L0X 激光测距的高频并发处理，并将测量数据解耦渲染于 OLED 屏幕上。

## 🛠️ 硬件与外设配置
- **主控芯片：** STM32F103C8T6 (ARM Cortex-M3)
- **操作系统：** FreeRTOS (CMSIS-RTOS V2 API)
- **传感器：** VL53L0X (ToF 激光测距，I2C 协议)、NTC 热敏电阻 (温度采集)
- **显示与交互：** 0.96寸 OLED 显示屏 (软件 I2C)、独立按键 (GPIOB)
- **底层驱动与硬件外设：** ADC + DMA 循环采样、软件模拟 I2C、GPIO 状态机

## 🚀 软件架构与核心亮点

1. **数据与 UI 彻底解耦（生产者-消费者模型）**
   - 自定义 `ScreenMsg_t` 结构体，结合 `enum` 标记设备类型、`union` 压缩内存（共用存储 `int` 距离与 `float` 温度）。
   - 采用 FreeRTOS **消息队列（Message Queue）** 将传感器采集任务与 OLED 显示渲染任务彻底分离，降低任务耦合度。

2. **共享 I2C 总线互斥保护**
   - 针对 OLED 与 VL53L0X 传感器共享同一 I2C 总线可能导致的并发冲突与死机问题，使用 FreeRTOS **互斥量 (`osMutex`)** 建立临界区保护，确保总线操作的线程安全。

3. **ADC + DMA 静默高频采集**
   - 配置 ADC 配合 DMA 循环模式（Circular Mode）进行 NTC 模拟信号后台静默采样，无需 CPU 频繁干预；基于 B值法转换公式进行温度计算。

4. **模块化按键驱动设计**
   - 采用指针传递替代局部 `static` 变量，消除多实例调用时的内存冲突，实现按键边沿检测与状态更新。

## 📁 目录结构说明
```text
├── Core/               # 核心系统代码 (main.c, freertos.c 等)
├── BSP/                # 板级外设驱动 (BSP_I2C, OLED, NTC, BUTTON, VL53L0X)
└── cmake/              # CMake 工程文件

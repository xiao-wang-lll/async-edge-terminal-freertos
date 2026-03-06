# 基于FreeRTOS 与全异步架构的智能边缘终端

<p align="center">
  <img src="https://img.shields.io/badge/MCU-STM32F407VG-green?style=for-the-badge" alt="MCU">
  <img src="https://img.shields.io/badge/Architecture-Cortex--M4-blue?style=for-the-badge" alt="Architecture">
  <img src="https://img.shields.io/badge/OS-FreeRTOS-green?style=for-the-badge" alt="OS">
  <img src="https://img.shields.io/badge/Language-C-orange?style=for-the-badge" alt="Language">
  <img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge" alt="License">
</p>


<p align="center">
  <img src="https://img.shields.io/github/stars/yourusername/stm32f4-weather-clock?style=social" alt="Stars">
  <img src="https://img.shields.io/github/forks/yourusername/stm32f4-weather-clock?style=social" alt="Forks">
  <img src="https://img.shields.io/github/issues/yourusername/stm32f4-weather-clock?style=social" alt="Issues">
</p>

English | [中文](README_CN.md)

---

##  项目介绍

基于 **STM32F407VG** 和 **FreeRTOS** 的物联网智能边缘终端，实现在线实时时间显示、室内环境监测、网络天气数据、Wifi信息以及信号显示等功能。

这是一个从零开始手写的嵌入式项目，涵盖了：

-  底层驱动开发（SPI、I2C、UART、GPIO等）
-  FreeRTOS 多任务调度
-  ESP32-C3 WiFi 模块通信
-  UI 界面设计与优化
-  网络协议实现（HTTP、SNTP）

---

## 功能特性

| 功能           | 描述                                        |
| -------------- | ------------------------------------------- |
| **实时时钟**   | RTC硬件计时，网络自动对时（SNTP），断电保持 |
| **室内监测**   | AHT20、DHT11 数字温湿度传感器采集           |
| **天气数据**   | 通过 ESP32-C3 获取心知天气 API 数据         |
| **高清显示**   | 240×320 ST7789 IPS 全彩显示屏               |
| **多任务架构** | FreeRTOS 任务调度，响应流畅                 |
| **模块化设计** | 驱动层与应用层解耦，易于维护                |
| **日志系统**   | EasyLogger 超轻量级日志库                   |
| **即插即用**   | 支持 WiFi 断线自动重连                      |

---

## 硬件要求

### 核心控制器

- **MCU**: STM32F407VG
- **主频**: 168 MHz
- **Flash**: 1 MB
- **SRAM**: 192 KB

### 外设配置

| 外设         | 型号/规格          |
| ------------ | ------------------ |
| 屏幕         | ST7789 IPS 240×240 |
| WiFi模块     | ESP32-C3           |
| 温湿度传感器 | DHT11 / AHT20      |
| 存储芯片     | AT24C32 EEPROM     |

### 引脚连接（参考）

```
SPI1 (ST7789):
- SCK: PA5
- MISO: PA6
- MOSI: PA7
- CS: PA4
- DC: PA2
- RST: PA3

I2C1 (AHT20/AT24C32):
- SCL: PB6
- SDA: PB7

USART1 (ESP32-C3):
- TX: PA9
- RX: PA10

DHT11:
- DATA: PB5
```

---

## 项目结构

```
stm32f4-weather-clock/
├── app/                    # 应用层代码
│   ├── main.c             # 程序入口
│   ├── app.c              # 应用逻辑
│   ├── ui.c               # UI引擎（队列化解耦）
│   ├── weather.c          # 天气数据处理
│   ├── wifi.c             # WiFi管理
│   ├── workqueue.c        # 工作队列实现
│   ├── page/              # 页面模块
│   ├── font/              # 矢量字库
│   └── image/             # 图片资源
├── driver/                # 硬件驱动层
│   ├── st7789/           # LCD屏幕驱动
│   ├── esp_at/           # ESP32 AT指令封装
│   ├── dht11/            # 温湿度传感器
│   ├── aht20/            # AHT20驱动
│   ├── rtc/              # 实时时钟
│   └── ...
├── third_lib/            # 第三方库
│   ├── FreeRTOS/         # 实时操作系统
│   └── EasyLogger/        # 日志库
├── firmware/             # STM32 HAL库
├── resources/            # 设计资源
└── mdk/                 # Keil工程文件
```

---

##  快速开始

### 1. 克隆项目

```bash
git clone https://github.com/yourusername/stm32f4-weather-clock.git
cd stm32f4-weather-clock
```

### 2. 配置 WiFi

在 `app/wifi.c` 中修改以下内容：

```c
#define WIFI_SSID     "Your_SSID"      // 你的WiFi名称
#define WIFI_PASSWORD "Your_PASSWORD"  // 你的WiFi密码
```

### 3. 配置天气 API

在 `app/weather.c` 中设置心知天气密钥：

```c
#define WEATHER_KEY "Your_API_Key"
```

### 4. 编译烧录

使用 Keil MDK 打开 `mdk/stm32F407.uvprojx` 文件，编译并烧录到开发板。

---

## 技术架构

### 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐  │
│  │Time Sync │ │WiFi Mgr  │ │Weather   │ │UI Engine │  │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘  │
│       │            │            │            │         │
│       └────────────┴─────┬───────┴────────────┘         │
│                          ▼                               │
│              ┌─────────────────────┐                    │
│              │    Work Queue       │                    │
│              └──────────┬──────────┘                    │
└─────────────────────────┼───────────────────────────────┘
                          ▼
┌─────────────────────────┼───────────────────────────────┐
│              FreeRTOS Scheduler                          │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │
│  │Task 1   │ │Task 2   │ │Task 3   │ │UI Task   │   │
│  │(Timer)  │ │(Timer)  │ │(Timer)  │ │(Queue)   │   │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘   │
└─────────────────────────┬───────────────────────────────┘
                          ▼
┌─────────────────────────┼───────────────────────────────┐
│                    Driver Layer                          │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐        │
│  │ST7789│ │ESP_AT│ │DHT11 │ │RTC   │ │I2C   │        │
│  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘        │
└──────────────────────────────────────────────────────────┘
                          ▼
┌──────────────────────────────────────────────────────────┐
│                    Hardware                              │
│   STM32F407  │  ESP32-C3  │  ST7789  │  DHT11        │
└──────────────────────────────────────────────────────────┘
```

### 核心技术点

#### 1. UI 队列化解耦

采用 FreeRTOS 队列实现多任务刷屏同步，避免显示冲突：

```c
// 消息结构设计（联合体 + 结构体）
typedef struct {
    ui_action_t action;
    union {
        struct { ... } fill_color;
        struct { ... } write_string;
        struct { ... } draw_image;
    } data;
} ui_message_t;
```

#### 2. 工作队列模式

异步执行模块任务，避免阻塞：

```c
// 定时器任务 -> 工作队列 -> 异步执行
void work_timer_cb(TimerHandle_t timer) {
    uint32_t event = (uint32_t)pvTimerGetTimerID(timer);
    workqueue_run(app_work, (void *)event);
}
```

#### 3. 内存优化

- 动态内存分配解决栈空间限制
- 联合体节省消息结构体空间
- UI 队列深度合理设置（16条消息）

---

## 任务调度说明

| 任务名称       | 触发周期 | 优先级 | 功能描述                |
| -------------- | -------- | ------ | ----------------------- |
| time_sync      | 1小时    | 中     | SNTP网络对时，同步RTC   |
| wifi_update    | 5秒      | 中     | WiFi状态检测与重连      |
| time_update    | 1秒      | 高     | 时间显示刷新（VIP通道） |
| inner_update   | 3秒      | 低     | 室内温湿度采集          |
| outdoor_update | 3分钟    | 低     | 室外天气数据获取        |
| ui_task        | 队列触发 | 最高   | LCD显示刷新             |

---

##  开发环境

- **IDE**: Keil MDK 5.x
- **Compiler**: ARM GCC / ARMCC
- **Framework**: STM32 HAL Library
- **RTOS**: FreeRTOS
- **Logger**: EasyLogger

---

##  注意事项

1. **WiFi配置**：请确保 ESP32-C3 已烧录 AT 固件
2. **API密钥**：需在心知天气官网注册获取免费 API Key
3. **硬件连接**：根据引脚定义正确连接各模块
4. **编译优化**：建议将 Keil 优化等级设为 O0 以便于调试

---

## 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/xxx`)
3. 提交更改 (`git commit -m 'Add xxx'`)
4. 推送到分支 (`git push origin feature/xxx`)
5. 创建 Pull Request

---

## 个人CSDN博客

https://blog.csdn.net/m0_56408226?type=blog

---

## 联系方式

17735813721@163.com




---

<p align="center">Star 本项目 if it helps!</p>

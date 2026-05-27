# MyFreeRTOS — STM32H7 FreeRTOS 脚手架项目

> 一个开箱即用的 STM32H7 FreeRTOS 脚手架工程，集成 Bootloader、双区 OTA 升级、SN 码读写、软件版本管理等功能，并附带 FreeRTOS 核心机制的可运行例程，帮助开发者快速上手嵌入式 RTOS 开发。

---

## 项目简介

本项目旨在提供一个**生产级可用的 FreeRTOS 脚手架**，而非仅仅是一个点灯 Demo。它解决的是嵌入式开发中最常见的两个痛点：

1. **从零搭建工程繁琐** — Bootloader、OTA、Flash 分区、SN 管理……这些基础能力每次都要重写，本项目直接集成
2. **FreeRTOS 学习曲线陡峭** — 信号量怎么用？互斥量为什么重要？事件组解决什么问题？本项目附带完整可运行例程

### 核心特性

| 特性 | 说明 |
|------|------|
| **双区 OTA 升级** | BankA / BankB 交替烧写，升级失败不影响当前运行固件，安全可靠 |
| **Bootloader (Ymodem)** | 独立 IAP 工程，支持串口菜单手动操作和上位机一键烧录 |
| **SN 码管理** | Flash 独立扇区存储，串口命令读写，上位机工具一键操作 |
| **软件版本号** | 语义化版本管理（Major.Minor.Patch），串口命令读取 |
| **看门狗防死机** | 全局 IWDG 喂狗策略，Flash 擦写期间 RAM 运行 + 关中断保护 |
| **上位机工具** | Python Tkinter 工具，一键 OTA 烧录 + 串口调试 + SN 读写 |

### FreeRTOS 例程一览

APP 层内置了 FreeRTOS 核心机制的完整可运行例程（位于 `user_FreeRTOS_Test.c`），默认关闭，将 `FreeRTOS_Test_Init()` 中的 `if(0)` 改为 `if(1)` 即可启用：

| 机制 | 例程内容 | 学习要点 |
|------|----------|----------|
| **消息队列** | 发送任务每 2s 投递消息，接收任务阻塞等待 | `osMessageQueuePut` / `osMessageQueueGet`，生产者-消费者模型 |
| **二值信号量** | 发送任务每 3s 释放一次，接收任务同步响应 | 任务间同步，"发令枪"模式 |
| **计数信号量** | 连续释放 3 次，接收方逐个获取 | 资源池管理，限制并发访问数量 |
| **互斥量** | 两个任务竞争串口输出，观察互斥保护效果 | 共享资源保护，优先级继承防反转 |
| **事件组** | 等待 BIT_0 和 BIT_1 同时满足才执行 | 多条件同步，`osFlagsWaitAll` 逻辑与等待 |
| **软件定时器** | 周期定时器每秒报数 + 单次定时器 5 秒后触发 | `osTimerPeriodic` / `osTimerOnce`，定时器服务任务 |
| **CPU 利用率统计** | 每 5 秒打印各任务运行时间和 CPU 占比 | `vTaskGetRunTimeStats()`，性能调优必备 |

> 例程代码配有详细中文注释，解释了每个 API 的行为和设计意图，适合作为 FreeRTOS 入门学习材料。

---

## 目录

- [项目架构](#项目架构)
- [Flash 分区布局](#flash-分区布局)
- [文件夹结构](#文件夹结构)
- [编译步骤](#编译步骤)
- [初次烧写 APP](#初次烧写-app)
- [OTA 升级步骤](#ota-升级步骤)
- [SN 码读写](#sn-码读写)
- [软件版本号读取](#软件版本号读取)
- [Bootloader 串口菜单](#bootloader-串口菜单)
- [上位机工具](#上位机工具)

---

## 项目架构

```
┌─────────────────────────────────────────────────┐
│                   STM32H723ZGTx                  │
├──────────┬──────────┬──────┬──────────┬─────────┤
│ Bootloader│ BootInfo │  SN  │  BankA   │  BankB  │
│  128KB    │  128B    │ 128KB│  256KB   │  256KB  │
│ 0x08000000│0x08020000│      │0x08060000│0x080A0000│
└──────────┴──────────┴──────┴──────────┴─────────┘
     MyIAP              ↑         MyAPP (BankA/B)
                        │
                   共享 Flash 分区
```

- **MyIAP** — Bootloader 工程，负责固件升级（Ymodem 协议）和 APP 跳转
- **MyAPP** — 应用层工程，基于 FreeRTOS，包含业务逻辑、SN 管理等

启动流程：

```
上电 → Bootloader 启动
        ├─ 检测 RTC 备份寄存器 (0x5A5A) → APP 触发的 OTA 请求 → 进入 IAP 菜单
        ├─ 串口等待 5 秒，检测 'U' 按键 → 手动进入 IAP 菜单
        └─ 读取 BootInfo → 跳转到 Active Bank 的 APP
```

---

## Flash 分区布局

| 扇区 | 起始地址 | 大小 | 用途 |
|------|----------|------|------|
| Sector 0 | `0x08000000` | 128 KB | **Bootloader** (MyIAP) |
| Sector 1 | `0x08020000` | 128 KB | **BootInfo** (启动标志，仅用前 32 字节) |
| Sector 2 | `0x08040000` | 128 KB | **SN 码** (最大 512 字节) |
| Sector 3-4 | `0x08060000` | 256 KB | **BankA** (APP 固件区 A) |
| Sector 5-6 | `0x080A0000` | 256 KB | **BankB** (APP 固件区 B) |
| Sector 7 | `0x080E0000` | 128 KB | 保留 |

**BootInfo 结构**（存储于 Sector 1）：

```c
typedef struct {
    uint32_t magic;              // 魔数 0x424F4F54 ("BOOT")
    BootState_t boot_state;      // 当前激活区: BANKA / BANKB / NONE
    uint32_t vector_table_offset;// 中断向量表偏移地址
} BootInfo_t;
```

---

## 文件夹结构

```
MyFreeRTOS/
├── MyIAP/                          # Bootloader 工程
│   ├── Core/
│   │   ├── Inc/                    # HAL 头文件 (main.h, usart.h, ...)
│   │   └── Src/                    # HAL 源文件 (main.c, usart.c, ...)
│   ├── Ymodem/                     # IAP 核心模块
│   │   ├── common.c/h             # 串口收发工具函数
│   │   ├── flash_if.c/h           # Flash 操作 & 双区启动管理
│   │   ├── menu.c/h               # IAP 串口菜单
│   │   └── ymodem.c/h             # Ymodem 协议实现
│   ├── Drivers/                    # STM32 HAL 驱动
│   ├── STM32H723ZGTX_FLASH.ld     # 链接脚本 (Flash: 0x08000000, 1024KB)
│   └── MyIAP.ioc                   # STM32CubeMX 配置文件
│
├── MyAPP/                          # 应用层工程 (FreeRTOS)
│   ├── Core/
│   │   ├── Inc/                    # HAL 头文件 + FreeRTOSConfig.h
│   │   └── Src/                    # HAL 源文件 + freertos.c
│   ├── User/
│   │   ├── Common/
│   │   │   ├── Inc/
│   │   │   │   ├── sn_manager.h   # SN 码管理接口
│   │   │   │   ├── uart_console.h # 串口控制台接口
│   │   │   │   └── Define.h       # 公共定义
│   │   │   └── Src/
│   │   │       ├── sn_manager.c   # SN 码读写实现
│   │   │       └── uart_console.c # 串口收发队列实现
│   │   ├── Tasks/
│   │   │   ├── Inc/               # 各任务头文件
│   │   │   └── Src/               # 各任务实现
│   │   │       ├── user_ConsoleRecvTask.c  # 串口命令解析 (OTA/SN/VERSION)
│   │   │       ├── user_HardwareInitTask.c # 硬件初始化任务
│   │   │       ├── user_KeyTask.c          # 按键任务
│   │   │       └── ...
│   │   └── version.h              # 软件版本号定义
│   ├── Middlewares/                # FreeRTOS 源码
│   ├── Drivers/                    # STM32 HAL 驱动
│   ├── STM32H723ZGTX_FLASH_BANKA.ld  # BankA 链接脚本 (Flash: 0x08060000, 256KB)
│   ├── STM32H723ZGTX_FLASH_BANKB.ld  # BankB 链接脚本 (Flash: 0x080A0000, 256KB)
│   └── MyAPP.ioc                   # STM32CubeMX 配置文件
│
└── main_tool.py                    # 上位机工具 (OTA 烧录 + 调试)
```

---

## 编译步骤

### 前置条件

- **STM32CubeIDE** (推荐 1.15+)
- **arm-none-eabi-gcc** 工具链 (STM32CubeIDE 自带)

### 编译 Bootloader (MyIAP)

1. 打开 STM32CubeIDE → `File` → `Open Projects from File System` → 选择 `MyIAP` 文件夹
2. 选择 **Debug** 或 **Release** 配置
3. 右键项目 → `Build Project`
4. 产物：`MyIAP/Debug/MyIAP.elf` / `MyIAP.bin`

### 编译 APP (MyAPP)

MyAPP 包含两个构建配置，分别对应两个 Flash Bank：

| 配置名 | 链接脚本 | Flash 起始 | 产物名 | 用途 |
|--------|----------|-----------|--------|------|
| **BankA** | `STM32H723ZGTX_FLASH_BANKA.ld` | `0x08060000` | `MyAPP_A.bin` | 烧写到 BankA |
| **BankB** | `STM32H723ZGTX_FLASH_BANKB.ld` | `0x080A0000` | `MyAPP_B.bin` | 烧写到 BankB |

1. 打开 STM32CubeIDE → 导入 `MyAPP` 项目
2. 右键项目 → `Build Configurations` → `Set Active` → 选择 `BankA` 或 `BankB`
3. 右键项目 → `Build Project`
4. 产物：`MyAPP/BankA/MyAPP_A.bin` 或 `MyAPP/BankB/MyAPP_B.bin`

> **OTA 注意**：升级时需要烧写到 **非活跃 Bank**。例如当前运行 BankA，则应编译 BankB 配置并烧写。

---

## 初次烧写 APP

首次将固件烧写到空白芯片，需先烧录 Bootloader，再通过 Bootloader 菜单烧录 APP。

### 步骤 1：烧录 Bootloader

1. 使用 ST-Link 连接目标板
2. 在 STM32CubeIDE 中打开 MyIAP 项目
3. 右键 → `Run As` → `STM32 C/C++ Application`（或使用 STM32CubeProgrammer）
4. 将 `MyIAP.bin` 烧写到 `0x08000000`

### 步骤 2：通过 Bootloader 烧录 APP

1. 复位开发板，串口终端（115200, 8N1）会显示：
   ```
   [BOOT] Press 'U' within 3 seconds to force enter IAP mode...
   ```
2. 在 5 秒内发送字符 `U`，进入 Bootloader 菜单
3. 输入 `1` → 选择 Download（Ymodem 下载）
4. Bootloader 会自动识别目标 Bank（首次为 BankA）
5. 使用串口工具的 **Ymodem Send** 功能发送 `MyAPP_A.bin`
6. 烧录完成后输入 `3` → 执行 APP 跳转

> **推荐**：使用项目自带的 `main_tool.py` 上位机工具，选择 **Boot 模式** 可一键完成。

---

## OTA 升级步骤

OTA 升级在 APP 运行期间触发，设备自动重启进入 Bootloader 完成固件更新。

### 方式一：串口命令触发

1. APP 运行期间，通过串口发送命令：
   ```
   OTA
   ```
2. APP 向 RTC 备份寄存器写入标志 `0x5A5A`，然后执行软复位
3. Bootloader 检测到标志，自动进入 IAP 菜单
4. 在菜单中选择 `1` → Ymodem 发送目标 Bank 的 `.bin` 文件
5. 烧录成功后选择 `3` → 跳转到新固件

### 方式二：上位机工具一键 OTA

1. 运行 `main_tool.py`：
   ```bash
   pip install pyserial ymodem
   python main_tool.py
   ```
2. 在顶部选择串口和波特率（115200）
3. 切换到 **OTA 固件烧录** 标签页
4. 选择固件所在文件夹（需包含 `_A.bin` 或 `_B.bin` 后缀的文件）
5. 选择 **OTA 模式**
6. 点击 **智能匹配并烧录**，工具自动完成：
   - 发送 `OTA` 命令触发重启
   - 等待进入 Bootloader 菜单
   - 查询当前活跃 Bank
   - 自动选择非活跃 Bank 对应的固件
   - Ymodem 传输固件
   - 烧录完成后自动跳转

### OTA 升级流程图

```
APP 运行中 ──发送 "OTA"──→ 写 RTC 标志 → 软复位
                                        ↓
Bootloader 启动 ──检测 RTC 标志──→ 进入 IAP 菜单
                                        ↓
                              查询 Active Bank
                                  ↓
                     选择 Inactive Bank 作为烧写目标
                                  ↓
                        Ymodem 接收新固件
                                  ↓
                     擦除 Inactive Bank → 写入固件
                                  ↓
                      更新 BootInfo → 切换 Active Bank
                                  ↓
                         跳转到新固件运行
```

---

## SN 码读写

SN 码存储于 Flash Sector 2（`0x08040000`），最大长度 512 字节。

### 串口命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `SN=<value>` | 写入 SN 码 | `SN=ABC123456789` |
| `SNREAD` | 读取 SN 码 | `SNREAD` |

### 读取 SN 码

串口发送：
```
SNREAD
```
返回：
```
[APP] SN: ABC123456789
```
若 SN 为空则返回：
```
[APP] SN: [Empty]
```

### 写入 SN 码

串口发送：
```
SN=ABC123456789
```
返回：
```
[APP] SN Write Success: ABC123456789
```

### 使用上位机工具

1. 运行 `main_tool.py`
2. 切换到 **调试与指令发送** 标签页
3. 点击 **打开调试串口**
4. 使用 **读取 SN 码** 按钮或输入 SN 后点击 **写入 SN 码**

---

## 软件版本号读取

版本号定义在 `MyAPP/User/version.h` 中：

```c
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 2
```

### 串口命令

```
VERSIONREAD
```

返回：
```
[APP] Version: 0.0.2
```

### 修改版本号

编辑 `MyAPP/User/version.h` 中的宏定义，重新编译即可：

| 宏 | 含义 |
|----|------|
| `VERSION_MAJOR` | 主版本号（重大架构变更） |
| `VERSION_MINOR` | 次版本号（硬件变更） |
| `VERSION_PATCH` | 补丁号（软件变更） |

---

## Bootloader 串口菜单

进入 IAP 模式后，串口终端（115200, 8N1）显示如下菜单：

```
=================== Main Menu ============================

  Download image to inactive bank (OTA) --------------- 1

  Upload image from active bank ------------------------ 2

  Execute the loaded application ----------------------- 3

  Enable the write protection -------------------------- 4

  Show boot info -------------------------------------- 5

  Query active bank ----------------------------------- 6

==========================================================
```

| 选项 | 功能 |
|------|------|
| `1` | Ymodem 下载固件到非活跃 Bank |
| `2` | Ymodem 上传当前活跃 Bank 的固件 |
| `3` | 跳转到已加载的 APP |
| `4` | 开启/关闭 Flash 写保护 |
| `5` | 显示 BootInfo（magic、boot_state、vector_table_offset） |
| `6` | 查询当前活跃 Bank |

---

## 上位机工具

`main_tool.py` 是基于 Tkinter 的综合调试与烧录工作站。

### 安装依赖

```bash
pip install pyserial ymodem
```

### 启动

```bash
python main_tool.py
```

### 功能

| 标签页 | 功能 |
|--------|------|
| **OTA 固件烧录** | 自动识别 Bank、Ymodem 传输、一键 OTA 或首次烧录 |
| **调试与指令发送** | 串口收发、SN 码读写、自定义命令发送 |

### OTA 烧录模式

| 模式 | 适用场景 |
|------|----------|
| **OTA 模式** | APP 正在运行，发送 `OTA` 命令触发跳转到 Bootloader |
| **Boot 模式** | 首次下载 / 设备已停留在 Boot 菜单 / 发送 `U` 拦截启动 |

# 最丐34键办公键盘 — CH552G 固件

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

基于 **CH552G** 单片机的超低成本的 34 键办公键盘固件，面向数字输入、办公快捷键和多媒体控制场景。

## 硬件

- **开源硬件地址**: [OSHWHub - 最丐34键办公键盘](https://oshwhub.com/patrickshih/zui-gai-34-jian-ban-gong-jian-pan-_2022-11-10_22-19-39)
- **原理图**: [SCH_最丐34键办公键盘V1.0_2026-03-18.pdf](SCH_最丐34键办公键盘V1.0_2026-03-18.pdf)

### 硬件规格

| 项目 | 说明 |
|------|------|
| **MCU** | CH552G（增强型 E8051 内核，16MHz） |
| **按键布局** | 34键 + EC11旋转编码器 |
| **扫描方式** | 7×5 行列查理复用扫描（7根IO扫描线） |
| **编码器** | EC11 旋转编码器（音量调节） |
| **指示灯** | P3.4 驱动 USB 状态 LED（低电平点亮） |
| **USB** | 全速 USB 2.0 HID 设备，即插即用 |

### 引脚定义

| 引脚 | 功能 |
|------|------|
| P3.0 | K1 - 矩阵扫描线1 |
| P3.1 | K2 - 矩阵扫描线2 |
| P3.3 | K3 - 矩阵扫描线3 |
| P1.1 | K4 - 矩阵扫描线4 |
| P1.5 | K5 - 矩阵扫描线5 |
| P1.6 | K6 - 矩阵扫描线6 |
| P1.7 | K7 - 矩阵扫描线7 |
| P3.2 | EC11_A - 旋转编码器 A 相 |
| P1.4 | EC11_B - 旋转编码器 B 相 |
| P3.4 | LED_STATUS - 状态指示灯（低电平点亮） |

## 功能特性

- **34键紧凑布局** — 数字小键盘 + 方向键 + Tab/Home/End 等办公常用键
- **EC11旋转编码器** — 顺时针/逆时针旋转调节系统音量
- **即插即用** — 标准 USB HID 键盘，Windows/macOS/Linux 免驱
- **组合键支持** — Ctrl+C/X/V（复制/剪切/粘贴）、Alt+Tab（切换窗口）
- **多媒体控制** — 播放/暂停（Play/Pause）
- **长按连续发送** — Delete、方向键长按自动连续触发
- **按键重复计数** — 如 00 键一次按下输出两个 0
- **USB 状态指示** — LED 闪烁（未连接）/ 常亮（已连接）

## 按键布局

```
[Esc] [Del]  [X]  [C]  [V]  [NumLk]  [/?]  [*]  [-]  [Tab]
[Home] [PgUp] [7]  [8]  [9]  [+]  [FwdDel] [End] [PgDn]
 [4]   [5]   [6]  [=]  [↑]
 [1]   [2]   [3]  [Enter] [←]  [↓]  [→]
      [0]   [00]  [.]  [▶/❚❚]
          EC11 编码器（音量控制）
```

> 注：[/?] = NumLock 分号键，[*] = NumLock 星号键，依此类推

## 开发环境

### 方式一：Keil C51（推荐）

1. 安装 [Keil C51](https://www.keil.com/c51/) 编译器
2. 安装 WCH CH55x 器件支持包（`CH552EVT/` 或官网下载）
3. 打开 `64keybord.uvproj` 项目文件
4. 编译并生成 hex 文件

### 方式二：VSCode + EIDE 插件

1. 安装 VSCode 和 [EIDE 插件](https://marketplace.visualstudio.com/items?itemName=CL.eide)
2. 打开 `CH552G-keyboard/` 文件夹
3. 在 EIDE 插件中选择目标 `Target 1`，点击编译
4. 编译产物在 `build/Target 1/` 目录

## 固件烧录

使用 **WCHISPTool**（WCH 官方烧录工具）：

1. 下载 [WCHISPTool](https://www.wch.cn/downloads/WCHISPTool_Setup_exe.html)
2. 将键盘通过 USB 连接到电脑（按住 BOOT 按键上电进入烧录模式）
3. 在 WCHISPTool 中选择 CH552G 型号
4. 选择编译生成的 `.hex` 文件
5. 点击下载

## 项目结构

```
CH552G-keyboard/
├── main.c              # 主程序：系统初始化、主循环
├── keybord.c           # 键盘扫描、消抖、HID映射、EC11处理
├── keybord.h           # 键盘相关定义与函数声明
├── div_usb.c           # USB HID驱动（键盘+多媒体报告）
├── div_usb.h           # USB配置（VID/PID、端点定义）
├── STARTUP.A51         # Keil C51 启动代码
├── CH551.H             # CH551/CH552G 寄存器头文件
├── 64keybord.uvproj    # Keil 项目文件
├── 64keybord.uvopt     # Keil 选项配置
├── .eide/              # EIDE 项目配置
├── .vscode/            # VSCode 任务配置
└── .gitignore

CH552EVT/               # CH552/CH554 数据手册和官方文档
CH554EVT/               # CH554 参考示例代码
```

## 固件架构

主循环流程：

```
系统初始化（时钟/GPIO/Timer0/USB）
       ↓
   ┌─────────────────┐
   │  LED 状态控制    │  ← USB未连接时闪烁，连接后常亮
   │  键盘扫描        │  ← 7×5矩阵逐行扫描
   │  按键消抖        │  ← 5状态机（IDLE→消抖→按下→长按→释放）
   │  EC11编码器      │  ← 旋转检测→音量加减
   │  HID报告发送     │  ← 差分更新，减少USB流量
   └─────────────────┘
```

### USB HID 描述符

- **Report ID 1** — 标准键盘输入报告（9字节）：报告ID + 修饰键 + 保留 + 6个键码
- **Report ID 3** — Consumer 多媒体控制报告（4字节）：报告ID + 功能码
- **VID**: 0x3062 | **PID**: 0x4700

## 许可证

本项目基于 [GNU General Public License v3.0](LICENSE) 开源。

## 已知问题修复 (v1.1)

> 以下问题已于 2026-07 修复。

### 修复 #1：长按按键不能连续执行（高置信度）

**根因**：`keybord_trembling()` 每轮都无条件 `memset(HIDKey, 0, 8)` 清零报告，但 `LONG_PRESS` 状态仅在每 50 轮才重新填入键码。USB 差分检测导致主机看到的是"按下→释放→按下→释放"脉冲，而非持续按住信号，OS 的 typematic 键重复机制无法触发。

**修复**：移除全局 `memset`，改为在各状态按需清零（IDLE/DEBOUNCING/RELEASED 时清零，PRESSED/LONG_PRESS 时保持按键码）。长按期间持续将键码保持在 HID 报告中，由 OS typematic 自动处理重复速率。

### 修复 #2：Ctrl+C / Ctrl+V 等组合键不可靠（高置信度）

**根因**：普通按键在 `PRESSED` 状态发送一次后转入 `RELEASED` 状态，而 RELEASED 顶部每轮 `memset(HIDKey, 0, 8)` 立即清零报告。这导致按键仅被按住 **1 个扫描周期（约 1-2ms）** 就在仍物理按住时被释放——形成极短的脉冲。单字符按键偶尔能注册，但 Ctrl+C/Ctrl+V 这类需要修饰键与主键同时被主机识别的组合键极易失败（修饰键或整个事件被漏掉）。

**修复**：RELEASED 状态在按键仍按住（`temp == temp_old`）时**保留 HIDKey**（持续按下，不脉冲），仅在真正释放（`temp == 0`）时才清零发送释放。组合键会持续按住直到松开，修饰键与主键同时稳定存在于报告中，组合键可靠触发。同时数字键等也获得正常的 OS typematic 重复（按住数字键可连续输入）。

### 修复 #3：主循环多余扫描调用

**根因**：`main()` 中 `keybord_scanning()` 和 `keybord_trembling()` 各调用一次，但后者内部已包含扫描。多余的调用引入额外 GPIO 切换噪声且返回值被丢弃。

**修复**：删除主循环中多余的 `keybord_scanning()` 调用。

### 修复 #4：GPIO 初始化不完整

**根因**：`gpio_init()` 仅配置了 LED 引脚（P3.4），7 根矩阵扫描线和 2 根 EC11 编码器引脚依赖上电默认状态，可能不稳定导致上电误触发。

**修复**：显式配置所有矩阵引脚（P3.0/P3.1/P3.3/P1.1/P1.5/P1.6/P1.7）为准双向口并使能上拉，EC11 引脚（P3.2/P1.4）为输入模式并使能上拉，并添加 20 轮扫描预热使滤波器缓冲区稳定。

### 修复 #5：EC11 编码器旋转一次后卡死

**根因**：`ec11_handler()` 中 `last_state = current_state` 仅在转换被识别为有效旋转（CW/CCW）时才更新。EC11 机械触点抖动会产生短暂的**无效转换**（如两相同时跳变 `00->11`，transition `0x03`），既不匹配 CW 也不匹配 CCW 分支。一旦检测到无效转换，`last_state` 不更新，导致 `current_state != last_state` 持续成立，debounce 反复累加并重复处理同一个无效转换，**状态机永久卡死**--表现为旋转编码器音量只触发一次后不再响应。

**修复**：将 `last_state = current_state` 移到计算 `transition` 之后、方向判断 if-else 之前，**无条件更新**（无论本次转换是否为有效旋转）。这样保证持续跟踪编码器实际位置，遇到抖动产生的无效转换时最多漏掉一个事件，但绝不会卡死。

### 修复 #6：USB EP2 发送等待逻辑失效（xact error）

**根因**：`drv_usb_write_ep2()` 中 `while (UEP2_CTRL & MASK_UEP_T_RES == UEP_T_RES_ACK)` 存在运算符优先级错误。C 语言 `==` 优先级高于 `&`，实际被解析为 `while (UEP2_CTRL & (MASK_UEP_T_RES == UEP_T_RES_ACK))`，即 `while (UEP2_CTRL & 0)` => `while (0)`，等待逻辑完全失效。

本意是"EP2 仍处于 ACK（上一包发送中）状态时等待"，失效后若键盘报告与消费者报告在同一 USB 帧内连续发送，会覆盖正在传输的 `buf_ep2` 缓冲区，主机收到 CRC 错误的包，表现为 Bus Hound 抓包中的 `USTS c0000011 xact error`。

**修复**：加括号修正为 `while ((UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK)`。

**验证**：Bus Hound 抓包确认 USB 枚举完全成功--设备描述符（VID=0x3062, PID=0x4700）、配置描述符（HID 类, EP2 IN 中断 64B/1ms）、90 字节报告描述符（ReportID=1 键盘 + ReportID=3 消费者控制）、SET_REPORT LED 报告（`01 01` NumLock）、字符串描述符（"CH552G" / "Keyboard"）全部正确收发。

### 代码清理

- 移除 `TEMP_Byte` / `Bit0-Bit7` 死代码（12 行永久不执行的残留逻辑）
- 移除主循环中 `state == KEY_STATE_RELEASED` 的冗余判断
- 移除未调用的测试函数 `send_H()`、`sendHello()`、`drv_usb_dial()`（消除 Keil L16 "UNCALLED SEGMENT" 警告，精简 code 体积约 200+ 行）
- 加固 `PRESSED` 状态守卫：`temp <= 35` → `temp > 0 && temp <= 35`，防止 `temp=0` 时 `key_code[temp-1]` 数组越界读取随机键码

## 致谢

- [WCH 沁恒微电子](https://www.wch.cn) — CH552/CH554 芯片及官方资料
- 参考例程基于 WCH CH554 评估板示例代码（`CH554EVT/`）
- 源项目 [最丐17Toutch机械数字键盘](https://oshwhub.com/yANgZEN/zui-gai-shuo-zi-jian-pan) — 本固件对应的硬件设计最初源于此项目
- [画板当吃生菜](https://oshwhub.com/yangzen/works) — 感谢原始作者的硬件开源工作

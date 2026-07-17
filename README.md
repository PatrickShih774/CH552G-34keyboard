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
| **矩阵** | 7×5 行列扫描（7根IO扫描线） |
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

## 致谢

- [WCH 沁恒微电子](https://www.wch.cn) — CH552/CH554 芯片及官方资料
- 参考例程基于 WCH CH554 评估板示例代码（`CH554EVT/`）
- 源项目 [最丐数字键盘](https://oshwhub.com/yANgZEN/zui-gai-shuo-zi-jian-pan) — 本固件对应的硬件设计最初源于此项目
- [yANgZEN](https://oshwhub.com/yangzen/works) — 感谢原始作者的硬件开源工作

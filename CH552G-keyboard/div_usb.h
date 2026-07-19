#ifndef __USB_H__
#define __USB_H__

#include <CH551.H>
#include <string.h>

#define uchar unsigned char
#define uint unsigned int

#define HIBYTE(c) ((c >> 8) & 0xff)
#define LOBYTE(c) ((c) & 0xff)

#define USB_VID    0X3062
#define USB_PID    0X4700
#define VERSION    0X0100

#define EP0_SIZE 64
#define EP2_SIZE 64

/* ========== USB HID 报告结构体 ========== */

/** @brief 键盘输入报告（USB 线缆格式，9 字节） */
typedef struct {
    uchar report_id;      /**< Report ID = 1 */
    uchar modifier;       /**< 修饰键位图（Ctrl/Shift/Alt/Win） */
    uchar reserved;       /**< 保留字节，恒为 0 */
    uchar keycodes[6];    /**< 最多 6 个同时按下的键码 */
} keyboard_report_t;

/** @brief Consumer 多媒体控制报告（USB 线缆格式，3 字节） */
typedef struct {
    uchar report_id;      /**< Report ID = 3 */
    uchar usage_lo;       /**< Consumer usage 低字节 */
    uchar usage_hi;       /**< Consumer usage 高字节 */
} consumer_report_t;

/** @brief 初始化 USB 外设（全速设备模式，HID 端点配置） */
void drv_usb_init(void);
/** @brief 发送键盘 HID 报告（9 字节含 ReportID，差分检测） */
uchar drv_usb_keyboard(uchar *p);
/** @brief 发送 Consumer 多媒体报告（3 字节，force=1 跳过差分检测） */
uchar drv_usb_mul(uchar dat, uchar force);

// USB连接状态变量（定义在div_usb.c中）
extern uchar idata Ready;

#endif
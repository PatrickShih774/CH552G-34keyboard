#ifndef _KEYBORD_h_
#define _KEYBORD_h_

#include <CH551.H>
#include <intrins.h>
#include "div_usb.h"

#define uchar unsigned char
#define uint  unsigned int

#define num5  (CLOCK_CFG & bRST)
sbit K1 = P3 ^ 0;
sbit K2 = P3 ^ 1;
sbit K3 = P3 ^ 3;
sbit K4 = P1 ^ 1;
sbit K5 = P1 ^ 5;
sbit K6 = P1 ^ 6;
sbit K7 = P1 ^ 7;

// EC11旋转编码器定义
sbit EC11_A = P3 ^ 2; // 编码器A相
sbit EC11_B = P1 ^ 4; // 编码器B相

// LED指示灯定义（P3.4低电平有效）
sbit LED_STATUS = P3 ^ 4; // 状态指示灯

// LED控制宏（低电平点亮）
#define LED_ON()                 LED_STATUS = 0
#define LED_OFF()                LED_STATUS = 1
#define LED_TOGGLE()             LED_STATUS = ~LED_STATUS

#define HID_KEYBOARD_RESERVED    0
#define HID_KEYBOARD_A           4
#define HID_KEYBOARD_B           5
#define HID_KEYBOARD_C           6
#define HID_KEYBOARD_D           7
#define HID_KEYBOARD_E           8
#define HID_KEYBOARD_F           9
#define HID_KEYBOARD_G           10
#define HID_KEYBOARD_H           11
#define HID_KEYBOARD_I           12
#define HID_KEYBOARD_J           13
#define HID_KEYBOARD_K           14
#define HID_KEYBOARD_L           15
#define HID_KEYBOARD_M           16
#define HID_KEYBOARD_N           17
#define HID_KEYBOARD_O           18
#define HID_KEYBOARD_P           19
#define HID_KEYBOARD_Q           20
#define HID_KEYBOARD_R           21
#define HID_KEYBOARD_S           22
#define HID_KEYBOARD_T           23
#define HID_KEYBOARD_U           24
#define HID_KEYBOARD_V           25
#define HID_KEYBOARD_W           26
#define HID_KEYBOARD_X           27
#define HID_KEYBOARD_Y           28
#define HID_KEYBOARD_Z           29
#define HID_KEYBOARD_1           30
#define HID_KEYBOARD_2           31
#define HID_KEYBOARD_3           32
#define HID_KEYBOARD_4           33
#define HID_KEYBOARD_5           34
#define HID_KEYBOARD_6           35
#define HID_KEYBOARD_7           36
#define HID_KEYBOARD_8           37
#define HID_KEYBOARD_9           38
#define HID_KEYBOARD_0           39
#define HID_KEYBOARD_RETURN      40
#define HID_KEYBOARD_ESCAPE      41
#define HID_KEYBOARD_DELETE      42
#define HID_KEYBOARD_TAB         43
#define HID_KEYBOARD_SPACEBAR    44
#define HID_KEYBOARD_MINUS       45
#define HID_KEYBOARD_EQUAL       46
#define HID_KEYBOARD_LEFT_BRKT   47
#define HID_KEYBOARD_RIGHT_BRKT  48
#define HID_KEYBOARD_BACK_SLASH  49
#define HID_KEYBOARD_SEMI_COLON  51
#define HID_KEYBOARD_SGL_QUOTE   52
#define HID_KEYBOARD_GRV_ACCENT  53
#define HID_KEYBOARD_COMMA       54
#define HID_KEYBOARD_DOT         55
#define HID_KEYBOARD_FWD_SLASH   56
#define HID_KEYBOARD_CAPS_LOCK   57
#define HID_KEYBOARD_F1          58
#define HID_KEYBOARD_F2          59
#define HID_KEYBOARD_F3          60
#define HID_KEYBOARD_F4          61
#define HID_KEYBOARD_F5          62
#define HID_KEYBOARD_F6          63
#define HID_KEYBOARD_F7          64
#define HID_KEYBOARD_F8          65
#define HID_KEYBOARD_F9          66
#define HID_KEYBOARD_F10         67
#define HID_KEYBOARD_F11         68
#define HID_KEYBOARD_F12         69
#define HID_KEYBOARD_PRNT_SCREEN 70
#define HID_KEYBOARD_SCROLL_LOCK 71
#define HID_KEYBOARD_PAUSE       72
#define HID_KEYBOARD_INSERT      73
#define HID_KEYBOARD_HOME        74
#define HID_KEYBOARD_PAGE_UP     75
#define HID_KEYBOARD_DELETE_FWD  76
#define HID_KEYBOARD_END         77
#define HID_KEYBOARD_PAGE_DOWN   78
#define HID_KEYBOARD_RIGHT_ARROW 79
#define HID_KEYBOARD_LEFT_ARROW  80
#define HID_KEYBOARD_DOWN_ARROW  81
#define HID_KEYBOARD_UP_ARROW    82
#define HID_KEYBPAD_NUM_LOCK     83
#define HID_KEYBPAD_DIVIDE       84
#define HID_KEYBPAD_MULTIPLY     85
#define HID_KEYBPAD_SUBTRACT     86
#define HID_KEYBPAD_ADD          87
#define HID_KEYBPAD_ENTER        88
#define HID_KEYBPAD_1            89
#define HID_KEYBPAD_2            90
#define HID_KEYBPAD_3            91
#define HID_KEYBPAD_4            92
#define HID_KEYBPAD_5            93
#define HID_KEYBPAD_6            94
#define HID_KEYBPAD_7            95
#define HID_KEYBPAD_8            96
#define HID_KEYBPAD_9            97
#define HID_KEYBPAD_0            98
#define HID_KEYBPAD_DOT          99
#define HID_KEYBOARD_MENU        101

#define HID_L_WIN                0X08
#define HID_L_ALT                0X04
#define HID_L_SHIFT              0X02
#define HID_L_CTL                0X01
#define HID_R_WIN                0X80
#define HID_R_ALT                0X40
#define HID_R_SHIFT              0X20
#define HID_R_CTL                0X10

// 多媒体按键定义（Consumer Page）
#define HID_CONSUMER_VOLUME_UP     233 // 0xE9 - Volume Increment
#define HID_CONSUMER_VOLUME_DOWN   234 // 0xEA - Volume Decrement
#define HID_CONSUMER_BRIGH_UP      111 // 0x6F - Brightness Increment
#define HID_CONSUMER_BRIGH_DOWN    112 // 0x70 - Brightness Decrement

#define HID_CONSUMER_SCAN_NEXT_TRK 181 // 0xB5 - Scan Next Track
#define HID_CONSUMER_SCAN_PREV_TRK 182 // 0xB6 - Scan Previous Track
#define HID_CONSUMER_STOP          183 // 0xB7 - Stop
#define HID_CONSUMER_PLAY_PAUSE    205 // 0xCD - Play/Pause

#define HID_CONSUMER_CALCULATOR    146 // 0x92 - Calculator
#define HID_CONSUMER_FAST_FORWARD  179 // 0xB3 - Fast Forward
#define HID_CONSUMER_REWIND        180 // 0xB4 - Rewind
#define HID_CONSUMER_MUTE          226 // 0xE2 - Mute

#define HID_CONSUMER_CHROME        35  // 0x23 - Chrome
#define HID_CONSUMER_MULTI         131 // 0x83 - Multi

extern uchar HIDKey[8];
/** @brief 多媒体按键值（0 表示无，非 0 表示 consumer usage 码） */
extern uchar MULKey[1];

/* ========== 按键映射表结构体类型 ========== */

/** @brief 修饰键映射表条目：按键码 + 修饰键 */
typedef struct {
    uchar key_code;   /**< HID 按键码 */
    uchar modifier;   /**< 修饰键位图（HID_L_CTL 等） */
} key_modifier_entry_t;

/** @brief 重复次数映射表条目：key_code[] 索引 + 发送次数 */
typedef struct {
    uchar index;      /**< key_code[] 数组索引 */
    uchar count;      /**< 连续发送次数 */
} key_repeat_entry_t;

/** @brief 纯修饰键映射表条目：key_code[] 索引 + 修饰键 */
typedef struct {
    uchar index;      /**< key_code[] 数组索引 */
    uchar modifier;   /**< 修饰键位图 */
} key_modifier_only_entry_t;

/** @brief 长按支持映射表条目：key_code[] 索引 + 是否支持 */
typedef struct {
    uchar index;      /**< key_code[] 数组索引 */
    uchar support;    /**< 1=支持长按连续发送 */
} key_long_press_entry_t;

/** @brief 7×? 矩阵扫描，返回按键索引（1~35，0=无） */
uchar keybord_scanning(void);
/** @brief 按键消抖状态机（5 状态），返回 0 */
uchar keybord_trembling(void);
/** @brief 发送键盘 HID 报告（差分检测），返回 1=已发送 */
uchar HIDKey_transfer(void);
/** @brief 发送多媒体 HID 报告（差分检测），返回 1=已发送 */
uchar MULKey_transfer(void);
/** @brief 获取按键码对应的修饰键（Ctrl/Alt 等），0=无修饰键 */
uchar get_key_modifier(uchar key_code_val);
/** @brief 获取按键索引对应的重复次数（如 00 键发 2 次） */
uchar get_key_repeat_count(uchar key_index);
/** @brief 获取按键索引对应的纯修饰键（无键码），0=不是纯修饰键 */
uchar get_modifier_only_key(uchar key_index);
/** @brief 检查按键索引是否支持长按连续发送，1=支持 */
uchar is_long_press_key(uchar key_index);
/** @brief 检查按键码是否为 Consumer Page 多媒体键，1=是 */
uchar is_multimedia_key(uchar key_code_val);
/** @brief EC11 旋转编码器处理（音量控制） */
void ec11_handler(void);

#endif
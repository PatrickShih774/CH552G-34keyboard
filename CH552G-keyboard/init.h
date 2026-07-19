#ifndef __INIT_H__
#define __INIT_H__

#include "keybord.h"

/** @brief Timer0 溢出计数器（供 delay_ms 做忙等待时基） */
extern volatile uchar t0_num;

/** @brief 系统时钟与安全模式初始化（16MHz，USB 唤醒） */
void system_init(void);

/** @brief GPIO 引脚初始化（矩阵扫描、EC11、LED） */
void gpio_init(void);

/** @brief 定时器 T0 初始化（模式 2，8 位自动重装，约 187.5µs/溢出） */
void T0_init(void);

/**
 * @brief 基于 Timer0 的毫秒级延时，同时喂狗
 * @param ms 延时毫秒数（实际约 0.75ms/ms）
 */
void delay_ms(uchar ms);

/**
 * @brief LED 状态控制
 * - USB 未连接：快闪（约 100ms 周期）
 * - USB 已连接：常亮
 */
void led_handler(void);

#endif
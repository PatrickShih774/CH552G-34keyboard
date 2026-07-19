#include "div_usb.h"
#include "keybord.h"
#include "init.h"
#include <CH551.H>

/**
 * @brief 主函数
 *
 * 初始化流程：系统时钟 → GPIO → Timer0 → USB（全局中断使能）。
 * 然后执行 20 轮 GPIO 预热扫描以稳定引脚电平。
 *
 * 主循环每轮执行：
 * 1. LED 状态控制（USB 连接指示）
 * 2. 键盘矩阵扫描与消抖状态机
 * 3. EC11 旋转编码器处理
 * 4. USB HID 报告发送（仅枚举完成后）
 * 5. 延时约 1ms 以控制循环速率
 */
void main()
{
    uchar i;

    system_init();
    gpio_init();
    T0_init();

    EA = 1;
    drv_usb_init();

    // GPIO 和滤波器预热：运行若干轮扫描以稳定引脚电平
    for (i = 0; i < 20; i++)
    {
        keybord_scanning();
    }

    while (1)
    {
        led_handler();           // LED状态控制
        keybord_trembling();     // 键盘消抖与按键处理
        ec11_handler();          // EC11旋转编码器处理

        // 仅在 USB 枚举完成后发送 EP2 报告，避免主机未轮询 EP2 时
        // drv_usb_write_ep2 的等待逻辑死锁主循环。
        if (Ready)
        {
            if (!HIDKey_transfer())  // 发送键盘报告
                MULKey_transfer();   // 发送多媒体报告（错开更新）
        }

        delay_ms(1);
    }
}
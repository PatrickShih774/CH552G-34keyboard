#include "div_usb.h"
#include "keybord.h"
#include <CH551.H>

// LED状态变量
static uint led_flash_timer = 0;  // LED闪烁计时器

void system_init()
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = 0x8C;
    WAKE_CTRL = bWAK_BY_USB;
    GLOBAL_CFG |= 1;
    SAFE_MOD = 0x00;
}

void gpio_init()
{
    // LED引脚初始化
    // P3.4设置为推挽输出
    P3_MOD_OC &= ~(1 << 4);

    // 初始熄灭LED
    LED_OFF();
}

void T0_init()
{
    TMOD &= 0xF0;
    TMOD |= 0x02;
    TL0 = 0x06;
    TH0 = 0x06;
    TF0 = 0;
    TR0 = 1;
    ET0 = 1;
}

volatile uchar t0_num = 0;

void delay_ms(uchar ms)
{
    WDOG_COUNT = 0xF0 - ms / 5;
    for (; ms > 0; ms--)
    {
        while (t0_num < 4)
            ;
        t0_num -= 4;
    }
}

/**
 * @brief LED状态控制函数
 *
 * 根据USB连接状态控制LED显示：
 * - USB未连接：快闪（约100ms周期）
 * - USB已连接：常亮
 */
void led_handler(void)
{
    if (Ready)
    {
        // USB已连接，LED常亮
        LED_ON();
    }
    else
    {
        // USB未连接，LED快闪（约100ms周期）
        led_flash_timer++;
        if (led_flash_timer >= 100)
        {
            led_flash_timer = 0;
            LED_TOGGLE();
        }
    }
}

void main()
{
    system_init();
    gpio_init();
    T0_init();

    EA = 1;
    drv_usb_init();

    while (1)
    {
        led_handler();  // LED状态控制

        keybord_scanning();
        keybord_trembling();
        ec11_handler();  // EC11旋转编码器处理
       // HIDKey_transfer();
        if(!HIDKey_transfer())//确保不同时更新
			MULKey_transfer();
        delay_ms(1);
    }
}

void T0_time() interrupt INT_NO_TMR0
{
    TF0 = 0;
    t0_num++;
}

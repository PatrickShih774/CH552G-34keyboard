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
    // LED引脚初始化 — P3.4 设为推挽输出
    P3_MOD_OC &= ~(1 << 4);
    LED_OFF();

    // 矩阵扫描引脚初始化 — 配置为准双向口（标准8051模式）
    // 准双向口: MOD_OC=1, DIR_PU=1 → 输出1时弱上拉（可被按键拉低），输出0时强驱动
    // 这是键盘矩阵扫描的正确配置，避免推挽强输出烧毁引脚或干扰扫描
    // P3.0(K1), P3.1(K2), P3.3(K3)
    P3_MOD_OC |= ((1 << 0) | (1 << 1) | (1 << 3));
    P3_DIR_PU |= ((1 << 0) | (1 << 1) | (1 << 3));

    // P1.1(K4), P1.5(K5), P1.6(K6), P1.7(K7)
    P1_MOD_OC |= ((1 << 1) | (1 << 5) | (1 << 6) | (1 << 7));
    P1_DIR_PU |= ((1 << 1) | (1 << 5) | (1 << 6) | (1 << 7));

    // EC11 编码器引脚 — 准双向口输入模式，使能上拉
    // P3.2(EC11_A), P1.4(EC11_B)
    P3_MOD_OC |= (1 << 2);
    P3_DIR_PU |= (1 << 2);
    P1_MOD_OC |= (1 << 4);
    P1_DIR_PU |= (1 << 4);

    // 初始化所有矩阵扫描线为高电平
    K1 = 1; K2 = 1; K3 = 1; K4 = 1; K5 = 1; K6 = 1; K7 = 1;
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

        if (!HIDKey_transfer())  // 发送键盘报告
            MULKey_transfer();   // 发送多媒体报告（错开更新）

        delay_ms(1);
    }
}

void T0_time() interrupt INT_NO_TMR0
{
    TF0 = 0;
    t0_num++;
}

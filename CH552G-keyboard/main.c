#include "div_usb.h"
#include "keybord.h"
#include <CH551.H>

// LED状态变量
static uint led_flash_timer = 0;  // LED闪烁计时器

/**
 * @brief 系统时钟与安全模式初始化
 *
 * 配置系统时钟为 16MHz（内部振荡器），使能 USB 唤醒，
 * 设置全局配置。SAFE_MOD 序列为解锁 CH552 安全寄存器。
 */
void system_init()
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = 0x8C;
    WAKE_CTRL = bWAK_BY_USB;
    GLOBAL_CFG |= 1;
    SAFE_MOD = 0x00;
}

/**
 * @brief GPIO 引脚初始化
 *
 * 配置所有 I/O 引脚的工作模式：
 * - P3.4(LED)：推挽输出
 * - P3.0/P3.1/P3.3/P1.1/P1.5/P1.6/P1.7(矩阵扫描)：准双向口，使能上拉
 * - P3.2(EC11_A)/P1.4(EC11_B)：准双向口输入，使能上拉
 * 初始化所有矩阵扫描线为高电平。
 */
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

/**
 * @brief 定时器 T0 初始化（模式 2，8 位自动重装）
 *
 * 配置 Timer0 为 8 位自动重装模式（模式 2），
 * TH0/TL0 = 0x06，每 250 个时钟周期溢出一次，
 * 用于提供毫秒级延时基准（约 187.5µs/溢出）。
 * 使能 Timer0 中断。
 */
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

/**
 * @brief 毫秒级延时
 *
 * 基于 Timer0 溢出计数实现延时。
 * 每个 Timer0 周期约 187.5µs，等待 4 次溢出（约 750µs）为 1 个基本单位。
 * 同时喂狗（WDOG_COUNT 写入），防止看门狗超时复位。
 *
 * @param ms 延时毫秒数（实际每毫秒约 0.75ms，因 Timer0 周期近似）
 */
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

/**
 * @brief 定时器 T0 中断服务函数
 *
 * 每次 Timer0 溢出触发，递增 t0_num 计数器。
 * t0_num 供 delay_ms() 做忙等待的时基。
 */
void T0_time() interrupt INT_NO_TMR0
{
    TF0 = 0;
    t0_num++;
}

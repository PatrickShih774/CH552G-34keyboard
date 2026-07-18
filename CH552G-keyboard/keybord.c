#include "keybord.h"

uchar HIDKey[8] = {0};
uchar MULKey[1] = {0}; // 多媒体按键数组

uchar code key_code[35] =
    {
        HID_KEYBOARD_ESCAPE, HID_KEYBOARD_DELETE, HID_KEYBOARD_X, HID_KEYBOARD_C, HID_KEYBOARD_V, HID_KEYBPAD_NUM_LOCK,
        HID_KEYBPAD_DIVIDE, HID_KEYBPAD_MULTIPLY, HID_KEYBPAD_SUBTRACT, HID_KEYBOARD_TAB, HID_KEYBOARD_HOME,
        HID_KEYBOARD_PAGE_UP, HID_KEYBPAD_7, HID_KEYBPAD_8, HID_KEYBPAD_9, HID_KEYBPAD_ADD, HID_KEYBOARD_DELETE_FWD,
        HID_KEYBOARD_END, HID_KEYBOARD_PAGE_DOWN, HID_KEYBPAD_4, HID_KEYBPAD_5, HID_KEYBPAD_6, HID_KEYBOARD_EQUAL,
        HID_KEYBOARD_UP_ARROW, HID_KEYBPAD_1, HID_KEYBPAD_2, HID_KEYBPAD_3, HID_KEYBPAD_ENTER, HID_KEYBOARD_LEFT_ARROW,
        HID_KEYBOARD_DOWN_ARROW, HID_KEYBOARD_RIGHT_ARROW, HID_KEYBPAD_0, HID_KEYBPAD_0, HID_KEYBPAD_DOT, HID_CONSUMER_PLAY_PAUSE};

/**
 * @brief 按键修饰键映射表
 *
 * 为特定按键添加修饰键（Ctrl/Shift/Alt等），实现组合键功能
 * 格式：{按键码, 修饰键}
 * 修饰键为0表示无修饰键
 *
 * 示例：
 * {HID_KEYBOARD_C, HID_L_CTL}  -> Ctrl+C 复制
 * {HID_KEYBOARD_V, HID_L_CTL}  -> Ctrl+V 粘贴
 */
uchar code key_modifier_map[][2] =
    {
        {HID_KEYBOARD_X, HID_L_CTL},   // Ctrl+X 复制
        {HID_KEYBOARD_C, HID_L_CTL},   // Ctrl+C 复制
        {HID_KEYBOARD_V, HID_L_CTL},   // Ctrl+V 粘贴
        {HID_KEYBOARD_TAB, HID_L_ALT}, // Alt+Tab 切换窗口
        {0, 0}                         // 结束标记
};

/**
 * @brief 连续按键映射表
 *
 * 为特定按键设置连续发送次数
 * 格式：{key_code数组索引, 连续次数}
 * 次数为1表示正常单次按键
 * 使用索引而不是按键码，可以区分key_code中相同的按键
 *
 * 示例：
 * {31, 2}  -> key_code[31]发送两次（两个0）
 * {30, 1}  -> key_code[30]发送一次（一个0）
 */
uchar code key_repeat_map[][2] =
    {
        {30, 1}, // 第一个HID_KEYBPAD_0，发送一次（一个0）
        {31, 2}, // 第二个HID_KEYBPAD_0，发送两次（两个0）
        {0, 0}   // 结束标记
};

/**
 * @brief 纯修饰键映射表
 *
 * 标记哪些按键只发送修饰键，不发送按键码
 * 格式：{key_code数组索引, 修饰键}
 * 修饰键为0表示不是纯修饰键
 *
 * 示例：
 * {34, HID_L_WIN}  -> 只发送Win键（不带按键码）
 * 注意：索引34现在是多媒体按键(HID_CONSUMER_PLAY_PAUSE)，不再使用纯修饰键功能
 */
uchar code key_modifier_only_map[][2] =
    {
        // {34, HID_L_WIN}, // 注释掉：索引34现在是多媒体按键，通过MULKey_transfer发送
        {0, 0} // 结束标记
};

/**
 * @brief 长按连续发送映射表
 *
 * 标记哪些按键支持长按连续发送
 * 格式：{key_code数组索引, 1}
 * 值为1表示支持长按连续发送，0表示不支持
 *
 * 示例：
 * {1, 1}  -> HID_KEYBOARD_DELETE支持长按连续删除
 */
uchar code key_long_press_map[][2] =
    {
        {1, 1},  // DELETE键长按连续删除
        {24, 1}, // 方向键左长按连续左移
        {29, 1}, // 方向键右长按连续右移
        {30, 1}, // 方向键下长按连续下移
        {25, 1}, // 方向键上长按连续上移
        {0, 0}   // 结束标记
};

/**
 * @brief 检查按键是否支持长按连续发送
 *
 * @param key_index key_code数组索引（temp值）
 * @return 1表示支持，0表示不支持
 */
uchar is_long_press_key(uchar key_index)
{
    uchar i = 0;
    while (key_long_press_map[i][0] != 0)
    {
        if (key_long_press_map[i][0] == key_index)
        {
            return key_long_press_map[i][1];
        }
        i++;
    }
    return 0; // 不支持长按
}

/**
 * @brief 获取按键对应的连续发送次数
 *
 * @param key_index key_code数组索引（temp值）
 * @return 连续次数，默认为1
 */
uchar get_key_repeat_count(uchar key_index)
{
    uchar i = 0;
    while (key_repeat_map[i][0] != 0)
    {
        if (key_repeat_map[i][0] == key_index)
        {
            return key_repeat_map[i][1];
        }
        i++;
    }
    return 1; // 默认发送1次
}

/**
 * @brief 获取按键对应的纯修饰键
 *
 * @param key_index key_code数组索引（temp值）
 * @return 修饰键值，不是纯修饰键返回0
 */
uchar get_modifier_only_key(uchar key_index)
{
    uchar i = 0;
    while (key_modifier_only_map[i][0] != 0)
    {
        if (key_modifier_only_map[i][0] == key_index)
        {
            return key_modifier_only_map[i][1];
        }
        i++;
    }
    return 0; // 不是纯修饰键
}

/**
 * @brief 获取按键对应的修饰键
 *
 * @param key_code 按键码
 * @return 修饰键值，无修饰键返回0
 */
uchar get_key_modifier(uchar key_code_val)
{
    uchar i = 0;
    while (key_modifier_map[i][0] != 0)
    {
        if (key_modifier_map[i][0] == key_code_val)
        {
            return key_modifier_map[i][1];
        }
        i++;
    }
    return 0;
}

/**
 * @brief 检查按键是否是多媒体按键（Consumer Page）
 *
 * @param key_code 按键码
 * @return 1表示是多媒体按键，0表示不是
 */
uchar is_multimedia_key(uchar key_code_val)
{
    // 多媒体按键的值范围通常是 0x80 以上（Consumer Page）
    // HID键盘按键范围是 0-127 (0x00-0x7F)
    // 多媒体按键范围是 128-255 (0x80-0xFF)
    if (key_code_val >= 128)
        return 1;
    return 0;
}

void key_delay()
{
    uchar i;
    i = 27;
    while (--i)
        ;
}

uchar keybord_scanning()
{
    static uchar idata temp_old = 0;
    uchar temp = 0;
    uchar num = 0;
    K1 = K2 = K3 = K4 = K5 = K6 = K7 = 1;

    // if (!K1)
    // {
    //     temp = 1;
    //     num++;
    // }
    // if (!K2)
    // {
    //     temp = 2;
    //     num++;
    // }
    // if (!K3)
    // {
    //     temp = 3;
    //     num++;
    // }
    // if (!K4)
    // {
    //     temp = 4;
    //     num++;
    // }
    // if (!K5)
    // {
    //     temp = 5;
    //     num++;
    // }
    // if (!K6)
    // {
    //     temp = 6;
    //     num++;
    // }
    // if (!K7)
    // {
    //     temp = 7;
    //     num++;
    // }
    // if (num)
    //     goto end;

    K1 = 0;
    key_delay();
    if (!K2)
    {
        temp = 1;
        num++;
    }
    if (!K3)
    {
        temp = 2;
        num++;
    }
    if (!K4)
    {
        temp = 3;
        num++;
    }
    if (!K5)
    {
        temp = 4;
        num++;
    }
    if (!K6)
    {
        temp = 5;
        num++;
    }
    if (!K7)
    {
        temp = 6;
        num++;
    }
    K1 = 1;

    K2 = 0;
    key_delay();
    if (!K1)
    {
        temp = 7;
        num++;
    }
    if (!K3)
    {
        temp = 8;
        num++;
    }
    if (!K4)
    {
        temp = 9;
        num++;
    }
    if (!K5)
    {
        temp = 10;
        num++;
    }
    if (!K6)
    {
        temp = 11;
        num++;
    }
    if (!K7)
    {
        temp = 12;
        num++;
    }
    K2 = 1;

    K3 = 0;
    key_delay();
    if (!K1)
    {
        temp = 13;
        num++;
    }
    if (!K2)
    {
        temp = 14;
        num++;
    }
    if (!K4)
    {
        temp = 15;
        num++;
    }
    if (!K5)
    {
        temp = 16;
        num++;
    }
    if (!K6)
    {
        temp = 17;
        num++;
    }
    if (!K7)
    {
        temp = 18;
        num++;
    }
    K3 = 1;

    K4 = 0;
    key_delay();
    if (!K1)
    {
        temp = 19;
        num++;
    }
    if (!K2)
    {
        temp = 20;
        num++;
    }
    if (!K3)
    {
        temp = 21;
        num++;
    }
    if (!K5)
    {
        temp = 22;
        num++;
    }
    if (!K6)
    {
        temp = 23;
        num++;
    }
    if (!K7)
    {
        temp = 24;
        num++;
    }
    K4 = 1;

    K5 = 0;
    key_delay();
    if (!K1)
    {
        temp = 25;
        num++;
    }
    if (!K2)
    {
        temp = 26;
        num++;
    }
    if (!K3)
    {
        temp = 27;
        num++;
    }
    if (!K4)
    {
        temp = 28;
        num++;
    }

    if (!K6)
    {
        temp = 29;
        num++;
    }
    if (!K7)
    {
        temp = 30;
        num++;
    }
    K5 = 1;

    K6 = 0;
    key_delay();
    if (!K1)
    {
        temp = 31;
        num++;
    }
    if (!K2)
    {
        temp = 32;
        num++;
    }
    if (!K3)
    {
        temp = 33;
        num++;
    }
    if (!K4)
    {
        temp = 34;
        num++;
    }
    if (!K5)
    {
        temp = 35;
        num++;
    }

    K6 = 1;
    // end:
    //     if (num <= 1)
    temp_old = temp;
    return temp_old;
}

uchar filter(uchar dat)
{
    static uchar idata tick = 0;
    static uchar idata key_temp[4] = 0;
    static uchar idata key_old = 0;
    key_temp[tick++] = dat;
    tick &= 3;
    if (key_temp[0] == key_temp[1] &&
        key_temp[1] == key_temp[2] &&
        key_temp[2] == key_temp[3])
        key_old = key_temp[0];
    return key_old;
}

/**
 * @brief 键盘消抖和按键处理函数（改进版）
 *
 * 实现稳健的消抖机制，确保：
 * 1. 普通按键每次按下只发送一次HID码
 * 2. 长按按键支持连续发送功能
 * 3. 有效防止机械抖动造成的重复触发
 *
 * 状态机设计：
 * - KEY_STATE_IDLE: 等待按键按下
 * - KEY_STATE_DEBOUNCING: 消抖中（确认真实按键）
 * - KEY_STATE_PRESSED: 按键已确认，执行发送
 * - KEY_STATE_LONG_PRESS: 长按模式（仅长按按键）
 * - KEY_STATE_RELEASED: 等待释放完成
 */
uchar keybord_trembling()
{
// 按键状态定义
#define KEY_STATE_IDLE 0       // 空闲状态（等待按键）
#define KEY_STATE_DEBOUNCING 1 // 消抖中
#define KEY_STATE_PRESSED 2    // 按键已确认（已发送）
#define KEY_STATE_LONG_PRESS 3 // 长按连续发送中
#define KEY_STATE_RELEASED 4   // 等待释放消抖

    static uchar idata state = KEY_STATE_IDLE;
    static uchar idata debounce_counter = 0;
    static uchar idata temp_old = 0;
    static uint idata long_press_timer = 0;    // 长按计时器
    static uchar idata long_press_started = 0; // 长按已开始标志
    static uchar idata key_sent = 0;           // 当前按键是否已发送

    uchar temp;

    temp = filter(keybord_scanning());

    switch (state)
    {
    case KEY_STATE_IDLE:
        // 空闲状态：确保报告清零，检测按键按下
        memset(HIDKey, 0, 8);
        if (temp != 0)
        {
            temp_old = temp;
            debounce_counter = 1;
            state = KEY_STATE_DEBOUNCING;
        }
        break;

    case KEY_STATE_DEBOUNCING:
        // 消抖状态：保持报告清零，连续确认按键
        memset(HIDKey, 0, 8);
        if (temp == temp_old)
        {
            debounce_counter++;
            // 消抖完成（连续3次确认）
            if (debounce_counter >= 3)
            {
                state = KEY_STATE_PRESSED;
                key_sent = 0; // 重置发送标志
            }
        }
        else
        {
            // 按键不稳定，返回空闲
            state = KEY_STATE_IDLE;
            temp_old = 0;
        }
        break;

    case KEY_STATE_PRESSED:
        // 按键已确认，执行发送（只发送一次）
        if (!key_sent && temp > 0 && temp <= 35)
        {
            uchar current_key = key_code[temp - 1];

            // 检查是否是多媒体按键（Consumer Page）
            if (is_multimedia_key(current_key))
            {
                MULKey[0] = current_key;
            }
            // 检查是否是纯修饰键
            else if (get_modifier_only_key(temp - 1) != 0)
            {
                HIDKey[0] = get_modifier_only_key(temp - 1);
            }
            else if (HIDKey[1] < 6)
            {
                uchar modifier = get_key_modifier(current_key);
                uchar repeat_count = get_key_repeat_count(temp - 1);
                uchar j;

                // 连续发送多次（用于重复按键如双0）
                for (j = 0; j < repeat_count && HIDKey[1] < 6; j++)
                {
                    HIDKey[2 + HIDKey[1]++] = current_key;
                }

                if (modifier != 0)
                {
                    HIDKey[0] = modifier;
                }
            }

            key_sent = 1; // 标记已发送

            // 检查是否是长按按键
            if (is_long_press_key(temp - 1) && !is_multimedia_key(key_code[temp - 1]))
            {
                // 长按按键：初始化长按计时器，进入长按状态
                long_press_timer = 0;
                long_press_started = 0;
                state = KEY_STATE_LONG_PRESS;
            }
            else
            {
                // 普通按键：进入等待释放状态
                state = KEY_STATE_RELEASED;
            }
        }

        // 如果按键已释放，清零报告并进入释放消抖
        if (temp == 0)
        {
            memset(HIDKey, 0, 8);
            debounce_counter = 0;
            state = KEY_STATE_RELEASED;
        }
        break;

    case KEY_STATE_LONG_PRESS:
        // 长按连续发送状态
        if (temp == temp_old && temp <= 35)
        {
            uchar current_key = key_code[temp - 1];

            // 多媒体按键不支持长按
            if (!is_multimedia_key(current_key))
            {
                long_press_timer++;

                // 长按超过1000ms（1秒）才开始连续发送
                if (long_press_timer > 1000)
                {
                    long_press_started = 1;
                }

                // 长按开始后，持续保持按键在报告中（不脉冲）
                // OS 的 typematic 机制会自动处理按键重复
                if (long_press_started)
                {
                    uchar modifier = get_key_modifier(current_key);
                    HIDKey[0] = modifier;
                    HIDKey[2] = current_key;
                    // HIDKey[1] 保持为 0（保留字节），其余键码槽位为空
                }
            }
        }
        else if (temp == 0)
        {
            // 按键释放，清零报告并进入释放消抖
            memset(HIDKey, 0, 8);
            debounce_counter = 0;
            state = KEY_STATE_RELEASED;
        }
        break;

    case KEY_STATE_RELEASED:
        // 等待按键释放：保持按键按下状态（保留 HIDKey），确保组合键可靠
        if (temp == 0)
        {
            // 按键已释放 - 清零报告发送释放信号
            memset(HIDKey, 0, 8);
            debounce_counter++;
            // 释放消抖完成
            if (debounce_counter >= 3)
            {
                // 完全释放，返回空闲状态
                state = KEY_STATE_IDLE;
                temp_old = 0;
                long_press_timer = 0;
                long_press_started = 0;
                key_sent = 0;
            }
        }
        else if (temp == temp_old)
        {
            // 按键仍然按住 - 保持 HIDKey 不变（按键持续按下，不脉冲）
            // 由 OS typematic 机制处理按键重复
            // 长按按键转入长按状态处理
            if (is_long_press_key(temp - 1))
            {
                state = KEY_STATE_LONG_PRESS;
            }
        }
        else
        {
            // 检测到不同按键 - 释放当前按键，处理新按键
            memset(HIDKey, 0, 8);
            state = KEY_STATE_IDLE;
            temp_old = 0;
            long_press_timer = 0;
            long_press_started = 0;
            key_sent = 0;
        }
        break;

    default:
        state = KEY_STATE_IDLE;
        break;
    }

    HIDKey[1] = 0;  // 确保 HID 报告保留字节为 0
    return 0;
}

uchar HIDKey_transfer()
{
    return drv_usb_keyboard(HIDKey);
}
uchar MULKey_transfer()
{
    static uchar tick = 0;
    uchar flag;
    flag = drv_usb_mul(MULKey[0]);

    if (MULKey[0]) // 确保触发一次会持续5ms
    {
        if (tick < 5)
            tick++;
        else
            MULKey[0] = 0;
    }
    else
        tick = 0;
    return flag;
}

/**
 * @brief EC11旋转编码器处理函数
 *
 * 检测编码器旋转方向，发送音量控制按键
 * 顺时针旋转：音量增加
 * 逆时针旋转：音量减少
 */
void ec11_handler(void)
{
    static uchar last_state = 0; // 上一次的状态
    static uchar debounce = 0;   // 消抖计数
    uchar current_state;
    uchar a, b;
    uchar transition;

    // 读取A相和B相状态
    a = EC11_A;
    b = EC11_B;

    // 组合当前状态 (A << 1) | B
    current_state = (a << 1) | b;

    // 状态变化检测
    if (current_state != last_state)
    {
        debounce++;
        if (debounce >= 2) // 简单消抖
        {
            debounce = 0;

            // 判断旋转方向
            // 顺时针：00->01->11->10->00 或 00->10->11->01->00
            // 逆时针：00->10->11->01->00 或 00->01->11->10->00

            // 使用状态表判断方向
            // 顺时针状态转换：0->1->3->2->0 (二进制: 00->01->11->10->00)
            // 逆时针状态转换：0->2->3->1->0 (二进制: 00->10->11->01->00)

            transition = (last_state << 2) | current_state;

            // 无条件更新 last_state（无论本次转换是否为有效旋转）。
            // 原因：EC11 机械触点抖动会产生短暂无效转换（如 00->11，两相同时跳变），
            // 若只在有效旋转时更新 last_state，遇到无效转换时 last_state 永远不更新，
            // 状态机将卡死在该转换上，后续旋转全部失效（表现为"触发一次就不工作"）。
            // 无条件更新可保证持续跟踪编码器位置，最多漏掉一个事件但不会卡死。
            last_state = current_state;

            // 顺时针判断
            if (transition == 0x01 || transition == 0x13 || transition == 0x32 || transition == 0x20)
            {
                // 顺时针旋转 - 音量增加
                MULKey[0] = HID_CONSUMER_VOLUME_UP;
                MULKey_transfer();
            }
            // 逆时针判断
            else if (transition == 0x02 || transition == 0x23 || transition == 0x31 || transition == 0x10)
            {
                // 逆时针旋转 - 音量减少
                MULKey[0] = HID_CONSUMER_VOLUME_DOWN;
                MULKey_transfer();
            }
        }
    }
    else
    {
        debounce = 0;
    }
}

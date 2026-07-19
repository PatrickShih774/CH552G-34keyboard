#include "div_usb.h"
#include "keybord.h"

uchar xdata xdata_null[192] _at_ 0x0000;

uchar xdata buf_ep0[EP0_SIZE + MAX_PACKET_SIZE + MAX_PACKET_SIZE] _at_ 0x0340;
static uchar xdata buf_ep2[EP2_SIZE] _at_ 0x0300;

static uchar idata SetupReq, SetupLen, Count, FLAG, UsbConfig;
uchar idata Ready; // USB连接状态，非静态以便外部引用
static idata uchar *pDescr;
static USB_SETUP_REQ idata SetupReqBuf;

#define UsbSetupBuf ((PUSB_SETUP_REQ)buf_ep0)

/**
 * @brief HID Keyboard Report Descriptor with Report ID
 *
 * HID Protocol Compliance:
 * - Report ID (0x85) is required for devices with multiple report types
 * - Report ID = 1 for standard keyboard input report
 * - Report ID allows host to distinguish between different report types
 *
 * Report Structure (9 bytes total):
 * Byte 0: Report ID (0x01) - Identifies this as keyboard input report
 * Byte 1: Modifier keys (Ctrl, Shift, Alt, GUI) - 8 bits
 * Byte 2: Reserved (0x00) - Must be 0
 * Byte 3-8: Key codes (up to 6 simultaneous keys)
 *
 * Total Report Size: 9 bytes (1 byte Report ID + 8 bytes data)
 */
char code desc_hid_keyboard[] =
    {
        /*
         * Usage Page: Generic Desktop (0x01)
         * Defines the general category of the device
         */
        0x05,
        0x01,

        /*
         * Usage: Keyboard (0x06)
         * Specifies this is a keyboard device within Generic Desktop
         */
        0x09,
        0x06,

        /*
         * Collection: Application (0x01)
         * Groups all keyboard-related items together
         * All following items belong to this collection until End Collection
         */
        0xa1,
        0x01,

        /*
         * Report ID: 1 (0x85, 0x01)
         * HID Protocol Requirement:
         * - Mandatory for devices with multiple report types
         * - Allows host driver to identify report format
         * - Value 1 = Standard Keyboard Input Report
         * - Prepended to all keyboard input reports
         */
        0x85,
        0x01,

        /*
         * Modifier Keys Section
         * Usage Page: Keyboard/Keypad (0x07)
         * Defines key codes for modifier keys
         */
        0x05,
        0x07,

        /*
         * Usage Minimum: 224 (0xE0) - Left Control
         * Usage Maximum: 231 (0xE7) - Right GUI
         * Defines the range of modifier keys
         */
        0x19,
        0xe0,
        0x29,
        0xe7,

        /*
         * Logical Minimum: 0
         * Logical Maximum: 1
         * Modifier keys are binary: 0=released, 1=pressed
         */
        0x15,
        0x00,
        0x25,
        0x01,

        /*
         * Report Size: 1 bit
         * Report Count: 8
         * Each modifier key uses 1 bit, total 8 bits = 1 byte
         * Bit 0: Left Control    Bit 4: Left GUI
         * Bit 1: Left Shift      Bit 5: Right Control
         * Bit 2: Left Alt        Bit 6: Right Shift
         * Bit 3: Left GUI        Bit 7: Right Alt
         */
        0x75,
        0x01,
        0x95,
        0x08,

        /*
         * Input: Data, Variable, Absolute
         * Defines this as input data that can change
         */
        0x81,
        0x02,

        /*
         * Reserved Byte
         * Report Size: 8 bits (1 byte)
         * Report Count: 1
         * This byte is reserved and must be set to 0
         * Required by HID specification for legacy compatibility
         */
        0x95,
        0x01,
        0x75,
        0x08,
        0x81,
        0x03,

        /*
         * LED Output Report Section
         * Usage Page: LEDs (0x08)
         * Controls keyboard LEDs (Num Lock, Caps Lock, Scroll Lock, etc.)
         */
        0x95,
        0x05,
        0x75,
        0x01,
        0x05,
        0x08,
        0x19,
        0x01,
        0x29,
        0x05,

        /*
         * Output: Data, Variable, Absolute
         * Defines LED control as output data
         */
        0x91,
        0x02,

        /*
         * LED Report Padding
         * 3 bits of padding to align to byte boundary
         */
        0x95,
        0x01,
        0x75,
        0x03,
        0x91,
        0x03,

        /*
         * Key Codes Section
         * Usage Page: Keyboard/Keypad (0x07)
         * Defines the main key codes
         */
        0x95,
        0x06, // Report Count: 6 (up to 6 simultaneous keys)
        0x75,
        0x08, // Report Size: 8 bits per key code
        0x15,
        0x00, // Logical Minimum: 0
        0x25,
        0xFF, // Logical Maximum: 255
        0x05,
        0x07, // Usage Page: Keyboard/Keypad
        0x19,
        0x00, // Usage Minimum: 0 (No event)
        0x29,
        0x65, // Usage Maximum: 101 (Keyboard Application)

        /*
         * Input: Data, Array, Absolute
         * Array type allows for multiple simultaneous key presses
         * Each byte contains one key code (0 = no key)
         */
        0x81,
        0x00,

        /*
         * End Collection
         * Closes the Application Collection
         */
        0xc0,
        /* consumer */
        0x05,
        0x0c, // USAGE_PAGE (Consumer Devices)
        0x09,
        0x01, // USAGE (Consumer Control)
        0xa1,
        0x01, // COLLECTION (Application)
        0x85,
        0x03, //   REPORT_ID (3)
        0x19,
        0x00, //   USAGE_MINIMUM (Unassigned)
        0x2a,
        0x3c,
        0x03, //   USAGE_MAXIMUM
        0x15,
        0x00, //   LOGICAL_MINIMUM (0)
        0x26,
        0x3c,
        0x03, //   LOGICAL_MAXIMUM (828)
        0x95,
        0x01, //   REPORT_COUNT (1)
        0x75,
        0x10, //   REPORT_SIZE (16)
        0x81,
        0x00, //   INPUT (Data,Ary,Abs)
        0xc0, // END_COLLECTION  /* 25 */

};

char code desc_device[] =
    {
        0x12,
        0x01,
        0x00, 0x02,
        0x00,
        0x00,
        0x00,
        EP0_SIZE,
        LOBYTE(USB_VID), HIBYTE(USB_VID),
        LOBYTE(USB_PID), HIBYTE(USB_PID),
        LOBYTE(VERSION), HIBYTE(VERSION),
        0x01,
        0x02,
        0x03,
        0x01};

#define config_len (9 + 9 + 9 + 7)
char code desc_config[config_len] =
    {
        0x09, 0x02, config_len, 0x00, 0x01, 0x01, 0x00, 0xA0, 0x5A,

        0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00,
        0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, LOBYTE(sizeof(desc_hid_keyboard)), HIBYTE(sizeof(desc_hid_keyboard)),
        0x07, 0x05, 0x82, 0x03, EP2_SIZE, 0x00, 0x01};

char code LangDes[] = {0x04, 0x03, 0x09, 0x04};
char code Manuf_Des[] = {0x0E, 0x03, 'C', 0x00, 'H', 0x00, '5', 0x00, '5', 0x00, '2', 0x00, 'G', 0x00};
char code Prod_Des[] = {0x10, 0x03, 'K', 0x00, 'e', 0x00, 'y', 0x00, 'b', 0x00, 'o', 0x00, 'a', 0x00, 'r', 0x00};

char code Ser_Des[30] = {0};

/**
 * @brief USB 外设初始化
 *
 * 配置 USB 控制器为全速设备模式，使能设备上拉。
 * 初始化 EP0（控制端点）、EP2（HID 中断 IN）、EP4（预留）。
 * 设置 DMA 缓冲区地址，使能 USB 中断（挂起/传输/总线复位）。
 */
void drv_usb_init(void)
{
    SetupReq = 0;
    SetupLen = 0;
    Ready = 0;
    Count = 0;
    UsbConfig = 0;

    IE_USB = 0;

    USB_CTRL = 0x06;

    UDEV_CTRL = 0x00;

    memset((void *)&SetupReqBuf, 0, sizeof(SetupReqBuf));
    memset(buf_ep0, 0, sizeof(buf_ep0));
    memset(buf_ep2, 0, sizeof(buf_ep2));

    USB_CTRL = 0x00;

    UDEV_CTRL = 0x80;
    UDEV_CTRL |= bUD_PD_DIS;

    UDEV_CTRL &= ~bUD_LOW_SPEED;
    USB_CTRL &= ~bUC_LOW_SPEED;

    UEP4_T_LEN = 0;
    UEP0_DMA = buf_ep0;
    UEP4_1_MOD |= bUEP4_TX_EN | bUEP4_RX_EN;
    UEP4_CTRL = UEP_T_RES_NAK | UEP_R_RES_ACK;
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    UEP0_T_LEN = 0;

    UEP2_DMA = buf_ep2;
    UEP2_3_MOD |= bUEP2_TX_EN;
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
    UEP2_T_LEN = 0;

    USB_DEV_AD = 0x00;
    USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;
    UDEV_CTRL |= bUD_PORT_EN;
    USB_INT_FG = 0xFF;
    USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
    IE_USB = 1;
}

/**
 * @brief 向 USB EP2 写入数据并发送
 *
 * 等待上一包发送完成（T_RES == ACK 表示传输中），
 * 然后将数据拷贝到 EP2 缓冲区并触发发送。
 *
 * @param buf 待发送数据缓冲区指针
 * @param len 数据长度（超过 EP2_SIZE 时截断）
 *
 * @note 等待超时保护：若主机未轮询 EP2（枚举未完成、主机挂起或断连），
 *       TX 永不完成，T_RES 恒为 ACK 会死锁主循环。超时后放弃本次发送，
 *       待主机恢复轮询后自愈。
 * @note == 优先级高于 &，原写法 UEP2_CTRL & MASK == ACK 被解析为
 *       UEP2_CTRL & (MASK == ACK) => while(0)，等待逻辑完全失效。
 *       必须加括号：(UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK
 */
void drv_usb_write_ep2(char *buf, uchar len)
{
    {
        uint timeout = 10000;
        while ((UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK)
        {
            if (--timeout == 0)
                return;
        }
    }

    len = (len > EP2_SIZE) ? EP2_SIZE : len;
    memcpy(buf_ep2, buf, len);

    UEP2_T_LEN = len;
    UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
}
/**
 * @brief 发送键盘 HID 报告（带差分检测）
 *
 * 报告格式（9 字节，含 ReportID）：
 *   p[0] = 修饰键（Ctrl/Shift/Alt/Win 位图）
 *   p[1] = 保留字节（恒为 0）
 *   p[2..7] = 6 个按键键码
 *
 * 差分检测：与上次发送的键盘报告逐字节比较，
 * 数据未变化时不发送以节省 USB 带宽。
 *
 * @param p 指向 8 字节键盘报告数据的指针
 * @return 0 表示无变化未发送，1 表示已发送
 */
uchar drv_usb_keyboard(uchar *p)
{
    static keyboard_report_t idata temp = {1, 0, 0, {0}};
    uchar i;
    uchar flag = 0;
    for (i = 0; i < 8; i++)
    {
        if (((uchar *)(&temp))[i + 1] != p[i])
        {
            ((uchar *)(&temp))[i + 1] = p[i];
            flag = 1;
        }
    }
    if (flag)
    {
        drv_usb_write_ep2((uchar *)(&temp), 9);
    }
    return flag;
}
/**
 * @brief 发送 Consumer 多媒体 HID 报告（带差分检测 + 强制发送）
 *
 * 报告格式（3 字节）：
 *   [0] = ReportID = 3
 *   [1] = usage 低字节
 *   [2] = usage 高字节（在用 usage 均 ≤ 0xFF，恒为 0）
 *
 * @param dat   多媒体按键功能码（Consumer Page usage）
 * @param force 1 = 强制发送，跳过差分检测；0 = 仅数据变化时发送
 * @return 0 表示无变化未发送，1 表示已发送
 */
uchar drv_usb_mul(uchar dat, uchar force)
{
    static consumer_report_t idata temp = {3, 0, 0};
    uchar flag = 0;

    if (force || temp.usage_lo != dat)
    {
        temp.usage_lo = dat;
        temp.usage_hi = 0; // usage 高字节（在用 usage 均 <= 0xFF）
        flag = 1;
    }

    if (flag)
    {
        drv_usb_write_ep2((uchar *)(&temp), 3);
    }
    return flag;
}

// LED 状态通过 SET_REPORT 输出报告接收，见 UIS_TOKEN_OUT | 0 处理

/**
 * @brief USB 中断服务函数
 *
 * 处理 USB 总线事件：
 * - EP2 IN TX 完成 → 清除发送状态
 * - EP4 IN/OUT → 预留端点处理
 * - SETUP（EP0）→ 枚举/描述符/请求处理
 * - EP0 IN → 分段传输续传
 * - EP0 OUT → SET_REPORT（LED 状态）ACK
 * - 总线复位 → 重置设备地址与端点状态
 * - 挂起 → 进入低功耗模式（PD）
 *
 * 使用寄存器组 1（using 1）以避免与主循环寄存器冲突。
 */
void usb_handler(void) interrupt INT_NO_USB using 1
{
    uchar len;

    if (UIF_TRANSFER)
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 2:
            UEP2_T_LEN = 0;
            UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
            break;

        case UIS_TOKEN_OUT | 2:
            break;

        case UIS_TOKEN_IN | 4:
            UEP4_T_LEN = 0;
            UEP4_CTRL ^= bUEP_T_TOG;
            UEP4_CTRL = UEP4_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
            break;

        case UIS_TOKEN_OUT | 4:
            if (U_TOG_OK)
            {
                len = USB_RX_LEN;
                UEP4_CTRL ^= bUEP_R_TOG;
            }
            break;

        case UIS_TOKEN_SETUP | 0:
            len = USB_RX_LEN;

            if (len == (sizeof(USB_SETUP_REQ)))
            {
                if (UsbSetupBuf->wLengthH)
                    SetupLen = 0xff;
                else
                    SetupLen = UsbSetupBuf->wLengthL;
                len = 0;
                SetupReq = UsbSetupBuf->bRequest;

                if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
                {
                    switch (SetupReq)
                    {
                    case HID_GET_REPORT:
                        pDescr = buf_ep0;
                        len = SetupLen;
                        break;

                    case HID_GET_IDLE:
                        break;

                    case HID_GET_PROTOCOL:
                        break;

                    case HID_SET_REPORT:
                        Ready = 1;
                        break;

                    case HID_SET_IDLE:
                        UEP0_T_LEN = 0;
                        break;

                    case HID_SET_PROTOCOL:
                        break;

                    default:
                        len = 0xFF;
                        break;
                    }

                    if (SetupLen > len)
                    {
                        SetupLen = len;
                    }

                    len = SetupLen >= EP0_SIZE ? EP0_SIZE : SetupLen;
                    memcpy(buf_ep0, pDescr, len);
                    SetupLen -= len;
                    pDescr += len;
                }
                else
                {
                    switch (SetupReq)
                    {
                    case USB_GET_DESCRIPTOR:
                        switch (UsbSetupBuf->wValueH)
                        {
                        case 1:
                            pDescr = desc_device;
                            len = sizeof(desc_device);
                            break;

                        case 2:
                            pDescr = desc_config;
                            len = sizeof(desc_config);
                            break;

                        case 3:
                            switch (UsbSetupBuf->wValueL)
                            {
                            case 0:
                                pDescr = LangDes;
                                len = sizeof(LangDes);
                                break;
                            case 1:
                                pDescr = Manuf_Des;
                                len = sizeof(Manuf_Des);
                                break;
                            case 2:
                                pDescr = Prod_Des;
                                len = sizeof(Prod_Des);
                                break;
                            default:
                                pDescr = Ser_Des;
                                len = sizeof(Ser_Des);
                                break;
                            }
                            break;

                        case 0x22:
                            pDescr = desc_hid_keyboard;
                            len = sizeof(desc_hid_keyboard);
                            Ready = 1;
                            break;

                        default:
                            len = 0xff;
                            break;
                        }

                        if (SetupLen > len)
                        {
                            SetupLen = len;
                        }

                        len = SetupLen >= EP0_SIZE ? EP0_SIZE : SetupLen;
                        memcpy(buf_ep0, pDescr, len);
                        SetupLen -= len;
                        pDescr += len;
                        break;

                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;
                        break;

                    case USB_GET_CONFIGURATION:
                        buf_ep0[0] = UsbConfig;
                        if (SetupLen >= 1)
                        {
                            len = 1;
                        }
                        break;

                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        break;

                    case 0x0A:
                        break;

                    case USB_CLEAR_FEATURE:
                        if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                        {
                            switch (UsbSetupBuf->wIndexL)
                            {
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
                                break;
                            case 0x02:
                                UEP2_CTRL = UEP2_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
                                break;
                            case 0x84:
                                UEP4_CTRL = UEP4_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
                                break;
                            case 0x04:
                                UEP4_CTRL = UEP4_CTRL & ~(bUEP_R_TOG | MASK_UEP_R_RES) | UEP_R_RES_ACK;
                                break;
                            default:
                                len = 0xFF;
                                break;
                            }
                        }
                        else
                        {
                            len = 0xFF;
                        }
                        break;

                    case USB_SET_FEATURE:
                        if ((UsbSetupBuf->bRequestType & 0x1F) == 0x00)
                        {
                            if ((((uint)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01)
                            {
                            }
                            else
                            {
                                len = 0xFF;
                            }
                        }
                        else if ((UsbSetupBuf->bRequestType & 0x1F) == 0x02)
                        {
                            if ((((uint)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x00)
                            {
                                switch (((uint)UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL)
                                {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;
                                    break;
                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;
                                    break;
                                case 0x84:
                                    UEP4_CTRL = UEP4_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;
                                    break;
                                case 0x04:
                                    UEP4_CTRL = UEP4_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    len = 0xFF;
                                    break;
                                }
                            }
                            else
                            {
                                len = 0xFF;
                            }
                        }
                        else
                        {
                            len = 0xFF;
                        }
                        break;

                    case USB_GET_STATUS:
                        buf_ep0[0] = 0x00;
                        buf_ep0[1] = 0x00;
                        if (SetupLen >= 2)
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;

                    default:
                        len = 0xff;
                        break;
                    }
                }
            }
            else
            {
                len = 0xff;
            }

            if (len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
            }
            else if (len <= EP0_SIZE)
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
            }
            else
            {
                UEP0_T_LEN = 0;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
            }
            break;

        case UIS_TOKEN_IN | 0:
            switch (SetupReq)
            {
            case USB_GET_DESCRIPTOR:
            case HID_GET_REPORT:
                len = SetupLen >= EP0_SIZE ? EP0_SIZE : SetupLen;
                memcpy(buf_ep0, pDescr, len);
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;
                break;

            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;

            default:
                UEP0_T_LEN = 0;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;

        case UIS_TOKEN_OUT | 0:
            // 主机 SET_REPORT 输出报告（Report ID 1，键盘 LED 状态）。
            // 固件不驱动 LED 指示，仅按协议 ACK 该输出报告。
            UEP0_CTRL ^= bUEP_R_TOG;
            break;

        default:
            break;
        }

        UIF_TRANSFER = 0;
    }

    if (UIF_BUS_RST)
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_ACK;
        UEP4_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;

        Ready = 0;
    }

    if (UIF_SUSPEND)
    {
        UIF_SUSPEND = 0;
        if (USB_MIS_ST & bUMS_SUSPEND)
        {
            PCON |= PD;
        }
    }
    else
    {
        USB_INT_FG = 0xFF;
    }
}
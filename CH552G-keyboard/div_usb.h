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

void drv_usb_init(void);
uchar drv_usb_keyboard(uchar *p);
uchar drv_usb_mul(uchar dat);

// USB连接状态变量（定义在div_usb.c中）
extern uchar idata Ready;

#endif
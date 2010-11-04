/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Driver for ARC USBOTG Device Controller
 *
 * Copyright (C) 2007 by Björn Stenberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include "system.h"
#include "config.h"
#include "string.h"
#include "usb_ch9.h"
#include "usb_core.h"
#include "kernel.h"
#include "panic.h"
#include "usb_drv.h"

/*#define LOGF_ENABLE*/
#include "logf.h"

/* USB device mode registers (Little Endian) */

#define REG_ID               (*(volatile unsigned int *)(USB_BASE+0x000))
#define REG_HWGENERAL        (*(volatile unsigned int *)(USB_BASE+0x004))
#define REG_HWHOST           (*(volatile unsigned int *)(USB_BASE+0x008))
#define REG_HWDEVICE         (*(volatile unsigned int *)(USB_BASE+0x00c))
#define REG_TXBUF            (*(volatile unsigned int *)(USB_BASE+0x010))
#define REG_RXBUF            (*(volatile unsigned int *)(USB_BASE+0x014))
#define REG_CAPLENGTH        (*(volatile unsigned char*)(USB_BASE+0x100))
#define REG_DCIVERSION       (*(volatile unsigned int *)(USB_BASE+0x120))
#define REG_DCCPARAMS        (*(volatile unsigned int *)(USB_BASE+0x124))
#define REG_USBCMD           (*(volatile unsigned int *)(USB_BASE+0x140))
#define REG_USBSTS           (*(volatile unsigned int *)(USB_BASE+0x144))
#define REG_USBINTR          (*(volatile unsigned int *)(USB_BASE+0x148))
#define REG_FRINDEX          (*(volatile unsigned int *)(USB_BASE+0x14c))
#define REG_DEVICEADDR       (*(volatile unsigned int *)(USB_BASE+0x154))
#define REG_ENDPOINTLISTADDR (*(volatile unsigned int *)(USB_BASE+0x158))
#define REG_BURSTSIZE        (*(volatile unsigned int *)(USB_BASE+0x160))
#define REG_ULPI             (*(volatile unsigned int *)(USB_BASE+0x170))
#define REG_CONFIGFLAG       (*(volatile unsigned int *)(USB_BASE+0x180))
#define REG_PORTSC1          (*(volatile unsigned int *)(USB_BASE+0x184))
#define REG_OTGSC            (*(volatile unsigned int *)(USB_BASE+0x1a4))
#define REG_USBMODE          (*(volatile unsigned int *)(USB_BASE+0x1a8))
#define REG_ENDPTSETUPSTAT   (*(volatile unsigned int *)(USB_BASE+0x1ac))
#define REG_ENDPTPRIME       (*(volatile unsigned int *)(USB_BASE+0x1b0))
#define REG_ENDPTFLUSH       (*(volatile unsigned int *)(USB_BASE+0x1b4))
#define REG_ENDPTSTATUS      (*(volatile unsigned int *)(USB_BASE+0x1b8))
#define REG_ENDPTCOMPLETE    (*(volatile unsigned int *)(USB_BASE+0x1bc))
#define REG_ENDPTCTRL0       (*(volatile unsigned int *)(USB_BASE+0x1c0))
#define REG_ENDPTCTRL1       (*(volatile unsigned int *)(USB_BASE+0x1c4))
#define REG_ENDPTCTRL2       (*(volatile unsigned int *)(USB_BASE+0x1c8))
#define REG_ENDPTCTRL(_x_)   (*(volatile unsigned int *)(USB_BASE+0x1c0+4*(_x_)))

/* Frame Index Register Bit Masks */
#define USB_FRINDEX_MASKS                      (0x3fff)

/* USB CMD  Register Bit Masks */
#define USBCMD_RUN                            (0x00000001)
#define USBCMD_CTRL_RESET                     (0x00000002)
#define USBCMD_PERIODIC_SCHEDULE_EN           (0x00000010)
#define USBCMD_ASYNC_SCHEDULE_EN              (0x00000020)
#define USBCMD_INT_AA_DOORBELL                (0x00000040)
#define USBCMD_ASP                            (0x00000300)
#define USBCMD_ASYNC_SCH_PARK_EN              (0x00000800)
#define USBCMD_SUTW                           (0x00002000)
#define USBCMD_ATDTW                          (0x00004000)
#define USBCMD_ITC                            (0x00FF0000)

/* bit 15,3,2 are frame list size */
#define USBCMD_FRAME_SIZE_1024                (0x00000000)
#define USBCMD_FRAME_SIZE_512                 (0x00000004)
#define USBCMD_FRAME_SIZE_256                 (0x00000008)
#define USBCMD_FRAME_SIZE_128                 (0x0000000C)
#define USBCMD_FRAME_SIZE_64                  (0x00008000)
#define USBCMD_FRAME_SIZE_32                  (0x00008004)
#define USBCMD_FRAME_SIZE_16                  (0x00008008)
#define USBCMD_FRAME_SIZE_8                   (0x0000800C)

/* bit 9-8 are async schedule park mode count */
#define USBCMD_ASP_00                         (0x00000000)
#define USBCMD_ASP_01                         (0x00000100)
#define USBCMD_ASP_10                         (0x00000200)
#define USBCMD_ASP_11                         (0x00000300)
#define USBCMD_ASP_BIT_POS                    (8)

/* bit 23-16 are interrupt threshold control */
#define USBCMD_ITC_NO_THRESHOLD               (0x00000000)
#define USBCMD_ITC_1_MICRO_FRM                (0x00010000)
#define USBCMD_ITC_2_MICRO_FRM                (0x00020000)
#define USBCMD_ITC_4_MICRO_FRM                (0x00040000)
#define USBCMD_ITC_8_MICRO_FRM                (0x00080000)
#define USBCMD_ITC_16_MICRO_FRM               (0x00100000)
#define USBCMD_ITC_32_MICRO_FRM               (0x00200000)
#define USBCMD_ITC_64_MICRO_FRM               (0x00400000)
#define USBCMD_ITC_BIT_POS                    (16)

/* USB STS Register Bit Masks */
#define USBSTS_INT                            (0x00000001)
#define USBSTS_ERR                            (0x00000002)
#define USBSTS_PORT_CHANGE                    (0x00000004)
#define USBSTS_FRM_LST_ROLL                   (0x00000008)
#define USBSTS_SYS_ERR                        (0x00000010) /* not used */
#define USBSTS_IAA                            (0x00000020)
#define USBSTS_RESET                          (0x00000040)
#define USBSTS_SOF                            (0x00000080)
#define USBSTS_SUSPEND                        (0x00000100)
#define USBSTS_HC_HALTED                      (0x00001000)
#define USBSTS_RCL                            (0x00002000)
#define USBSTS_PERIODIC_SCHEDULE              (0x00004000)
#define USBSTS_ASYNC_SCHEDULE                 (0x00008000)

/* USB INTR Register Bit Masks */
#define USBINTR_INT_EN                        (0x00000001)
#define USBINTR_ERR_INT_EN                    (0x00000002)
#define USBINTR_PTC_DETECT_EN                 (0x00000004)
#define USBINTR_FRM_LST_ROLL_EN               (0x00000008)
#define USBINTR_SYS_ERR_EN                    (0x00000010)
#define USBINTR_ASYN_ADV_EN                   (0x00000020)
#define USBINTR_RESET_EN                      (0x00000040)
#define USBINTR_SOF_EN                        (0x00000080)
#define USBINTR_DEVICE_SUSPEND                (0x00000100)

/* ULPI Register Bit Masks */
#define ULPI_ULPIWU                           (0x80000000)
#define ULPI_ULPIRUN                          (0x40000000)
#define ULPI_ULPIRW                           (0x20000000)
#define ULPI_ULPISS                           (0x08000000)
#define ULPI_ULPIPORT                         (0x07000000)
#define ULPI_ULPIADDR                         (0x00FF0000)
#define ULPI_ULPIDATRD                        (0x0000FF00)
#define ULPI_ULPIDATWR                        (0x000000FF)

/* Device Address bit masks */
#define USBDEVICEADDRESS_MASK                 (0xFE000000)
#define USBDEVICEADDRESS_BIT_POS              (25)

/* endpoint list address bit masks */
#define USB_EP_LIST_ADDRESS_MASK               (0xfffff800)

/* PORTSCX  Register Bit Masks */
#define PORTSCX_CURRENT_CONNECT_STATUS         (0x00000001)
#define PORTSCX_CONNECT_STATUS_CHANGE          (0x00000002)
#define PORTSCX_PORT_ENABLE                    (0x00000004)
#define PORTSCX_PORT_EN_DIS_CHANGE             (0x00000008)
#define PORTSCX_OVER_CURRENT_ACT               (0x00000010)
#define PORTSCX_OVER_CURRENT_CHG               (0x00000020)
#define PORTSCX_PORT_FORCE_RESUME              (0x00000040)
#define PORTSCX_PORT_SUSPEND                   (0x00000080)
#define PORTSCX_PORT_RESET                     (0x00000100)
#define PORTSCX_LINE_STATUS_BITS               (0x00000C00)
#define PORTSCX_PORT_POWER                     (0x00001000)
#define PORTSCX_PORT_INDICTOR_CTRL             (0x0000C000)
#define PORTSCX_PORT_TEST_CTRL                 (0x000F0000)
#define PORTSCX_WAKE_ON_CONNECT_EN             (0x00100000)
#define PORTSCX_WAKE_ON_CONNECT_DIS            (0x00200000)
#define PORTSCX_WAKE_ON_OVER_CURRENT           (0x00400000)
#define PORTSCX_PHY_LOW_POWER_SPD              (0x00800000)
#define PORTSCX_PORT_FORCE_FULL_SPEED          (0x01000000)
#define PORTSCX_PORT_SPEED_MASK                (0x0C000000)
#define PORTSCX_PORT_WIDTH                     (0x10000000)
#define PORTSCX_PHY_TYPE_SEL                   (0xC0000000)

/* bit 11-10 are line status */
#define PORTSCX_LINE_STATUS_SE0                (0x00000000)
#define PORTSCX_LINE_STATUS_JSTATE             (0x00000400)
#define PORTSCX_LINE_STATUS_KSTATE             (0x00000800)
#define PORTSCX_LINE_STATUS_UNDEF              (0x00000C00)
#define PORTSCX_LINE_STATUS_BIT_POS            (10)

/* bit 15-14 are port indicator control */
#define PORTSCX_PIC_OFF                        (0x00000000)
#define PORTSCX_PIC_AMBER                      (0x00004000)
#define PORTSCX_PIC_GREEN                      (0x00008000)
#define PORTSCX_PIC_UNDEF                      (0x0000C000)
#define PORTSCX_PIC_BIT_POS                    (14)

/* bit 19-16 are port test control */
#define PORTSCX_PTC_DISABLE                    (0x00000000)
#define PORTSCX_PTC_JSTATE                     (0x00010000)
#define PORTSCX_PTC_KSTATE                     (0x00020000)
#define PORTSCX_PTC_SE0NAK                     (0x00030000)
#define PORTSCX_PTC_PACKET                     (0x00040000)
#define PORTSCX_PTC_FORCE_EN                   (0x00050000)
#define PORTSCX_PTC_BIT_POS                    (16)

/* bit 27-26 are port speed */
#define PORTSCX_PORT_SPEED_FULL                (0x00000000)
#define PORTSCX_PORT_SPEED_LOW                 (0x04000000)
#define PORTSCX_PORT_SPEED_HIGH                (0x08000000)
#define PORTSCX_PORT_SPEED_UNDEF               (0x0C000000)
#define PORTSCX_SPEED_BIT_POS                  (26)

/* bit 28 is parallel transceiver width for UTMI interface */
#define PORTSCX_PTW                            (0x10000000)
#define PORTSCX_PTW_8BIT                       (0x00000000)
#define PORTSCX_PTW_16BIT                      (0x10000000)

/* bit 31-30 are port transceiver select */
#define PORTSCX_PTS_UTMI                       (0x00000000)
#define PORTSCX_PTS_CLASSIC                    (0x40000000)
#define PORTSCX_PTS_ULPI                       (0x80000000)
#define PORTSCX_PTS_FSLS                       (0xC0000000)
#define PORTSCX_PTS_BIT_POS                    (30)

/* USB MODE Register Bit Masks */
#define USBMODE_CTRL_MODE_IDLE                (0x00000000)
#define USBMODE_CTRL_MODE_DEVICE              (0x00000002)
#define USBMODE_CTRL_MODE_HOST                (0x00000003)
#define USBMODE_CTRL_MODE_RSV                 (0x00000001)
#define USBMODE_SETUP_LOCK_OFF                (0x00000008)
#define USBMODE_STREAM_DISABLE                (0x00000010)

/* Endpoint Flush Register */
#define EPFLUSH_TX_OFFSET                      (0x00010000)
#define EPFLUSH_RX_OFFSET                      (0x00000000)

/* Endpoint Setup Status bit masks */
#define EPSETUP_STATUS_MASK                   (0x0000003F)
#define EPSETUP_STATUS_EP0                    (0x00000001)

/* ENDPOINTCTRLx  Register Bit Masks */
#define EPCTRL_TX_ENABLE                       (0x00800000)
#define EPCTRL_TX_DATA_TOGGLE_RST              (0x00400000)    /* Not EP0 */
#define EPCTRL_TX_DATA_TOGGLE_INH              (0x00200000)    /* Not EP0 */
#define EPCTRL_TX_TYPE                         (0x000C0000)
#define EPCTRL_TX_DATA_SOURCE                  (0x00020000)    /* Not EP0 */
#define EPCTRL_TX_EP_STALL                     (0x00010000)
#define EPCTRL_RX_ENABLE                       (0x00000080)
#define EPCTRL_RX_DATA_TOGGLE_RST              (0x00000040)    /* Not EP0 */
#define EPCTRL_RX_DATA_TOGGLE_INH              (0x00000020)    /* Not EP0 */
#define EPCTRL_RX_TYPE                         (0x0000000C)
#define EPCTRL_RX_DATA_SINK                    (0x00000002)    /* Not EP0 */
#define EPCTRL_RX_EP_STALL                     (0x00000001)

/* bit 19-18 and 3-2 are endpoint type */
#define EPCTRL_TX_EP_TYPE_SHIFT                (18)
#define EPCTRL_RX_EP_TYPE_SHIFT                (2)

/* pri_ctrl Register Bit Masks */
#define PRI_CTRL_PRI_LVL1                      (0x0000000C)
#define PRI_CTRL_PRI_LVL0                      (0x00000003)

/* si_ctrl Register Bit Masks */
#define SI_CTRL_ERR_DISABLE                    (0x00000010)
#define SI_CTRL_IDRC_DISABLE                   (0x00000008)
#define SI_CTRL_RD_SAFE_EN                     (0x00000004)
#define SI_CTRL_RD_PREFETCH_DISABLE            (0x00000002)
#define SI_CTRL_RD_PREFEFETCH_VAL              (0x00000001)

/* control Register Bit Masks */
#define USB_CTRL_IOENB                         (0x00000004)
#define USB_CTRL_ULPI_INT0EN                   (0x00000001)

/* OTGSC Register Bit Masks */
#define OTGSC_B_SESSION_VALID                  (0x00000800)
#define OTGSC_A_VBUS_VALID                     (0x00000200)

#define QH_MULT_POS                            (30)
#define QH_ZLT_SEL                             (0x20000000)
#define QH_MAX_PKT_LEN_POS                     (16)
#define QH_IOS                                 (0x00008000)
#define QH_NEXT_TERMINATE                      (0x00000001)
#define QH_IOC                                 (0x00008000)
#define QH_MULTO                               (0x00000C00)
#define QH_STATUS_HALT                         (0x00000040)
#define QH_STATUS_ACTIVE                       (0x00000080)

#define DTD_NEXT_TERMINATE                   (0x00000001)
#define DTD_IOC                              (0x00008000)
#define DTD_STATUS_ACTIVE                    (0x00000080)
#define DTD_STATUS_HALTED                    (0x00000040)
#define DTD_STATUS_DATA_BUFF_ERR             (0x00000020)
#define DTD_STATUS_TRANSACTION_ERR           (0x00000008)
#define DTD_RESERVED_FIELDS                  (0x80007300)
#define DTD_ADDR_MASK                        (0xFFFFFFE0)
#define DTD_PACKET_SIZE                      (0x7FFF0000)
#define DTD_LENGTH_BIT_POS                   (16)
#define DTD_MULT_OVERRIDE_POS                (10)
#define DTD_MULT_OVERRIDE_MASK               (0x00000c00)
#define DTD_ERROR_MASK                       (DTD_STATUS_HALTED | \
                                               DTD_STATUS_DATA_BUFF_ERR | \
                                               DTD_STATUS_TRANSACTION_ERR)
#define DTD_MAX_TRANSFER_LENGTH              (0x4000)

#define DTD_SOFTWARE_LENGTH_MASK             0x000fffff
#define DTD_SOFTWARE_OFFSET_MASK             0xfff00000
#define DTD_SOFTWARE_OFFSET_BIT_POS          20

/*-------------------------------------------------------------------------*/

/* 4 transfer descriptors per endpoint allow 64k transfers, which is the usual MSC
   transfer size, so it seems like a good size */
#define NUM_TDS_PER_EP 4

typedef struct usb_endpoint
{
    bool allocated[2];
    short type[2];
    short max_pkt_size[2];
    short mode[2];
    int nb_tds[2];
    struct transfer_descriptor *tds[2];
} usb_endpoint_t;
static usb_endpoint_t endpoints[USB_NUM_ENDPOINTS];

/* manual: 32.13.2 Endpoint Transfer Descriptor (dTD) */
struct transfer_descriptor {
    unsigned int next_td_ptr;           /* Next TD pointer(31-5), T(0) set
                                           indicate invalid */
    unsigned int size_ioc_sts;          /* Total bytes (30-16), IOC (15),
                                           MultO(11-10), STS (7-0)  */
    unsigned int buff_ptr0;             /* Buffer pointer Page 0 */
    unsigned int buff_ptr1;             /* Buffer pointer Page 1 */
    unsigned int buff_ptr2;             /* Buffer pointer Page 2 */
    unsigned int buff_ptr3;             /* Buffer pointer Page 3 */
    unsigned int buff_ptr4;             /* Buffer pointer Page 4 */
    /* for software use */
    /* The controller is free to modify the current offset value of the buffer pointer page 0
     * For this reason, we keep a copy of the offset to retrieve the buffer pointer on completion */
    unsigned int off_length;                /* page offset (31-20), length of the buffer(19-0) */
} __attribute__ ((packed)) __attribute__ ((aligned(32)));

/* manual: 32.13.1 Endpoint Queue Head (dQH) */
struct queue_head {
    unsigned int max_pkt_length;    /* Mult(31-30) , Zlt(29) , Max Pkt len
                                       and IOS(15) */
    unsigned int curr_dtd_ptr;      /* Current dTD Pointer(31-5) */
    struct transfer_descriptor dtd; /* dTD overlay */
    unsigned int setup_buffer[2];   /* Setup data 8 bytes */
    /* for software use */
    /* queue: index of the first td of the queue (-1 if empty queue) */
    /* repeat: index of the current td (-1 if not started) */
    int head_td;
    int tail_td; /* index of the last td of queue */
    int wait; /* only valid when there is one transfer in the queue */
    int status; /* only valid when there is one transfer in the queue and waiting */
} __attribute__((packed)) __attribute__((aligned(64)));

static struct queue_head qh_array[USB_NUM_ENDPOINTS*2]
    USB_QHARRAY_ATTR;

static struct wakeup transfer_completion_signal[USB_NUM_ENDPOINTS*2]
    SHAREDBSS_ATTR;

static const unsigned int pipe2mask[] = {
    0x01, 0x010000,
    0x02, 0x020000,
    0x04, 0x040000,
    0x08, 0x080000,
    0x10, 0x100000,
};

/*-------------------------------------------------------------------------*/
static void transfer_completed(void);
static void control_received(void);
static int prime_transfer(int ep_num, struct transfer_descriptor *new_td, bool send, bool wait);
static void bus_reset(void);
static void init_control_queue_heads(void);
static void init_queue_heads(void);
static void init_endpoints(void);
/*-------------------------------------------------------------------------*/
static void usb_drv_stop(void)
{
    /* disable interrupts */
    REG_USBINTR = 0;
    /* stop usb controller (disconnect) */
    REG_USBCMD &= ~USBCMD_RUN;
}

static void usb_drv_reset(void)
{
    int oldlevel = disable_irq_save();
    REG_USBCMD &= ~USBCMD_RUN;
    restore_irq(oldlevel);

#ifdef USB_PORTSCX_PHY_TYPE
    /* If a PHY type is specified, set it now */
    REG_PORTSC1 = (REG_PORTSC1 & ~PORTSCX_PHY_TYPE_SEL) | USB_PORTSCX_PHY_TYPE;
#endif
    sleep(HZ/20);
    REG_USBCMD |= USBCMD_CTRL_RESET;
    while (REG_USBCMD & USBCMD_CTRL_RESET);

#if CONFIG_CPU == PP5022 || CONFIG_CPU == PP5024
    /* On a CPU which identifies as a PP5022, this
       initialization must be done after USB is reset.
     */
    outl(inl(0x70000060) | 0xF, 0x70000060);
    outl(inl(0x70000028) | 0x10000, 0x70000028);
    outl(inl(0x70000028) & ~0x10000, 0x70000028);
    outl(inl(0x70000060) & ~0x20, 0x70000060);
    udelay(10);
    outl(inl(0x70000060) | 0x20, 0x70000060);
    udelay(10);
    outl((inl(0x70000060) & ~0xF) | 4, 0x70000060);
    udelay(10);
    outl(inl(0x70000060) & ~0x20, 0x70000060);
    udelay(10);
    outl(inl(0x70000060) & ~0xF, 0x70000060);
    udelay(10);
    outl(inl(0x70000060) | 0x20, 0x70000060);
    udelay(10);
    outl(inl(0x70000028) | 0x800, 0x70000028);
    outl(inl(0x70000028) & ~0x800, 0x70000028);
    while ((inl(0x70000028) & 0x80) == 0);
#endif
}

/* One-time driver startup init */
void usb_drv_startup(void)
{
    /* Initialize all the signal objects once */
    int i;
    for(i=0;i<USB_NUM_ENDPOINTS*2;i++) {
        wakeup_init(&transfer_completion_signal[i]);
    }
}

/* manual: 32.14.1 Device Controller Initialization */
static void _usb_drv_init(bool attach)
{
    usb_drv_reset();

    REG_USBMODE = USBMODE_CTRL_MODE_DEVICE;

#ifdef USB_NO_HIGH_SPEED
    /* Force device to full speed */
    /* See 32.9.5.9.2 */
    REG_PORTSC1 |= PORTSCX_PORT_FORCE_FULL_SPEED;
#endif

    init_control_queue_heads();

    REG_ENDPOINTLISTADDR = (unsigned int)qh_array;
    REG_DEVICEADDR = 0;

    if (!attach) {
        /* enable RESET interrupt */
        REG_USBINTR = USBINTR_RESET_EN;
    }
    else
    {
        /* enable USB interrupts */
        REG_USBINTR =
            USBINTR_INT_EN |
            USBINTR_ERR_INT_EN |
            USBINTR_PTC_DETECT_EN |
            USBINTR_RESET_EN;
    }

    usb_drv_int_enable(true);

    /* go go go */
    REG_USBCMD |= USBCMD_RUN;

    logf("usb_drv_init() finished");
    logf("usb id %x", REG_ID);
    logf("usb dciversion %x", REG_DCIVERSION);
    logf("usb dccparams %x", REG_DCCPARAMS);

    /* now a bus reset will occur. see bus_reset() */
    (void)attach;
}

#ifdef LOGF_ENABLE
#define XFER_DIR_STR(dir) ((dir) ? "IN" : "OUT")
#define XFER_TYPE_STR(type) \
    ((type) == USB_ENDPOINT_XFER_CONTROL ? "CTRL" : \
     ((type) == USB_ENDPOINT_XFER_ISOC ? "ISOC" : \
      ((type) == USB_ENDPOINT_XFER_BULK ? "BULK" : \
       ((type) == USB_ENDPOINT_XFER_INT ? "INTR" : "INVL"))))

static void log_ep(int ep_num, int ep_dir, char* prefix)
{
    usb_endpoint_t* endpoint = &endpoints[ep_num];

    logf("%s: ep%d %s %s %d", prefix, ep_num, XFER_DIR_STR(ep_dir),
            XFER_TYPE_STR(endpoint->type[ep_dir]),
            endpoint->max_pkt_size[ep_dir]);
}
#else
#undef log_ep
#define log_ep(...)
#endif

void usb_drv_init(void)
{
    _usb_drv_init(false);
}

/* fully enable driver */
void usb_drv_attach(void)
{
    logf("usb_drv_attach");
    sleep(HZ/10);
    _usb_drv_init(true);
}

void usb_drv_exit(void)
{
    usb_drv_stop();

    /* TODO : is one of these needed to save power ?
    REG_PORTSC1 |= PORTSCX_PHY_LOW_POWER_SPD;
    REG_USBCMD |= USBCMD_CTRL_RESET;
    */

    usb_drv_int_enable(false);
}

void usb_drv_int(void)
{
    unsigned int usbintr = REG_USBINTR; /* Only watch enabled ints */
    unsigned int status = REG_USBSTS & usbintr;

#if 0
    if (status & USBSTS_INT) logf("int: usb ioc");
    if (status & USBSTS_ERR) logf("int: usb err");
    if (status & USBSTS_PORT_CHANGE) logf("int: portchange");
    if (status & USBSTS_RESET) logf("int: reset");
#endif

    /* usb transaction interrupt */
    if (status & USBSTS_INT) {
        REG_USBSTS = USBSTS_INT;

        /* a control packet? */
        if (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0) {
            control_received();
        }

        if (REG_ENDPTCOMPLETE)
            transfer_completed();
    }

    /* error interrupt */
    if (status & USBSTS_ERR) {
        REG_USBSTS = USBSTS_ERR;
        logf("usb error int");
    }

    /* reset interrupt */
    if (status & USBSTS_RESET) {
        REG_USBSTS = USBSTS_RESET;

        if (UNLIKELY(usbintr == USBINTR_RESET_EN)) {
            /* USB detected - detach and inform */
            usb_drv_stop();
            usb_drv_usb_detect_event();
        }
        else
        {
            bus_reset();
            usb_core_bus_reset(); /* tell mom */
        }
    }

    /* port change */
    if (status & USBSTS_PORT_CHANGE) {
        REG_USBSTS = USBSTS_PORT_CHANGE;
    }
}

bool usb_drv_stalled(int endpoint,bool in)
{
    if(in) {
        return ((REG_ENDPTCTRL(EP_NUM(endpoint)) & EPCTRL_TX_EP_STALL)!=0);
    }
    else {
        return ((REG_ENDPTCTRL(EP_NUM(endpoint)) & EPCTRL_RX_EP_STALL)!=0);
    }

}
void usb_drv_stall(int endpoint, bool stall, bool in)
{
    int ep_num = EP_NUM(endpoint);

    logf("%sstall %d", stall ? "" : "un", ep_num);

    if(in) {
        if (stall) {
            REG_ENDPTCTRL(ep_num) |= EPCTRL_TX_EP_STALL;
        }
        else {
            REG_ENDPTCTRL(ep_num) &= ~EPCTRL_TX_EP_STALL;
        }
    }
    else {
        if (stall) {
            REG_ENDPTCTRL(ep_num) |= EPCTRL_RX_EP_STALL;
        }
        else {
            REG_ENDPTCTRL(ep_num) &= ~EPCTRL_RX_EP_STALL;
        }
    }
}

int usb_drv_send_nonblocking(int endpoint, void* ptr, int length)
{
    return usb_drv_queue_send_nonblocking(endpoint, ptr, length);
}

int usb_drv_send_blocking(int endpoint, void* ptr, int length)
{
    return usb_drv_queue_send_blocking(endpoint, ptr, length);
}

int usb_drv_recv_blocking(int endpoint, void* ptr, int length)
{
    return usb_drv_queue_recv_blocking(endpoint, ptr, length);
}

int usb_drv_recv_nonblocking(int endpoint, void* ptr, int length)
{
    return usb_drv_queue_recv_nonblocking(endpoint, ptr, length);
}

int usb_drv_port_speed(void)
{
    return (REG_PORTSC1 & 0x08000000) ? 1 : 0;
}

bool usb_drv_connected(void)
{
    return (REG_PORTSC1 &
        (PORTSCX_PORT_SUSPEND | PORTSCX_CURRENT_CONNECT_STATUS))
            == PORTSCX_CURRENT_CONNECT_STATUS;
}

bool usb_drv_powered(void)
{
    /* true = bus 4V4 ok */
    return (REG_OTGSC & OTGSC_A_VBUS_VALID) ? true : false;
}

void usb_drv_set_address(int address)
{
    REG_DEVICEADDR = address << USBDEVICEADDRESS_BIT_POS;
    init_queue_heads();
    init_endpoints();
}

void usb_drv_reset_endpoint(int endpoint, bool send)
{
    int pipe = EP_NUM(endpoint) * 2 + (send ? 1 : 0);
    unsigned int mask = pipe2mask[pipe];
    REG_ENDPTFLUSH = mask;
    while (REG_ENDPTFLUSH & mask);
}

void usb_drv_set_test_mode(int mode)
{
    switch(mode){
        case 0:
            REG_PORTSC1 &= ~PORTSCX_PORT_TEST_CTRL;
            break;
        case 1:
            REG_PORTSC1 |= PORTSCX_PTC_JSTATE;
            break;
        case 2:
            REG_PORTSC1 |= PORTSCX_PTC_KSTATE;
            break;
        case 3:
            REG_PORTSC1 |= PORTSCX_PTC_SE0NAK;
            break;
        case 4:
            REG_PORTSC1 |= PORTSCX_PTC_PACKET;
            break;
        case 5:
            REG_PORTSC1 |= PORTSCX_PTC_FORCE_EN;
            break;
    }
    usb_drv_reset();
    REG_USBCMD |= USBCMD_RUN;
}

/*-------------------------------------------------------------------------*/
static int ep_to_pipe(int ep_num, int ep_dir)
{
    return ep_num * 2 + (ep_dir == DIR_IN ? 1 : 0);
}

int usb_drv_max_endpoint_packet_size(int ep)
{
    return endpoints[EP_NUM(ep)].max_pkt_size[EP_DIR(ep)];
}

int usb_drv_allocate_slots(int ep, int buffer_size, void *buffer)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);
    
    endpoints[ep_num].tds[ep_dir] = (struct transfer_descriptor *)buffer;
    endpoints[ep_num].nb_tds[ep_dir] = buffer_size / USB_DRV_SLOT_SIZE;
    
    return 0;
}
/* Release the slots previously allocated.
 * Returns 0 on success and <0 on error. */
int usb_drv_release_slots(int ep)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);
    
    endpoints[ep_num].tds[ep_dir] = NULL;
    endpoints[ep_num].nb_tds[ep_dir] = 0;
    
    return 0;
}

int usb_drv_select_endpoint_mode(int ep, int mode)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);
    int pipe = ep_to_pipe(ep_num, ep_dir);
    
    endpoints[ep_num].mode[ep_dir] = mode;
    if(mode == USB_DRV_ENDPOINT_MODE_QUEUE)
    {
        qh_array[pipe].head_td = -1;
    }
    else if(mode == USB_DRV_ENDPOINT_MODE_REPEAT)
    {
        
    }
    
    return 0;
}

static int usb_drv_queue_transfer(int ep_num, bool send, bool wait, void *ptr, int length)
{
    int pipe = ep_to_pipe(ep_num, send);
    int queue_size;
    int free_tds;
    int nb_tds;
    int first_td;
    int i;
    int status;
    struct transfer_descriptor *td;
    
    logf("usb: queue xfer ep=%d send=%d wait=%d ptr=0x%x len=%d", ep_num, send, wait, (unsigned int)ptr, length);
    
    if(endpoints[ep_num].mode[send] != USB_DRV_ENDPOINT_MODE_QUEUE)
    {
        panicf("usb: queue transfer on non-queue ep");
        return -1;
    }
    
    queue_size = endpoints[ep_num].nb_tds[send];
    /* first determine the number of tds needed */
    /* special case for ack packet (length=0) */
    if(length == 0)
        nb_tds = 1;
    else
        nb_tds = (length + DTD_MAX_TRANSFER_LENGTH - 1) / DTD_MAX_TRANSFER_LENGTH;
    /* check that the current has enough free tds */
    if(qh_array[pipe].head_td == -1)
        free_tds = queue_size;
    else if(qh_array[pipe].head_td <= qh_array[pipe].tail_td)
        free_tds = queue_size - qh_array[pipe].tail_td - 1 + qh_array[pipe].head_td;
    else
        free_tds = qh_array[pipe].head_td - qh_array[pipe].tail_td - 1;
    
    logf("usb: nb_tds=%d head=%d tail=%d queue=%d free_tds=%d", nb_tds, 
        qh_array[pipe].head_td, qh_array[pipe].tail_td, queue_size, free_tds);
    if(free_tds < nb_tds)
    {
        logf("usb: not enough free tds");
        return -1;
    }
    /* fill tds */
    if(qh_array[pipe].head_td == -1)
        first_td = 0;
    else
        first_td = (qh_array[pipe].tail_td + 1) % queue_size;
    
    for(i = 0; i < nb_tds; i++)
    {
        /* td length */
        int len = MIN(DTD_MAX_TRANSFER_LENGTH, length);
        /* last td of the chain ? */
        bool last_td = ((i + 1) == nb_tds);
        /* compute current td and next td indexes */
        int cur_td = (first_td + i) % queue_size;
        int next_td = (cur_td + 1) % queue_size;
        
        logf("fill td %d %d", cur_td, len);
        
        td = &endpoints[ep_num].tds[send][cur_td];
        /* FIXME td allow iso packets per frame override but we don't use it here */
        memset(td, 0, sizeof(struct transfer_descriptor));
        if(last_td)
            td->next_td_ptr = DTD_NEXT_TERMINATE;
        else
            td->next_td_ptr = (unsigned int)&endpoints[ep_num].tds[send][next_td]; 
            
        td->size_ioc_sts = (len << DTD_LENGTH_BIT_POS) | DTD_STATUS_ACTIVE;
        if(last_td)
            td->size_ioc_sts |= DTD_IOC;
        td->off_length = (len & DTD_SOFTWARE_LENGTH_MASK) | ((unsigned int)ptr & 0xfff) << DTD_SOFTWARE_OFFSET_BIT_POS;
        
        td->buff_ptr0 = (unsigned int)ptr;
        td->buff_ptr1 = ((unsigned int)ptr & 0xfffff000) + 0x1000;
        td->buff_ptr2 = ((unsigned int)ptr & 0xfffff000) + 0x2000;
        td->buff_ptr3 = ((unsigned int)ptr & 0xfffff000) + 0x3000;
        td->buff_ptr4 = ((unsigned int)ptr & 0xfffff000) + 0x4000;
        
        ptr += len;
        length -= len;
    }
    
    /* add TD to the queue */
    /* 32.14.5.4 */
    
    /* empty list */
    if(qh_array[pipe].head_td == -1)
    {
        /* setup the list */
        qh_array[pipe].head_td = 0;
        qh_array[pipe].tail_td = nb_tds - 1;
        
        logf("usb: nb_tds=%d head=%d tail=%d queue=%d", nb_tds, 
            qh_array[pipe].head_td, qh_array[pipe].tail_td, queue_size);
        /* prime endpoint */
        return prime_transfer(ep_num, &endpoints[ep_num].tds[send][0], send, wait);
    }
    else
    {
        /* add at the end of the list */
        endpoints[ep_num].tds[send][qh_array[pipe].tail_td].next_td_ptr = (unsigned int)&endpoints[ep_num].tds[send][first_td];
        qh_array[pipe].tail_td = (qh_array[pipe].tail_td + nb_tds) % queue_size;
        
        logf("usb: nb_tds=%d head=%d tail=%d queue=%d", nb_tds, 
            qh_array[pipe].head_td, qh_array[pipe].tail_td, queue_size);
        /* check prime */
        if(REG_ENDPTPRIME & pipe2mask[pipe])
            goto Lend;
        REG_USBCMD |= USBCMD_ATDTW;
        status = REG_ENDPTSTATUS & pipe2mask[pipe];
        
        if(REG_USBCMD & USBCMD_ATDTW)
        {
            REG_USBCMD &= ~USBCMD_ATDTW;
            if(status)
                goto Lend;
            else
                return prime_transfer(ep_num, &endpoints[ep_num].tds[send][first_td], send, wait);
        }
        else
        {
            REG_ENDPTPRIME |= pipe2mask[pipe];
            goto Lend;
        }
    }
    
    Lend:
    if(wait)
    {
        panicf("usb: error, how could you wait for a transfer whereas the first one in the queue is not finished ?!");
    }
    return 0;
}

static void create_repeat_list(int ep)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);
    int i;
    
    for(i = 0; i < endpoints[ep_num].nb_tds[ep_dir]; i++)
    {
        int next = (i + 1) % endpoints[ep_num].nb_tds[ep_dir];
        endpoints[ep_num].tds[ep_dir][i].next_td_ptr =
            (unsigned int)&endpoints[ep_num].tds[ep_dir][next];
    }
}

int usb_drv_fill_repeat_slot(int ep, int slot, void *ptr, int length)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);
    int pipe = ep_to_pipe(ep_num, ep_dir);
    struct transfer_descriptor *td;
    
    if(qh_array[pipe].dtd.next_td_ptr != QH_NEXT_TERMINATE)
    {
        panicf("usb: you can't fill a slot while the endpoint is active");
        return -1;
    }
    
    if(slot < 0 || slot >= endpoints[ep_num].nb_tds[ep_dir])
    {
        panicf("usb: slot index is out of bounds");
        return -1;
    }
    
    if(length > 0x4000)
    {
        panicf("usb: slot buffer is too big");
        return -1;
    }
    
    if(qh_array[pipe].dtd.next_td_ptr == QH_NEXT_TERMINATE)
        create_repeat_list(ep);
    
    td = &endpoints[ep_num].tds[ep_dir][slot];
    
    td->size_ioc_sts = (length << DTD_LENGTH_BIT_POS) | DTD_STATUS_ACTIVE | DTD_IOC;
    td->off_length = (length & DTD_SOFTWARE_LENGTH_MASK) | ((unsigned int)ptr & 0xfff) << DTD_SOFTWARE_OFFSET_BIT_POS;
    
    td->buff_ptr0 = (unsigned int)ptr;
    td->buff_ptr1 = ((unsigned int)ptr & 0xfffff000) + 0x1000;
    td->buff_ptr2 = ((unsigned int)ptr & 0xfffff000) + 0x2000;
    td->buff_ptr3 = ((unsigned int)ptr & 0xfffff000) + 0x3000;
    td->buff_ptr4 = ((unsigned int)ptr & 0xfffff000) + 0x4000;
    
    return 0;
}

int usb_drv_start_repeat(int ep)
{
    int pipe = ep_to_pipe(EP_NUM(ep), EP_DIR(ep));
    
    if(qh_array[pipe].dtd.next_td_ptr != QH_NEXT_TERMINATE)
    {
        logf("usb: endpoint is already active");
        return -1;
    }
    else
    {
        qh_array[pipe].head_td = 0;
        
        return prime_transfer(EP_NUM(ep), &endpoints[EP_NUM(ep)].tds[EP_DIR(ep)][0], EP_DIR(ep), false);
    }
}

int usb_drv_stop_repeat(int ep)
{
    usb_drv_reset_endpoint(EP_NUM(ep), EP_DIR(ep));
    /* FIXME: necessary ? */
    qh_array[ep_to_pipe(EP_NUM(ep), EP_DIR(ep))].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    qh_array[ep_to_pipe(EP_NUM(ep), EP_DIR(ep))].status = DTD_STATUS_HALTED;
    
    return 0;
}

int usb_drv_queue_send_blocking(int endpoint, void *ptr, int length)
{
    return usb_drv_queue_transfer(EP_NUM(endpoint), true, true, ptr, length);
}

int usb_drv_queue_send_nonblocking(int endpoint, void *ptr, int length)
{
    return usb_drv_queue_transfer(EP_NUM(endpoint), true, false, ptr, length);
}

int usb_drv_queue_recv_nonblocking(int endpoint, void *ptr, int length)
{
    return usb_drv_queue_transfer(EP_NUM(endpoint), false, false, ptr, length);
}

int usb_drv_queue_recv_blocking(int endpoint, void *ptr, int length)
{
    return usb_drv_queue_transfer(EP_NUM(endpoint), false, true, ptr, length);
}

/* manual: 32.14.5.2 */
static int prime_transfer(int ep_num, struct transfer_descriptor *new_td, bool send, bool wait)
{
    int rc = 0;
    int pipe = ep_to_pipe(ep_num, send);
    unsigned int mask = pipe2mask[pipe];
    struct queue_head* qh = &qh_array[pipe];
    static long last_tick;

    int oldlevel = disable_irq_save();

    logf("prime endpoint");
    qh->status = 0;
    qh->wait = wait;
    qh->dtd.next_td_ptr = (unsigned int)new_td;
    qh->dtd.size_ioc_sts &= ~(QH_STATUS_HALT | QH_STATUS_ACTIVE);

    REG_ENDPTPRIME |= mask;

    if(ep_num == EP_CONTROL && (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0)) {
        /* 32.14.3.2.2 */
        logf("new setup arrived");
        rc = -4;
        goto pt_error;
    }

    last_tick = current_tick;
    while ((REG_ENDPTPRIME & mask)) {
        if (REG_USBSTS & USBSTS_RESET) {
            rc = -1;
            goto pt_error;
        }

        if (TIME_AFTER(current_tick, last_tick + HZ/4)) {
            logf("prime timeout");
            rc = -2;
            goto pt_error;
        }
    }

    if (!(REG_ENDPTSTATUS & mask)) {
        if(REG_ENDPTCOMPLETE & mask)
        {
            logf("endpoint completed fast! %d %d %x", ep_num, pipe, qh->dtd.size_ioc_sts & 0xff);
        }
        else
        {
            logf("no prime! %d %d %x", ep_num, pipe, qh->dtd.size_ioc_sts & 0xff);
            rc = -3;
            goto pt_error;
        }
    }
    if(ep_num == EP_CONTROL && (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0)) {
        /* 32.14.3.2.2 */
        logf("new setup arrived");
        rc = -4;
        goto pt_error;
    }

    restore_irq(oldlevel);

    if (wait) {
        /* wait for transfer to finish */
        wakeup_wait(&transfer_completion_signal[pipe], TIMEOUT_BLOCK);
        //logf("all tds done");
    }

pt_error:
    if(rc<0)
        restore_irq(oldlevel);

    /* Error status must make sure an abandoned wakeup signal isn't left */
    if (rc < 0 && wait) {
        /* Make sure to remove any signal if interrupt fired before we zeroed
         * qh->wait. Could happen during a bus reset for example. */
        wakeup_wait(&transfer_completion_signal[pipe], TIMEOUT_NOBLOCK);
    }

    return rc;
}

void usb_drv_cancel_all_transfers(void)
{
    int i;
    REG_ENDPTFLUSH = ~0;
    while (REG_ENDPTFLUSH);

    /* BUG to implement */
    for(i = 0; i < USB_NUM_ENDPOINTS * 2; i++) {
        if(endpoints[i / 2].mode[i % 2] == USB_DRV_ENDPOINT_MODE_QUEUE)
            qh_array[i].head_td = -1;
        
        if(qh_array[i].wait) {
            qh_array[i].wait = 0;
            qh_array[i].status = DTD_STATUS_HALTED;
            wakeup_signal(&transfer_completion_signal[i]);
        }
    }
}

int usb_drv_request_endpoint(int type, int dir)
{
    int ep_num, ep_dir;
    short ep_type;

    /* Safety */
    ep_dir = EP_DIR(dir);
    ep_type = type & USB_ENDPOINT_XFERTYPE_MASK;

    logf("req: %s %s", XFER_DIR_STR(ep_dir), XFER_TYPE_STR(ep_type));

    /* Find an available ep/dir pair */
    for (ep_num=1;ep_num<USB_NUM_ENDPOINTS;ep_num++) {
        usb_endpoint_t* endpoint=&endpoints[ep_num];
        int other_dir=(ep_dir ? 0:1);

        if (endpoint->allocated[ep_dir])
            continue;

        if (endpoint->allocated[other_dir] &&
                endpoint->type[other_dir] != ep_type) {
            logf("ep of different type!");
            continue;
        }


        endpoint->allocated[ep_dir] = 1;
        endpoint->type[ep_dir] = ep_type;

        log_ep(ep_num, ep_dir, "add");
        return (ep_num | (dir & USB_ENDPOINT_DIR_MASK));
    }

    return -1;
}

void usb_drv_release_endpoint(int ep)
{
    int ep_num = EP_NUM(ep);
    int ep_dir = EP_DIR(ep);

    log_ep(ep_num, ep_dir, "rel");
    endpoints[ep_num].allocated[ep_dir] = 0;
}

static void control_received(void)
{
    int i;
    /* copy setup data from packet */
    static unsigned int tmp[2];
    tmp[0] = qh_array[0].setup_buffer[0];
    tmp[1] = qh_array[0].setup_buffer[1];

    /* acknowledge packet recieved */
    REG_ENDPTSETUPSTAT = EPSETUP_STATUS_EP0;

    /* Stop pending control transfers */
    for(i = 0 ; i < 2; i++) {
        if(qh_array[i].wait) {
            qh_array[i].wait = 0;
            qh_array[i].status = DTD_STATUS_HALTED;
            wakeup_signal(&transfer_completion_signal[i]);
        }
    }

    usb_core_control_request((struct usb_ctrlrequest*)tmp);
}

static void transfer_completed(void)
{
    int ep;
    unsigned int mask = REG_ENDPTCOMPLETE;
    REG_ENDPTCOMPLETE = mask;

    logf("xfer");

    for (ep=0; ep<USB_NUM_ENDPOINTS; ep++) {
        int dir;
        for (dir=0; dir<2; dir++) {
            int pipe = ep * 2 + dir;
            if (mask & pipe2mask[pipe]) {
                struct queue_head* qh = &qh_array[pipe];
                struct transfer_descriptor* td;
                void *buf;
                
                /* several transfers can finish on the same endpoints, especially on an iso endpoint
                 * in repeat mode, for this repeat, loop until all finished transfers are done */
                while(true)
                {
                    int length=0;
                    
                    if(endpoints[ep].mode[dir] == USB_DRV_ENDPOINT_MODE_QUEUE ||
                            endpoints[ep].mode[dir] == USB_DRV_ENDPOINT_MODE_REPEAT)
                    {
                        /* stop here if there are not more expected transfers */
                        if(qh->head_td == -1)
                            break;
                        
                        td = &endpoints[ep].tds[dir][qh->head_td];
                    }
                    else
                    {
                        panicf("usb: unimplemented transfer type");
                        td = NULL;
                        break;
                    }
                    
                    /* the lower bits of buff_ptr0 are changed by the controller, so use the offset
                     * we saved previously */
                    buf = (unsigned char *)((td->buff_ptr0 & 0xfffff000) | 
                            (td->off_length & DTD_SOFTWARE_OFFSET_MASK) >> DTD_SOFTWARE_OFFSET_BIT_POS);
                    
                    while(true)
                    {
                        logf("td=0x%x length=%d remaining=%d ptr=0x%x", (unsigned int)td, td->off_length & DTD_SOFTWARE_LENGTH_MASK,
                            (td->size_ioc_sts & DTD_PACKET_SIZE) >> DTD_LENGTH_BIT_POS, (unsigned int)buf);
                        length += (td->off_length & DTD_SOFTWARE_LENGTH_MASK) -
                            ((td->size_ioc_sts & DTD_PACKET_SIZE) >> DTD_LENGTH_BIT_POS);
                        /* It seems that the controller sets the pipe bit to one even if the TD
                         * dosn't have the IOC bit set. So we have the rely the active status bit
                         * to check that all the TDs of the transfer are really finished and let
                         * the transfer continue if it's no the case */
                        if(td->size_ioc_sts & DTD_STATUS_ACTIVE)
                        {
                            logf("skip half finished transfer");
                            goto Lskip;
                        }
                        
                        #if 1
                        if(td->size_ioc_sts & DTD_STATUS_DATA_BUFF_ERR) _logf("usb: data buffer error");
                        if(td->size_ioc_sts & DTD_STATUS_HALTED) _logf("usb: halted");
                        if(td->size_ioc_sts & DTD_STATUS_TRANSACTION_ERR) _logf("usb: transaction error");
                        #endif
                        
                        if(td->size_ioc_sts & DTD_IOC)
                            break;
                        else
                            td = (struct transfer_descriptor*) td->next_td_ptr;
                    }
                    logf("usb: xfer complete on EP%d %s: len=%d", ep, dir == DIR_IN ? "IN" : "OUT", length);
                    
                    if(endpoints[ep].mode[dir] == USB_DRV_ENDPOINT_MODE_QUEUE)
                    {
                        logf("usb: xfer complete head=%d tail=%d",
                            qh->head_td, qh->tail_td);
                        
                        if(td->next_td_ptr & DTD_NEXT_TERMINATE)
                        {
                            /* list is now empty */
                            qh->head_td = -1;
                        }
                        else
                        {
                            /* update head */
                            /* FIXME: this is really ugly */
                            qh->head_td = (td->next_td_ptr - (unsigned int)endpoints[ep].tds[dir]) / 
                                sizeof(struct transfer_descriptor);
                        }
                        
                        logf("usb: xfer complete head=%d tail=%d",
                            qh->head_td, qh->tail_td);
                        if(qh->wait)
                        {
                            qh->wait=0;
                            wakeup_signal(&transfer_completion_signal[pipe]);
                        }
                        usb_core_transfer_complete(ep, dir ? USB_DIR_IN : USB_DIR_OUT, 0, length, buf);
                    }
                    else if(endpoints[ep].mode[dir] == USB_DRV_ENDPOINT_MODE_REPEAT)
                    {
                        /* NOTE: in case of repeat, a transfer can't take more than one slot=TD
                         * so td still points to the first=last td of the transfer */
                        /* set page offset because the controller probably modified it */
                        td->buff_ptr0 = (td->buff_ptr0 & 0xfffff000) | 
                            ((td->off_length & DTD_SOFTWARE_OFFSET_MASK) >> DTD_SOFTWARE_OFFSET_BIT_POS);
                        /* set TD as active once more, restore length because it was changed by controller */
                        td->size_ioc_sts = (td->off_length & DTD_SOFTWARE_LENGTH_MASK) << DTD_LENGTH_BIT_POS 
                            | DTD_STATUS_ACTIVE | DTD_IOC;
                        
                        /* go to next TD */
                        qh_array[pipe].head_td = (qh_array[pipe].head_td + 1) % endpoints[ep].nb_tds[dir];
                        
                        /* check if next TD is also finished */
                        td = &endpoints[ep].tds[dir][qh->head_td];
                        
                        /* call directly completion handler to save time: the queue system is too slow */
                        usb_core_fast_transfer_complete(ep, dir ? USB_DIR_IN : USB_DIR_OUT, 0, length, buf);
                    }
                    
                    /* perhaps next transfer is complete also */
                    continue;
                    /* this transfer is not complete so stop here */
                    Lskip:
                    break;
                }
            }
        }
    }
}

/* manual: 32.14.2.1 Bus Reset */
static void bus_reset(void)
{
    int i;
    logf("usb bus_reset");

    REG_DEVICEADDR = 0;
    REG_ENDPTSETUPSTAT = REG_ENDPTSETUPSTAT;
    REG_ENDPTCOMPLETE  = REG_ENDPTCOMPLETE;

    for (i=0; i<100; i++) {
        if (!REG_ENDPTPRIME)
            break;

        if (REG_USBSTS & USBSTS_RESET) {
            logf("usb: double reset");
            return;
        }

        udelay(100);
    }
    if (REG_ENDPTPRIME) {
        logf("usb: short reset timeout");
    }

    usb_drv_cancel_all_transfers();

    if (!(REG_PORTSC1 & PORTSCX_PORT_RESET)) {
        logf("usb: slow reset!");
    }
}

/* manual: 32.14.4.1 Queue Head Initialization */
static void init_control_queue_heads(void)
{
    memset(qh_array, 0, sizeof qh_array);

    /*** control ***/
    qh_array[EP_CONTROL].max_pkt_length = 64 << QH_MAX_PKT_LEN_POS | QH_IOS;
    qh_array[EP_CONTROL].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    qh_array[EP_CONTROL+1].max_pkt_length = 64 << QH_MAX_PKT_LEN_POS;
    qh_array[EP_CONTROL+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;
}
/* manual: 32.14.4.1 Queue Head Initialization */
static void init_queue_heads(void)
{
    int i;

    /* TODO: this should take ep_allocation into account */
    for (i=1;i<USB_NUM_ENDPOINTS;i++) {
        int packetsize = (usb_drv_port_speed() ? 512 : 64);
        
        if(endpoints[i].type[DIR_OUT] == USB_ENDPOINT_XFER_ISOC)
            packetsize = (usb_drv_port_speed() ? 1024 : 1023);
        
        endpoints[i].max_pkt_size[DIR_IN] = packetsize;
        endpoints[i].max_pkt_size[DIR_OUT] = packetsize;
        /* OUT */
        if(endpoints[i].type[DIR_OUT] == USB_ENDPOINT_XFER_ISOC)
            /* FIXME: we can adjust the number of packets per frame, currently use one */
            qh_array[i*2].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL | 1 << QH_MULT_POS;
        else
            qh_array[i*2].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL;
        
        qh_array[i*2].dtd.next_td_ptr = QH_NEXT_TERMINATE;
        
        /* IN */
        if(endpoints[i].type[DIR_IN] == USB_ENDPOINT_XFER_ISOC)
            /* FIXME: we can adjust the number of packets per frame, currently use one */
            qh_array[i*2+1].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL | 3 << QH_MULT_POS;
        else
            qh_array[i*2+1].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL;
        
        qh_array[i*2+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    }
}

static void init_endpoints(void)
{
    int ep_num;

    logf("init_endpoints");
    /* RX/TX from the device POV: OUT/IN, respectively */
    for(ep_num=1;ep_num<USB_NUM_ENDPOINTS;ep_num++) {
        usb_endpoint_t *endpoint = &endpoints[ep_num];

        /* manual: 32.9.5.18 (Caution): Leaving an unconfigured endpoint control
         * will cause undefined behavior for the data pid tracking on the active
         * endpoint/direction. */
        if (!endpoint->allocated[DIR_OUT])
            endpoint->type[DIR_OUT] = USB_ENDPOINT_XFER_BULK;
        if (!endpoint->allocated[DIR_IN])
            endpoint->type[DIR_IN] = USB_ENDPOINT_XFER_BULK;

        REG_ENDPTCTRL(ep_num) =
            EPCTRL_RX_DATA_TOGGLE_RST | EPCTRL_RX_ENABLE |
            EPCTRL_TX_DATA_TOGGLE_RST | EPCTRL_TX_ENABLE |
            (endpoint->type[DIR_OUT] << EPCTRL_RX_EP_TYPE_SHIFT) |
            (endpoint->type[DIR_IN] << EPCTRL_TX_EP_TYPE_SHIFT);
    }
}

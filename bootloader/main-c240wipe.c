/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: main-e200r-installer.c 15599 2007-11-12 18:49:53Z amiconn $
 *
 * Copyright (C) 2006 by Barry Wardell
 *
 * Based on Rockbox iriver bootloader by Linus Nielsen Feltzing
 * and the ipodlinux bootloader by Daniel Palffy and Bernard Leach
 * 
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "cpu.h"
#include "file.h"
#include "system.h"
#include "kernel.h"
#include "lcd.h"
#include "font.h"
#include "storage.h"
#include "button.h"
#include "disk.h"
#include "crc32-mi4.h"
#include <string.h>
#include "i2c.h"
#include "backlight-target.h"
#include "power.h"


unsigned char mbr[] = {
    0x33, 0xc0, 0x8e, 0xd0, 0xbc, 0x00, 0x7c, 0xfb, 0x50, 0x07, 0x50, 0x1f, 0xfc, 0xbe, 0x1b, 0x7c, 
    0xbf, 0x1b, 0x06, 0x50, 0x57, 0xb9, 0xe5, 0x01, 0xf3, 0xa4, 0xcb, 0xbe, 0xbe, 0x07, 0xb1, 0x04, 
    0x38, 0x2c, 0x7c, 0x09, 0x75, 0x15, 0x83, 0xc6, 0x10, 0xe2, 0xf5, 0xcd, 0x18, 0x8b, 0x14, 0x8b, 
    0xee, 0x83, 0xc6, 0x10, 0x49, 0x74, 0x16, 0x38, 0x2c, 0x74, 0xf6, 0xbe, 0x10, 0x07, 0x4e, 0xac, 
    0x3c, 0x00, 0x74, 0xfa, 0xbb, 0x07, 0x00, 0xb4, 0x0e, 0xcd, 0x10, 0xeb, 0xf2, 0x89, 0x46, 0x25, 
    0x96, 0x8a, 0x46, 0x04, 0xb4, 0x06, 0x3c, 0x0e, 0x74, 0x11, 0xb4, 0x0b, 0x3c, 0x0c, 0x74, 0x05, 
    0x3a, 0xc4, 0x75, 0x2b, 0x40, 0xc6, 0x46, 0x25, 0x06, 0x75, 0x24, 0xbb, 0xaa, 0x55, 0x50, 0xb4, 
    0x41, 0xcd, 0x13, 0x58, 0x72, 0x16, 0x81, 0xfb, 0x55, 0xaa, 0x75, 0x10, 0xf6, 0xc1, 0x01, 0x74, 
    0x0b, 0x8a, 0xe0, 0x88, 0x56, 0x24, 0xc7, 0x06, 0xa1, 0x06, 0xeb, 0x1e, 0x88, 0x66, 0x04, 0xbf, 
    0x0a, 0x00, 0xb8, 0x01, 0x02, 0x8b, 0xdc, 0x33, 0xc9, 0x83, 0xff, 0x05, 0x7f, 0x03, 0x8b, 0x4e, 
    0x25, 0x03, 0x4e, 0x02, 0xcd, 0x13, 0x72, 0x29, 0xbe, 0x75, 0x07, 0x81, 0x3e, 0xfe, 0x7d, 0x55, 
    0xaa, 0x74, 0x5a, 0x83, 0xef, 0x05, 0x7f, 0xda, 0x85, 0xf6, 0x75, 0x83, 0xbe, 0x3f, 0x07, 0xeb, 
    0x8a, 0x98, 0x91, 0x52, 0x99, 0x03, 0x46, 0x08, 0x13, 0x56, 0x0a, 0xe8, 0x12, 0x00, 0x5a, 0xeb, 
    0xd5, 0x4f, 0x74, 0xe4, 0x33, 0xc0, 0xcd, 0x13, 0xeb, 0xb8, 0x00, 0x00, 0x80, 0x32, 0x22, 0x16, 
    0x56, 0x33, 0xf6, 0x56, 0x56, 0x52, 0x50, 0x06, 0x53, 0x51, 0xbe, 0x10, 0x00, 0x56, 0x8b, 0xf4, 
    0x50, 0x52, 0xb8, 0x00, 0x42, 0x8a, 0x56, 0x24, 0xcd, 0x13, 0x5a, 0x58, 0x8d, 0x64, 0x10, 0x72, 
    0x0a, 0x40, 0x75, 0x01, 0x42, 0x80, 0xc7, 0x02, 0xe2, 0xf7, 0xf8, 0x5e, 0xc3, 0xeb, 0x74, 0x49, 
    0x6e, 0x76, 0x61, 0x6c, 0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6f, 0x6e, 
    0x20, 0x74, 0x61, 0x62, 0x6c, 0x65, 0x2e, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70, 0x20, 0x63, 0x61, 
    0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x63, 0x6f, 0x6e, 0x74, 0x69, 0x6e, 0x75, 0x65, 0x2e, 0x00, 0x45, 
    0x72, 0x72, 0x6f, 0x72, 0x20, 0x6c, 0x6f, 0x61, 0x64, 0x69, 0x6e, 0x67, 0x20, 0x6f, 0x70, 0x65, 
    0x72, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x20, 0x53, 
    0x65, 0x74, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x63, 0x6f, 0x6e, 0x74, 
    0x69, 0x6e, 0x75, 0x65, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x8b, 0xfc, 0x1e, 0x57, 0x8b, 0xf5, 0xcb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 
    0x10, 0x00, 0x06, 0x59, 0x1f, 0x7a, 0xff, 0x03, 0x00, 0x00, 0x01, 0xfa, 0x1d, 0x00, 0x00, 0x59, 
    0x20, 0x7a, 0x84, 0xe5, 0x29, 0x7c, 0x00, 0xfe, 0x1d, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

unsigned char fat[]={
0xeb,0x3c,0x90,0x6d,0x6b,0x64,0x6f,0x73,0x66,0x73,0x00,0x00,0x02,0x20,0x01,0x00,
0x02,0x00,0x02,0x00,0x00,0xf8,0xf0,0x00,0x20,0x00,0x40,0x00,0x00,0x00,0x00,0x00,
0x00,0xfa,0x1d,0x00,0x00,0x00,0x29,0x3d,0xa7,0x07,0x48,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x46,0x41,0x54,0x31,0x36,0x20,0x20,0x20,0x0e,0x1f,
0xbe,0x5b,0x7c,0xac,0x22,0xc0,0x74,0x0b,0x56,0xb4,0x0e,0xbb,0x07,0x00,0xcd,0x10,
0x5e,0xeb,0xf0,0x32,0xe4,0xcd,0x16,0xcd,0x19,0xeb,0xfe,0x54,0x68,0x69,0x73,0x20,
0x69,0x73,0x20,0x6e,0x6f,0x74,0x20,0x61,0x20,0x62,0x6f,0x6f,0x74,0x61,0x62,0x6c,
0x65,0x20,0x64,0x69,0x73,0x6b,0x2e,0x20,0x20,0x50,0x6c,0x65,0x61,0x73,0x65,0x20,
0x69,0x6e,0x73,0x65,0x72,0x74,0x20,0x61,0x20,0x62,0x6f,0x6f,0x74,0x61,0x62,0x6c,
0x65,0x20,0x66,0x6c,0x6f,0x70,0x70,0x79,0x20,0x61,0x6e,0x64,0x0d,0x0a,0x70,0x72,
0x65,0x73,0x73,0x20,0x61,0x6e,0x79,0x20,0x6b,0x65,0x79,0x20,0x74,0x6f,0x20,0x74,
0x72,0x79,0x20,0x61,0x67,0x61,0x69,0x6e,0x20,0x2e,0x2e,0x2e,0x20,0x0d,0x0a,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x55,0xaa,
};


unsigned char backupfat[] = {
    0xf8, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define NUM_E200_CRCS ((int)((sizeof(e200_crcs) / sizeof(uint32_t))))

#define REG_USBCMD           (*(volatile unsigned int *)(USB_BASE+0x140))
#define REG_USBSTS           (*(volatile unsigned int *)(USB_BASE+0x144))
#define REG_CONFIGFLAG       (*(volatile unsigned int *)(USB_BASE+0x180))
#define REG_PORTSC1          (*(volatile unsigned int *)(USB_BASE+0x184))
#define REG_OTGSC            (*(volatile unsigned int *)(USB_BASE+0x1a4))
#define REG_USBMODE          (*(volatile unsigned int *)(USB_BASE+0x1a8))

unsigned char zero[1024*16];



void* main(void)
{
    int i;
    int btn;

    chksum_crc32gentab ();

    system_init();
    kernel_init();
    lcd_init();
    font_init();
    button_init();
    i2c_init();
    _backlight_on();
    
    lcd_set_foreground(LCD_WHITE);
    lcd_set_background(LCD_BLACK);
    lcd_clear_display();

    btn = button_read_device();
    verbose = true;

    lcd_setfont(FONT_SYSFIXED);

    printf("Rockbox c240 initializer");
    printf("");


    i=storage_init();
    disk_init(IF_MV(0));

    memset(zero,0,16*1024);
    printf("Zeroing flash");
    for(i=0;i<250816;i++)
    {
       storage_write_sectors(0,i*32,32,zero);
       if(i%64 == 0)
       {
           printf("%d kB left",(250816-i)/2);
       }
    }

    printf("Writing MBR");
    storage_write_sectors(0,0,1,mbr);
    printf("Writing FAT bootsector");
    storage_write_sectors(0,1023,1,fat);
    printf("Writing more FAT");
    storage_write_sectors(0,1024,1,backupfat);
    printf("Writing more FAT");
    storage_write_sectors(0,1264,1,backupfat);
    if (button_hold())
        printf("Release Hold and");

    printf("Press any key to shutdown");

    while(button_read_device() == BUTTON_NONE);

    power_off();

    return NULL;
}

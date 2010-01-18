/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Björn Stenberg <bjorn@haxx.se>
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
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "thread-sdl.h"

#include "debug.h"

#include "screens.h"
#include "button.h"

#include "string.h"
#include "lcd.h"

#include "power.h"

#include "ata.h" /* for volume definitions */

#include "usb.h" /* for usb_detect and USB_EXTRACTED */

extern char having_new_lcd;

#if CONFIG_CODEC != SWCODEC
void audio_set_buffer_margin(int seconds)
{
     (void)seconds;
}
#endif

int fat_startsector(void)
{
    return 63;
}

bool fat_ismounted(int volume)
{
    (void)volume;
    return true;
}

void fat_init(void)
{
}

int fat_mount(IF_MV2(int volume,) IF_MD2(int drive,) long startsector)
{
    IF_MV((void)volume);
    IF_MD((void)drive);
    (void)startsector;
    /* fails nicely */
    return -1;
}

void rtc_init(void)
{
}

int rtc_read(int address)
{
    return address ^ 0x55;
}

int rtc_write(int address, int value)
{
    (void)address;
    (void)value;
    return 0;
}

int rtc_read_datetime(struct tm *tm)
{
    time_t now = time(NULL);
    *tm = *localtime(&now);

    return 0;
}

int rtc_write_datetime(const struct tm *tm)
{
    (void)tm;
    return 0;
}

#ifdef HAVE_RTC_ALARM
void rtc_get_alarm(int *h, int *m)
{
    *h = 11;
    *m = 55;
}

void rtc_set_alarm(int h, int m)
{
    (void)h;
    (void)m;
}

bool rtc_enable_alarm(bool enable)
{
    return enable;
}

extern bool sim_alarm_wakeup;
bool rtc_check_alarm_started(bool release_alarm)
{
    (void)release_alarm;
    return sim_alarm_wakeup;
}

bool rtc_check_alarm_flag(void)
{
    return true;
}
#endif

#ifdef HAVE_HEADPHONE_DETECTION
bool headphones_inserted(void)
{
    return true;
}
#endif

#ifdef HAVE_SPDIF_POWER
void spdif_power_enable(bool on)
{
   (void)on;
}

bool spdif_powered(void)
{
    return false;
}
#endif

bool is_new_player(void)
{
    return having_new_lcd;
}

#ifdef HAVE_USB_POWER
bool usb_charging_enable(bool on)
{
    (void)on;
    return false;
}
#endif

#if CONFIG_CHARGING
bool charger_inserted(void)
{
    return false;
}

bool power_input_present(void)
{
    return false;
}

unsigned int power_input_status(void)
{
#ifdef HAVE_BATTERY_SWITCH
    return POWER_INPUT_BATTERY;
#else
    return POWER_INPUT_NONE;
#endif
}

bool charging_state(void)
{
    return false;
}
#endif /* CONFIG_CHARGING */

#ifdef HAVE_REMOTE_LCD_TICKING
void lcd_remote_emireduce(bool state)
{
    (void)state;
}
#endif

void lcd_set_contrast( int x )
{
    (void)x;
}

void lcd_init_device(void)
{
}

void mpeg_set_pitch(int pitch)
{
    (void)pitch;
}

static int sleeptime;
void set_sleep_timer(int seconds)
{
    sleeptime = seconds;
}

int get_sleep_timer(void)
{
    return sleeptime;
}

#ifdef HAVE_LCD_CHARCELLS
void lcd_clearrect (int x, int y, int nx, int ny)
{
  /* Reprint char if you want to change anything */
  (void)x;
  (void)y;
  (void)nx;
  (void)ny;
}

void lcd_fillrect (int x, int y, int nx, int ny)
{
  /* Reprint char if you want to change display anything */
  (void)x;
  (void)y;
  (void)nx;
  (void)ny;
}
#endif

void cpu_sleep(bool enabled)
{
    (void)enabled;
}

void button_set_flip(bool yesno)
{
    (void)yesno;
}

#ifdef HAVE_TOUCHPAD_SENSITIVITY_SETTING
void touchpad_set_sensitivity(int level)
{
    (void)level;
}
#endif

void system_exception_wait(void)
{
    thread_sdl_exception_wait();
}

void system_reboot(void)
{
    thread_sdl_exception_wait();
}

#ifdef USB_FIREWIRE_HANDLING
bool firewire_detect(void)
{
    return false;
}
#endif

#ifndef HAVE_USBSTACK
/* these are only called in usb.c is usb slave mode which is not handled by the simulator usb drive, so stub them */
int storage_soft_reset(void)
{
    return 0;
}

void storage_enable(bool on)
{
    (void)on;
}

void usb_enable(bool on)
{
    (void)on;
}

void usb_init_device(void)
{
}

int usb_detect(void)
{
    return USB_EXTRACTED;
}
#endif

/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2006 Jonathan Gordon
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

/* Button Code Definitions for the Philips SA9200 target */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "action.h"
#include "button.h"
#include "settings.h"

/*
 * The format of the list is as follows
 * { Action Code,   Button code,    Prereq button code }
 * if there's no need to check the previous button's value, use BUTTON_NONE
 * Insert LAST_ITEM_IN_LIST at the end of each mapping
 */

/* CONTEXT_CUSTOM's used in this file...

CONTEXT_CUSTOM|CONTEXT_TREE = the standard list/tree defines (without directions)
CONTEXT_CUSTOM|CONTEXT_SETTINGS = the direction keys for the eq/col picker screens
                                  i.e where up/down is inc/dec
               CONTEXT_SETTINGS = up/down is prev/next, l/r is inc/dec

*/


static const struct button_mapping button_context_standard[]  = {
    { ACTION_STD_PREV,          BUTTON_UP,                      BUTTON_NONE },
    { ACTION_STD_PREVREPEAT,    BUTTON_UP|BUTTON_REPEAT,        BUTTON_NONE },
    { ACTION_STD_NEXT,          BUTTON_DOWN,                    BUTTON_NONE },
    { ACTION_STD_NEXTREPEAT,    BUTTON_DOWN|BUTTON_REPEAT,      BUTTON_NONE },

    { ACTION_STD_CANCEL,        BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_STD_CANCEL,        BUTTON_PREV,                    BUTTON_NONE },
    { ACTION_STD_CANCEL,        BUTTON_POWER,                   BUTTON_NONE },

    { ACTION_STD_OK,            BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_STD_OK,            BUTTON_PLAY|BUTTON_REL,         BUTTON_PLAY },

    { ACTION_STD_MENU,          BUTTON_MENU|BUTTON_REL,         BUTTON_MENU },
    { ACTION_STD_QUICKSCREEN,   BUTTON_MENU|BUTTON_REPEAT,      BUTTON_MENU },
    { ACTION_STD_CONTEXT,       BUTTON_PLAY|BUTTON_REPEAT,      BUTTON_PLAY },

    LAST_ITEM_IN_LIST
}; /* button_context_standard */


static const struct button_mapping button_context_wps[]  = {
    { ACTION_WPS_PLAY,          BUTTON_PLAY|BUTTON_REL,         BUTTON_PLAY },
    { ACTION_WPS_STOP,          BUTTON_POWER|BUTTON_REL,        BUTTON_POWER },

    { ACTION_WPS_SKIPNEXT,      BUTTON_RIGHT|BUTTON_REL,        BUTTON_RIGHT },
    { ACTION_WPS_SKIPPREV,      BUTTON_LEFT|BUTTON_REL,         BUTTON_LEFT },

    { ACTION_WPS_SEEKBACK,      BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_WPS_SEEKFWD,       BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },

    { ACTION_WPS_STOPSEEK,      BUTTON_LEFT|BUTTON_REL,         BUTTON_LEFT|BUTTON_REPEAT },
    { ACTION_WPS_STOPSEEK,      BUTTON_RIGHT|BUTTON_REL,        BUTTON_RIGHT|BUTTON_REPEAT },

    { ACTION_WPS_ABSETB_NEXTDIR,BUTTON_POWER|BUTTON_NEXT,       BUTTON_POWER },
    { ACTION_WPS_ABSETA_PREVDIR,BUTTON_POWER|BUTTON_PREV,       BUTTON_POWER },
    { ACTION_WPS_ABRESET,       BUTTON_POWER|BUTTON_UP,         BUTTON_POWER },

    { ACTION_WPS_VOLDOWN,       BUTTON_DOWN|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_WPS_VOLDOWN,       BUTTON_DOWN,                    BUTTON_NONE },
    { ACTION_WPS_VOLDOWN,       BUTTON_VOL_DOWN,                BUTTON_NONE },
    { ACTION_WPS_VOLDOWN,       BUTTON_VOL_DOWN|BUTTON_REPEAT,  BUTTON_NONE },

    { ACTION_WPS_VOLUP,         BUTTON_UP|BUTTON_REPEAT,        BUTTON_NONE },
    { ACTION_WPS_VOLUP,         BUTTON_UP,                      BUTTON_NONE },
    { ACTION_WPS_VOLUP,         BUTTON_VOL_UP|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_WPS_VOLUP,         BUTTON_VOL_UP,                  BUTTON_NONE },

    { ACTION_WPS_MENU,          BUTTON_MENU|BUTTON_REL,         BUTTON_MENU },
    { ACTION_WPS_BROWSE,        BUTTON_PREV|BUTTON_REL,         BUTTON_PREV },

    { ACTION_WPS_CONTEXT,       BUTTON_PLAY|BUTTON_REPEAT,      BUTTON_PLAY },
    { ACTION_WPS_QUICKSCREEN,   BUTTON_MENU|BUTTON_REPEAT,      BUTTON_MENU },
    { ACTION_WPS_PITCHSCREEN,   BUTTON_NEXT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_WPS_VIEW_PLAYLIST, BUTTON_NEXT|BUTTON_REL,         BUTTON_NEXT },
    { ACTION_WPS_ID3SCREEN,     BUTTON_PREV|BUTTON_REPEAT,      BUTTON_NONE },

    LAST_ITEM_IN_LIST
}; /* button_context_wps */

static const struct button_mapping button_context_list[]  = {
    { ACTION_LISTTREE_PGUP,     BUTTON_VOL_UP|BUTTON_UP,        BUTTON_VOL_UP },
    { ACTION_LISTTREE_PGDOWN,   BUTTON_VOL_DOWN|BUTTON_DOWN,    BUTTON_VOL_DOWN },

#ifdef HAVE_VOLUME_IN_LIST
    { ACTION_LIST_VOLUP,        BUTTON_VOL_UP|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_LIST_VOLUP,        BUTTON_VOL_UP,                  BUTTON_NONE },

    { ACTION_LIST_VOLDOWN,      BUTTON_VOL_DOWN,                BUTTON_NONE },
    { ACTION_LIST_VOLDOWN,      BUTTON_VOL_DOWN|BUTTON_REPEAT,  BUTTON_NONE },
#endif
    
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_list */

static const struct button_mapping button_context_tree[]  = {
    { ACTION_TREE_WPS,          BUTTON_PREV|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_TREE_STOP,         BUTTON_NEXT|BUTTON_REPEAT,      BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_LIST)
}; /* button_context_tree */

static const struct button_mapping button_context_listtree_scroll_with_combo[]  = {
    { ACTION_TREE_ROOT_INIT,    BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },

    { ACTION_TREE_PGLEFT,       BUTTON_PREV,                    BUTTON_NONE },
    { ACTION_TREE_PGLEFT,       BUTTON_PREV|BUTTON_REPEAT,      BUTTON_NONE },

    { ACTION_TREE_PGRIGHT,      BUTTON_NEXT,                    BUTTON_NONE },
    { ACTION_TREE_PGRIGHT,      BUTTON_NEXT|BUTTON_REPEAT,      BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_CUSTOM|CONTEXT_TREE),
};

static const struct button_mapping button_context_listtree_scroll_without_combo[]  = {
    { ACTION_NONE,              BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_NONE,              BUTTON_RIGHT,                   BUTTON_NONE },

    { ACTION_STD_OK,            BUTTON_RIGHT|BUTTON_REL,        BUTTON_RIGHT },
    { ACTION_STD_CANCEL,        BUTTON_LEFT|BUTTON_REL,         BUTTON_LEFT },
    { ACTION_TREE_ROOT_INIT,    BUTTON_MENU,                    BUTTON_NONE },

    { ACTION_TREE_PGLEFT,       BUTTON_PREV,                    BUTTON_NONE },
    { ACTION_TREE_PGLEFT,       BUTTON_PREV|BUTTON_REL,         BUTTON_PREV|BUTTON_REPEAT },

    { ACTION_TREE_PGRIGHT,      BUTTON_NEXT|BUTTON_REPEAT,      BUTTON_NONE  },
    { ACTION_TREE_PGRIGHT,      BUTTON_NEXT|BUTTON_REL,         BUTTON_NEXT|BUTTON_REPEAT },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_CUSTOM|CONTEXT_TREE),
};

static const struct button_mapping button_context_settings[]  = {
    { ACTION_STD_CANCEL,        BUTTON_POWER,                   BUTTON_NONE },
    { ACTION_SETTINGS_RESET,    BUTTON_PLAY,                    BUTTON_NONE },

    { ACTION_SETTINGS_INC,      BUTTON_UP,                      BUTTON_NONE },
    { ACTION_SETTINGS_INCREPEAT,BUTTON_UP|BUTTON_REPEAT,        BUTTON_NONE },

    { ACTION_SETTINGS_DEC,      BUTTON_DOWN,                    BUTTON_NONE },
    { ACTION_SETTINGS_DECREPEAT,BUTTON_DOWN|BUTTON_REPEAT,      BUTTON_NONE },
   
    { ACTION_STD_PREV,          BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_STD_PREVREPEAT,    BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    
    { ACTION_STD_NEXT,          BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_STD_NEXTREPEAT,    BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_settings */

static const struct button_mapping button_context_settings_right_is_inc[]  = {
    { ACTION_SETTINGS_INC,      BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_SETTINGS_INCREPEAT,BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },
    
    { ACTION_SETTINGS_DEC,      BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_SETTINGS_DECREPEAT,BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_settingsgraphical */

static const struct button_mapping button_context_yesno[]  = {
    { ACTION_YESNO_ACCEPT,      BUTTON_PLAY,                    BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_settings_yesno */

/** Bookmark Screen **/
static const struct button_mapping button_context_bmark[]  = {
    { ACTION_BMS_DELETE,        BUTTON_NEXT,                    BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_LIST),
}; /* button_context_bmark */

static const struct button_mapping button_context_quickscreen[]  = {
    { ACTION_NONE,              BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_STD_CANCEL,        BUTTON_MENU,                    BUTTON_NONE },

    { ACTION_QS_TOP,            BUTTON_UP,                  BUTTON_NONE },
    { ACTION_QS_TOP,            BUTTON_UP|BUTTON_REPEAT,    BUTTON_NONE },

    { ACTION_QS_DOWN,           BUTTON_DOWN|BUTTON_REL,         BUTTON_NONE },
    { ACTION_QS_DOWN,           BUTTON_DOWN|BUTTON_REPEAT,      BUTTON_NONE },
    
    { ACTION_QS_LEFT,           BUTTON_LEFT|BUTTON_REL,         BUTTON_NONE },
    { ACTION_QS_LEFT,           BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },

    { ACTION_QS_RIGHT,          BUTTON_RIGHT|BUTTON_REL,        BUTTON_NONE },
    { ACTION_QS_RIGHT,          BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD),
}; /* button_context_quickscreen */

static const struct button_mapping button_context_pitchscreen[]  = {
    { ACTION_PS_RESET,          BUTTON_PLAY,                    BUTTON_NONE },
    { ACTION_PS_EXIT,           BUTTON_MENU,                    BUTTON_NONE },
    { ACTION_PS_EXIT,           BUTTON_PREV,                    BUTTON_NONE },
    { ACTION_PS_EXIT,           BUTTON_POWER,                   BUTTON_NONE },

    { ACTION_PS_INC_SMALL,      BUTTON_UP,                      BUTTON_NONE },
    { ACTION_PS_INC_BIG,        BUTTON_UP|BUTTON_REPEAT,        BUTTON_NONE },

    { ACTION_PS_DEC_SMALL,      BUTTON_DOWN,                    BUTTON_NONE },
    { ACTION_PS_DEC_BIG,        BUTTON_DOWN|BUTTON_REPEAT,      BUTTON_NONE },
    
    { ACTION_PS_NUDGE_LEFT,     BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_PS_NUDGE_LEFTOFF,  BUTTON_LEFT|BUTTON_REL,         BUTTON_NONE },
    
    { ACTION_PS_NUDGE_RIGHT,    BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_PS_NUDGE_RIGHTOFF, BUTTON_RIGHT|BUTTON_REL,        BUTTON_NONE },

    { ACTION_PS_SLOWER,         BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_PS_FASTER,         BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD),
}; /* button_context_pitchcreen */

static const struct button_mapping button_context_keyboard[]  = {
    { ACTION_KBD_SELECT,       BUTTON_PLAY|BUTTON_REL,          BUTTON_NONE },
    { ACTION_KBD_DONE,         BUTTON_PLAY|BUTTON_REPEAT,       BUTTON_NONE },
    { ACTION_KBD_ABORT,        BUTTON_POWER,                    BUTTON_NONE },

    { ACTION_KBD_LEFT,         BUTTON_LEFT,                     BUTTON_NONE },
    { ACTION_KBD_LEFT,         BUTTON_LEFT|BUTTON_REPEAT,       BUTTON_NONE },

    { ACTION_KBD_RIGHT,        BUTTON_RIGHT,                    BUTTON_NONE },
    { ACTION_KBD_RIGHT,        BUTTON_RIGHT|BUTTON_REPEAT,      BUTTON_NONE },

    { ACTION_KBD_UP,           BUTTON_UP,                       BUTTON_NONE },
    { ACTION_KBD_UP,           BUTTON_UP|BUTTON_REPEAT,         BUTTON_NONE },

    { ACTION_KBD_DOWN,         BUTTON_DOWN,                     BUTTON_NONE },
    { ACTION_KBD_DOWN,         BUTTON_DOWN|BUTTON_REPEAT,       BUTTON_NONE },

    { ACTION_KBD_CURSOR_LEFT,  BUTTON_PREV,                     BUTTON_NONE },
    { ACTION_KBD_CURSOR_LEFT,  BUTTON_PREV|BUTTON_REPEAT,       BUTTON_NONE },

    { ACTION_KBD_CURSOR_RIGHT, BUTTON_NEXT,                     BUTTON_NONE },
    { ACTION_KBD_CURSOR_RIGHT, BUTTON_NEXT|BUTTON_REPEAT,       BUTTON_NONE },

    { ACTION_KBD_BACKSPACE,    BUTTON_VOL_UP,                   BUTTON_NONE },
    { ACTION_KBD_BACKSPACE,    BUTTON_VOL_UP|BUTTON_REPEAT,     BUTTON_NONE },

    { ACTION_KBD_PAGE_FLIP,    BUTTON_VOL_DOWN,                 BUTTON_NONE },

    { ACTION_KBD_MORSE_INPUT,  BUTTON_MENU|BUTTON_REPEAT,       BUTTON_NONE },
    { ACTION_KBD_MORSE_SELECT, BUTTON_MENU|BUTTON_REL,          BUTTON_NONE },

    LAST_ITEM_IN_LIST
}; /* button_context_keyboard */

#ifdef USB_ENABLE_HID
static const struct button_mapping button_context_usb_hid[] = {
    { ACTION_USB_HID_INC,         BUTTON_UP,                     BUTTON_NONE },
    { ACTION_USB_HID_INC,         BUTTON_UP|BUTTON_REPEAT,       BUTTON_NONE },
    { ACTION_USB_HID_INC,         BUTTON_VOL_UP,                 BUTTON_NONE },
    { ACTION_USB_HID_INC,         BUTTON_VOL_UP|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_USB_HID_DEC,         BUTTON_DOWN,                   BUTTON_NONE },
    { ACTION_USB_HID_DEC,         BUTTON_DOWN|BUTTON_REPEAT,     BUTTON_NONE },
    { ACTION_USB_HID_DEC,         BUTTON_VOL_DOWN,               BUTTON_NONE },
    { ACTION_USB_HID_DEC,         BUTTON_VOL_DOWN|BUTTON_REPEAT, BUTTON_NONE },
    { ACTION_USB_HID_START,       BUTTON_PLAY|BUTTON_REL,        BUTTON_PLAY },
    { ACTION_USB_HID_START_LONG,  BUTTON_PLAY|BUTTON_REPEAT,     BUTTON_PLAY },
    { ACTION_USB_HID_QUIT,        BUTTON_PREV|BUTTON_REL,        BUTTON_PREV },
    { ACTION_USB_HID_QUIT_LONG,   BUTTON_PREV|BUTTON_REPEAT,     BUTTON_PREV },
    { ACTION_USB_HID_LEFT,        BUTTON_LEFT|BUTTON_REL,        BUTTON_LEFT },
    { ACTION_USB_HID_LEFT_LONG,   BUTTON_LEFT|BUTTON_REPEAT,     BUTTON_LEFT },
    { ACTION_USB_HID_RIGHT,       BUTTON_RIGHT|BUTTON_REL,       BUTTON_RIGHT },
    { ACTION_USB_HID_RIGHT_LONG,  BUTTON_RIGHT|BUTTON_REPEAT,    BUTTON_RIGHT },
    { ACTION_USB_HID_SELECT,      BUTTON_NEXT|BUTTON_REL,        BUTTON_NEXT },
    { ACTION_USB_HID_SELECT_LONG, BUTTON_NEXT|BUTTON_REPEAT,     BUTTON_NEXT },
    { ACTION_USB_HID_MENU,        BUTTON_MENU|BUTTON_REL,        BUTTON_MENU },
    { ACTION_USB_HID_MENU_LONG,   BUTTON_MENU|BUTTON_REPEAT,     BUTTON_MENU },
    { ACTION_USB_HID_MODE,        BUTTON_POWER|BUTTON_REL,       BUTTON_POWER },
    { ACTION_USB_HID_MODE_LONG,   BUTTON_POWER|BUTTON_REPEAT,    BUTTON_POWER },

    LAST_ITEM_IN_LIST
}; /* button_context_usb_hid */

static const struct button_mapping button_context_usb_hid_mouse[] = {
    { ACTION_USB_HID_MOUSE_UP,               BUTTON_UP,                                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_UP_REP,           BUTTON_UP|BUTTON_REPEAT,                  BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_DOWN,             BUTTON_DOWN,                              BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_DOWN_REP,         BUTTON_DOWN|BUTTON_REPEAT,                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LEFT,             BUTTON_LEFT,                              BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LEFT_REP,         BUTTON_LEFT|BUTTON_REPEAT,                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RIGHT,            BUTTON_RIGHT,                             BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RIGHT_REP,        BUTTON_RIGHT|BUTTON_REPEAT,               BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_UP,         BUTTON_PREV|BUTTON_UP,                  BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_UP_REP,     BUTTON_PREV|BUTTON_UP|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_DOWN,       BUTTON_PREV|BUTTON_DOWN,                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_DOWN_REP,   BUTTON_PREV|BUTTON_DOWN|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_LEFT,       BUTTON_PREV|BUTTON_LEFT,                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_LEFT_REP,   BUTTON_PREV|BUTTON_LEFT|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_RIGHT,      BUTTON_PREV|BUTTON_RIGHT,               BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_LDRAG_RIGHT_REP,  BUTTON_PREV|BUTTON_RIGHT|BUTTON_REPEAT, BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_UP,         BUTTON_NEXT|BUTTON_UP,                   BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_UP_REP,     BUTTON_NEXT|BUTTON_UP|BUTTON_REPEAT,     BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_DOWN,       BUTTON_NEXT|BUTTON_DOWN,                 BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_DOWN_REP,   BUTTON_NEXT|BUTTON_DOWN|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_LEFT,       BUTTON_NEXT|BUTTON_LEFT,                 BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_LEFT_REP,   BUTTON_NEXT|BUTTON_LEFT|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_RIGHT,      BUTTON_NEXT|BUTTON_RIGHT,                BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_RDRAG_RIGHT_REP,  BUTTON_NEXT|BUTTON_RIGHT|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_BUTTON_LEFT,      BUTTON_PREV,                            BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_BUTTON_LEFT_REL,  BUTTON_PREV|BUTTON_REL,                 BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_BUTTON_RIGHT,     BUTTON_NEXT,                            BUTTON_NONE },
    { ACTION_USB_HID_MOUSE_BUTTON_RIGHT_REL, BUTTON_NEXT|BUTTON_REL,                 BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_USB_HID)
}; /* button_context_usb_hid_mouse */
#endif

const struct button_mapping* get_context_mapping(int context)
{
    switch (context)
    {
        case CONTEXT_STD:
            return button_context_standard;

        case CONTEXT_WPS:
            return button_context_wps;

        case CONTEXT_LIST:
            return button_context_list;

        case CONTEXT_MAINMENU:
        case CONTEXT_TREE:
            if (global_settings.hold_lr_for_scroll_in_list)
                return button_context_listtree_scroll_without_combo;
            else
                return button_context_listtree_scroll_with_combo;

        case CONTEXT_CUSTOM|CONTEXT_TREE:
            return button_context_tree;

        case CONTEXT_SETTINGS_TIME:
        case CONTEXT_SETTINGS:
            return button_context_settings;

        case CONTEXT_CUSTOM|CONTEXT_SETTINGS:
        case CONTEXT_SETTINGS_COLOURCHOOSER:
        case CONTEXT_SETTINGS_EQ:
            return button_context_settings_right_is_inc;

        case CONTEXT_YESNOSCREEN:
            return button_context_yesno;

        case CONTEXT_BOOKMARKSCREEN:
            return button_context_bmark;

        case CONTEXT_QUICKSCREEN:
            return button_context_quickscreen;

        case CONTEXT_PITCHSCREEN:
            return button_context_pitchscreen;

        case CONTEXT_KEYBOARD:
            return button_context_keyboard;

#ifdef USB_ENABLE_HID
        case CONTEXT_USB_HID:
            return button_context_usb_hid;
        case CONTEXT_USB_HID_MOUSE:
            return button_context_usb_hid_mouse;
#endif
    }
    return button_context_standard;
}

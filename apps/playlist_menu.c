/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 Bj�rn Stenberg
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <string.h>

#include "menu.h"
#include "file.h"
#include "keyboard.h"
#include "playlist.h"
#include "tree.h"
#include "settings.h"
#include "playlist_viewer.h"

#include "lang.h"

#define DEFAULT_PLAYLIST_NAME "/dynamic.m3u"

static bool save_playlist(void)
{
    char filename[MAX_PATH+1];

    strncpy(filename, DEFAULT_PLAYLIST_NAME, sizeof(filename));

    if (!kbd_input(filename, sizeof(filename)))
    {
        playlist_save(NULL, filename);
        
        /* reload in case playlist was saved to cwd */
        reload_directory();
    }

    return false;
}

static bool recurse_directory(void)
{
    struct opt_items names[] = {
        { STR(LANG_OFF) },
        { STR(LANG_ON) },
        { STR(LANG_RESUME_SETTING_ASK)},
    };

    return set_option( str(LANG_RECURSE_DIRECTORY),
                       &global_settings.recursive_dir_insert, INT, names, 3,
                       NULL );
}

bool playlist_menu(void)
{
    int m;
    bool result;

    struct menu_items items[] = {
        { STR(LANG_CREATE_PLAYLIST),       create_playlist   },
        { STR(LANG_VIEW_DYNAMIC_PLAYLIST), playlist_viewer   },
        { STR(LANG_SAVE_DYNAMIC_PLAYLIST), save_playlist     },
        { STR(LANG_RECURSE_DIRECTORY),     recurse_directory },
    };
    
    m = menu_init( items, sizeof items / sizeof(struct menu_items), NULL );
    result = menu_run(m);
    menu_exit(m);
    return result;
}

/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_CLIP_H
# define _GIBBON_CLIP_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>

enum GibbonClipCode {
                        /* Numerical CLIP codes.  */
                        GIBBON_CLIP_CODE_UNHANDLED = 0,
                        GIBBON_CLIP_CODE_WELCOME = 1,
                        GIBBON_CLIP_CODE_OWN_INFO = 2,
                        GIBBON_CLIP_CODE_MOTD = 3,
                        GIBBON_CLIP_CODE_MOTD_END = 4,
                        GIBBON_CLIP_CODE_WHO_INFO = 5,
                        GIBBON_CLIP_CODE_WHO_INFO_END = 6,
                        GIBBON_CLIP_CODE_LOGIN = 7,
                        GIBBON_CLIP_CODE_LOGOUT = 8,
                        GIBBON_CLIP_CODE_MESSAGE = 9,
                        GIBBON_CLIP_CODE_MESSAGE_DELIVERED = 10,
                        GIBBON_CLIP_CODE_MESSAGE_SAVED = 11,
                        GIBBON_CLIP_CODE_SAYS = 12,
                        GIBBON_CLIP_CODE_SHOUTS = 13,
                        GIBBON_CLIP_CODE_WHISPERS = 14,
                        GIBBON_CLIP_CODE_KIBITZES = 15,
                        GIBBON_CLIP_CODE_YOU_SAY = 16,
                        GIBBON_CLIP_CODE_YOU_SHOUT = 17,
                        GIBBON_CLIP_CODE_YOU_WHISPER = 18,
                        GIBBON_CLIP_CODE_YOU_KIBITZ = 19,

                        /* Errors.  */
                        GIBBON_CLIP_CODE_ERROR = 100,

                        /* Game play.  */
                        GIBBON_CLIP_CODE_BOARD = 200,
                        GIBBON_CLIP_CODE_BAD_BOARD = 201,
                        GIBBON_CLIP_CODE_ROLLS = 202,
                        GIBBON_CLIP_CODE_MOVES = 203,

                        /* Between game action.  */
                        GIBBON_CLIP_CODE_YOURE_WATCHING = 302,

                        /* Various.  */
                        GIBBON_CLIP_CODE_START_MATCH = 400,
                        GIBBON_CLIP_CODE_WIN_MATCH = 401,
                        GIBBON_CLIP_CODE_RESUME_MATCH = 402
};

enum GibbonClipType {
                        GIBBON_CLIP_TYPE_END = 0,
                        GIBBON_CLIP_TYPE_UINT,
                        GIBBON_CLIP_TYPE_INT,
                        GIBBON_CLIP_TYPE_DOUBLE,
                        GIBBON_CLIP_TYPE_BOOLEAN,
                        GIBBON_CLIP_TYPE_STRING,
                        GIBBON_CLIP_TYPE_NAME,
                        GIBBON_CLIP_TYPE_TIMESTAMP
};

struct GibbonClipTokenSet {
        enum GibbonClipType type;
        union {
                guint64 i64;
                gchar *s;
                gdouble d;
        } v;
};

GSList *gibbon_clip_parse (const gchar *line);
void gibbon_clip_free_result (GSList *list);

gboolean gibbon_clip_get_uint64 (GSList **list, enum GibbonClipType type,
                                 guint64 *value);

#endif

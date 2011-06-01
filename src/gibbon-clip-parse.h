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

#ifndef _GIBBON_CLIP_PARSE_H
# define _GIBBON_CLIP_PARSE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>

enum GibbonClipCode {
                        GIBBON_CLIP_CODE_UNHANDLED = 0,
                        GIBBON_CLIP_CODE_WELCOME = 1,
                        GIBBON_CLIP_CODE_OWN_INFO = 2
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

#endif

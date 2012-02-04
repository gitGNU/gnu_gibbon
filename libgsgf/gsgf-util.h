/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBGSGF_UTIL_H
# define _LIBGSGF_UTIL_H

#include <glib.h>

G_BEGIN_DECLS

void gsgf_threads_init (void);
gchar *gsgf_util_read_simple_text (const gchar *raw, const gchar **end,
                                   gchar delim);
gchar *gsgf_util_read_text (const gchar *raw, const gchar **end, gchar delim);
gchar *gsgf_ascii_dtostring (gdouble d, gint width, gint precision,
                             gboolean zeropad, gboolean zerotrim);

G_END_DECLS

#endif

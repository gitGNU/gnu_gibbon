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

#ifndef _SVG_UTIL_H
#define _SVG_UTIL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <libxml/parser.h>
#include <svg-cairo.h>

struct svg_component {
        svg_cairo_t *scr;
        gdouble x, y, width, height;
};

G_BEGIN_DECLS

extern gboolean svg_util_get_dimensions (xmlNode* node, xmlDoc *doc,
                                         const gchar *filename,
                                         struct svg_component **svg,
                                         gboolean render);

void svg_util_free_component (struct svg_component *svg);
const gchar *svg_cairo_strerror (svg_cairo_status_t status);

extern gboolean svg_util_steal_text_params (struct svg_component *svg,
                                            const gchar *id,
                                            const gchar *new_text,
                                            gdouble scale,
                                            gdouble size,
                                            gdouble *saved_size);

G_END_DECLS

#endif

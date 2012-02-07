/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_COLOR_H
# define _LIBGSGF_COLOR_H

#include <glib.h>

#define GSGF_TYPE_COLOR \
        (gsgf_color_get_type ())
#define GSGF_COLOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_COLOR, \
                GSGFColor))
#define GSGF_COLOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_COLOR, GSGFColorClass))
#define GSGF_IS_COLOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_COLOR))
#define GSGF_IS_COLOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_COLOR))
#define GSGF_COLOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS (obj), \
                GSGF_TYPE_COLOR, GSGFColorClass))

/**
 * GSGFColor:
 *
 * One instance of a #GSGFColor.  All properties are private.
 **/
typedef struct _GSGFColor GSGFColor;
struct _GSGFColor
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        struct _GSGFColorPrivate *priv;
};

/**
 * GSGFColorClass:
 *
 * Representation of a color (black or white) in SGF.
 **/
typedef struct _GSGFColorClass GSGFColorClass;
struct _GSGFColorClass
{
        /* <private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_color_get_type (void) G_GNUC_CONST;

/**
 * GSGFColorEnum:
 * @GSGF_COLOR_WHITE: White.
 * @GSGF_COLOR_BLACK: Black.
 *
 * Constants for black and white.
 */
typedef enum {
        GSGF_COLOR_WHITE = 1,
        GSGF_COLOR_BLACK = 0
} GSGFColorEnum;

GSGFColor *gsgf_color_new (GSGFColorEnum color);

GSGFColorEnum gsgf_color_get_color (const GSGFColor *self);
GSGFCookedValue *gsgf_color_new_from_raw(const GSGFRaw* raw,
                                         const GSGFFlavor *flavor,
                                         const struct _GSGFProperty *property,
                                         GError **error);

#endif

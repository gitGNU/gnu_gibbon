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

#ifndef _LIBGSGF_RAW_H
# define _LIBGSGF_RAW_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_RAW             (gsgf_raw_get_type ())
#define GSGF_RAW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_RAW, GSGFRaw))
#define GSGF_RAW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_RAW, GSGFRawClass))
#define GSGF_IS_RAW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_RAW))
#define GSGF_IS_RAW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_RAW))
#define GSGF_RAW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_RAW, GSGFRawClass))

/**
 * GSGFRaw:
 *
 * One instance of a #GSGFRawClass.  All properties are private.
 **/
typedef struct _GSGFRaw        GSGFRaw;
struct _GSGFRaw
{
        GSGFValue parent_instance;

        /*< private >*/
        struct _GSGFRawPrivate *priv;
};

/**
 * GSGFRawClass:
 *
 * Class representing a raw of SGF.
 **/
typedef struct _GSGFRawClass   GSGFRawClass;
struct _GSGFRawClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_raw_get_type(void) G_GNUC_CONST;

GSGFRaw* gsgf_raw_new(const gchar *value);
gsize gsgf_raw_get_number_of_values(const GSGFRaw *self);
gchar *gsgf_raw_get_value(const GSGFRaw *self, gsize i);
void gsgf_raw_add_value(GSGFRaw *self, const gchar *value);

G_END_DECLS

#endif

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

#ifndef _LIBGSGF_VALUE_H
# define _LIBGSGF_VALUE_H

#include <glib.h>

#define GSGF_TYPE_VALUE \
        (gsgf_value_get_type ())
#define GSGF_VALUE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_VALUE, \
                GSGFValue))
#define GSGF_VALUE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_VALUE, GSGFValueClass))
#define GSGF_IS_VALUE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_VALUE))
#define GSGF_IS_VALUE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_VALUE))
#define GSGF_VALUE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_VALUE, GSGFValueClass))

/**
 * GSGFValue:
 *
 * Encapsulation for any value, raw or cooked.  All properties are private.
 **/
typedef struct _GSGFValue GSGFValue;

struct _GSGFValue
{
        GObject parent_instance;
};


/**
 * GSGFValueClass:
 *
 * Class definition for any SGF value.
 */
typedef struct _GSGFValueClass GSGFValueClass;
struct _GSGFValueClass
{
        /* <private >*/
        GObjectClass parent_class;

        gboolean (*write_stream) (const GSGFValue *self,
                                  GOutputStream *out, gsize *bytes_written,
                                  GCancellable *cancellable, GError **error);
};

GType gsgf_value_get_type (void) G_GNUC_CONST;

gboolean gsgf_value_write_stream (const GSGFValue *self,
                                  GOutputStream *out, gsize *bytes_written,
                                  GCancellable *cancellable, GError **error);

#endif

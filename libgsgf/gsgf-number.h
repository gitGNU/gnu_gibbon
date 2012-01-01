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

#ifndef _LIBGSGF_NUMBER_H
# define _LIBGSGF_NUMBER_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_NUMBER             (gsgf_number_get_type ())
#define GSGF_NUMBER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_NUMBER, GSGFNumber))
#define GSGF_NUMBER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_NUMBER, GSGFNumberClass))
#define GSGF_IS_NUMBER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_NUMBER))
#define GSGF_IS_NUMBER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_NUMBER))
#define GSGF_NUMBER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_NUMBER, GSGFNumberClass))

/**
 * GSGFNumber:
 *
 * One instance of a #GSGFNumberClass.  All properties are private.
 **/
typedef struct _GSGFNumber        GSGFNumber;
struct _GSGFNumber
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        struct _GSGFNumberPrivate *priv;
};

/**
 * GSGFNumberClass:
 *
 * Class representing an SGF number.
 **/
typedef struct _GSGFNumberClass   GSGFNumberClass;

struct _GSGFNumberClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_number_get_type(void) G_GNUC_CONST;

struct _GSGFProperty;

GSGFNumber *gsgf_number_new(gint64 value);
GSGFCookedValue *gsgf_number_new_from_raw(const GSGFRaw* raw,
                                          const GSGFFlavor *flavor,
                                          const struct _GSGFProperty *property,
                                          GError **error);
void gsgf_number_set_value(GSGFNumber *self, gint64 value);
gint64 gsgf_number_get_value(const GSGFNumber *self);

G_END_DECLS

#endif

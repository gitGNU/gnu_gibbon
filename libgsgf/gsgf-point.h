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

#ifndef _LIBGSGF_POINT_H
# define _LIBGSGF_POINT_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_POINT             (gsgf_point_get_type ())
#define GSGF_POINT(obj)             \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_POINT, GSGFPoint))
#define GSGF_POINT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_POINT, GSGFPointClass))
#define GSGF_IS_POINT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_POINT))
#define GSGF_IS_POINT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_POINT))
#define GSGF_POINT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_POINT, GSGFPointClass))

/**
 * GSGFPoint:
 *
 * One instance of a #GSGFPointClass.  All properties are private.
 **/
typedef struct _GSGFPoint        GSGFPoint;
struct _GSGFPoint
{
        /*< private >*/
        GSGFCookedValue parent_instance;
};

/**
 * GSGFPointClass:
 *
 * Class representing a point of SGF.
 **/
typedef struct _GSGFPointClass   GSGFPointClass;
struct _GSGFPointClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;

        gint (*get_normalized_value) (const struct _GSGFPoint *self);
};

GType gsgf_point_get_type(void) G_GNUC_CONST;

gint gsgf_point_get_normalized_value(const GSGFPoint *self);

G_END_DECLS

#endif

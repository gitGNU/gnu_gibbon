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

#ifndef _LIBGSGF_POINT_BACKGAMMON_H
# define _LIBGSGF_POINT_BACKGAMMON_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-point.h>

G_BEGIN_DECLS

#define GSGF_TYPE_POINT_BACKGAMMON  (gsgf_point_backgammon_get_type ())
#define GSGF_POINT_BACKGAMMON(obj)             \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_POINT_BACKGAMMON, \
			GSGFPointBackgammon))
#define GSGF_POINT_BACKGAMMON_CLASS(klass)     \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_POINT_BACKGAMMON, \
			GSGFPointBackgammonClass))
#define GSGF_IS_POINT_BACKGAMMON(obj)          \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_POINT_BACKGAMMON))
#define GSGF_IS_POINT_BACKGAMMON_CLASS(klass)  \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_POINT_BACKGAMMON))
#define GSGF_POINT_BACKGAMMON_GET_CLASS(obj)   \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_POINT_BACKGAMMON, \
			GSGFPointBackgammonClass))

/**
 * GSGFPointBackgammon:
 *
 * One instance of a #GSGFPointBackgammonClass.  All properties are private.
 **/
typedef struct _GSGFPointBackgammon GSGFPointBackgammon;
struct _GSGFPointBackgammon
{
        GSGFPoint parent_instance;

        /*< private >*/
        struct _GSGFPointBackgammonPrivate *priv;
};

/**
 * GSGFPointBackgammonClass:
 *
 * Class implementing an SGF backgammon point.
 **/
typedef struct _GSGFPointBackgammonClass GSGFPointBackgammonClass;
struct _GSGFPointBackgammonClass
{
        /*< private >*/
        GSGFPointClass parent_class;
};

GType gsgf_point_backgammon_get_type(void) G_GNUC_CONST;

struct _GSGFListOf;

GSGFPointBackgammon *gsgf_point_backgammon_new(gint point);
GSGFPointBackgammon *gsgf_point_backgammon_new_from_raw(const GSGFRaw *raw,
                                                        gsize i,
                                                        GError **error);
gboolean gsgf_point_backgammon_append_to_list_of(struct _GSGFListOf *list_of,
                                                 const GSGFRaw *raw,
                                                 gsize i, GError **error);

gint gsgf_point_backgammon_get_point(const GSGFPointBackgammon *self);

G_END_DECLS

#endif

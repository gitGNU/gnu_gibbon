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

#ifndef _LIBGSGF_LIST_OF_H
# define _LIBGSGF_LIST_OF_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_LIST_OF             (gsgf_list_of_get_type ())
#define GSGF_LIST_OF(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_LIST_OF, GSGFListOf))
#define GSGF_LIST_OF_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_LIST_OF, GSGFListOfClass))
#define GSGF_IS_LIST_OF(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_LIST_OF))
#define GSGF_IS_LIST_OF_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_LIST_OF))
#define GSGF_LIST_OF_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_LIST_OF, GSGFListOfClass))

/**
 * GSGFListOf:
 *
 * One instance of a #GSGFListOfClass.
 **/
typedef struct _GSGFListOf        GSGFListOf;
struct _GSGFListOf
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        struct _GSGFListOfPrivate *priv;
};

/**
 * GSGFListOfClass:
 *
 * Class representing a list_of of SGF.
 **/
typedef struct _GSGFListOfClass   GSGFListOfClass;
struct _GSGFListOfClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_list_of_get_type (void) G_GNUC_CONST;

GSGFListOf *gsgf_list_of_new(GType type, const GSGFFlavor *flavor);

GType gsgf_list_of_get_item_type(const GSGFListOf *self);
gsize gsgf_list_of_get_number_of_items(const GSGFListOf *self);
gboolean gsgf_list_of_append(GSGFListOf *self, GSGFCookedValue *item,
                             GError **error);
GSGFCookedValue *gsgf_list_of_get_nth_item(const GSGFListOf *self, gsize i);

G_END_DECLS

#endif

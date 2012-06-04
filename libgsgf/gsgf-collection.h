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

#ifndef _LIBGSGF_COLLECTION_H
# define _LIBGSGF_COLLECTION_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_COLLECTION             \
	(gsgf_collection_get_type ())
#define GSGF_COLLECTION(obj)             \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_COLLECTION, \
	                GSGFCollection))
#define GSGF_COLLECTION_CLASS(klass)     \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_COLLECTION, \
			GSGFCollectionClass))
#define GSGF_IS_COLLECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_COLLECTION))
#define GSGF_IS_COLLECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_COLLECTION))
#define GSGF_COLLECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_COLLECTION, GSGFCollectionClass))

/**
 * GSGFCollection:
 *
 * A collection of SGF games.
 **/

typedef struct _GSGFCollection        GSGFCollection;

struct _GSGFCollection
{
        GObject parent_instance;

        /*< private >*/
        struct _GSGFCollectionPrivate *priv;
};

/**
 * GSGFCollectionClass:
 *
 * Class definition for a collection of SGF games.
 **/
typedef struct _GSGFCollectionClass   GSGFCollectionClass;
struct _GSGFCollectionClass
{
        /*< private >*/
        GObjectClass parent_class;
};

struct _GSGFGameTree;

GType gsgf_collection_get_type(void) G_GNUC_CONST;

GSGFCollection *gsgf_collection_new();
GSGFCollection *gsgf_collection_parse_stream(GInputStream *stream,
                                             GCancellable *cancellable,
                                             GError **error);
GSGFCollection *gsgf_collection_parse_file(GFile *file,
                                           GCancellable *cancellable,
                                           GError **error);

struct _GSGFGameTree *gsgf_collection_add_game_tree (GSGFCollection *self,
                                                     const GSGFFlavor *flavor);

GList *gsgf_collection_get_game_trees(const GSGFCollection *self);

G_END_DECLS

#endif

/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#include <gtk/gtk.h>

#include <libgsgf/gsgf-collection.h> 

struct _GSGFCollectionPrivate {
        guint dummy;
};

#define GSGF_COLLECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COLLECTION,           \
                                      GSGFCollectionPrivate))
G_DEFINE_TYPE (GSGFCollection, gsgf_collection, G_TYPE_OBJECT);

static void
gsgf_collection_init (GSGFCollection *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GSGF_TYPE_COLLECTION,
                                                  GSGFCollectionPrivate);
        
        /* self->priv->... = NULL */
}

static void
gsgf_collection_finalize (GObject *object)
{
        GSGFCollection *collection = GSGF_COLLECTION (object);

        /* g_free (...) */

        G_OBJECT_CLASS (gsgf_collection_parent_class)->finalize (object);
}

static void
gsgf_collection_class_init (GSGFCollectionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GSGFCollectionPrivate));

        object_class->finalize = gsgf_collection_finalize;
}

GSGFCollection *
gsgf_collection_new ()
{
         GSGFCollection *self = g_object_new (GSGF_TYPE_COLLECTION, NULL);

         /* self->priv->move_number = 0; */
         
         return self;
}

GSGFCollection *
gsgf_collection_parse_file (const GFile *file, GCancellable *cancellable, GError **error)
{
         GSGFCollection *self = g_object_new (GSGF_TYPE_COLLECTION, NULL);

         /* self->priv->move_number = 0; */

         return self;
}

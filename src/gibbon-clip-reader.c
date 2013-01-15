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

/**
 * SECTION:gibbon-clip-reader
 * @short_description: Parse FIBS server output.
 *
 * Since: 0.2.0
 *
 * This class pre-processes the output from FIBS and calls the registered
 * handlers for it.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip-reader.h"

typedef struct _GibbonClipReaderPrivate GibbonClipReaderPrivate;
struct _GibbonClipReaderPrivate {
        int (*handler) (gpointer, GSList *, const gchar *);
        gpointer data;
};

#define GIBBON_CLIP_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CLIP_READER, GibbonClipReaderPrivate))

G_DEFINE_TYPE (GibbonClipReader, gibbon_clip_reader, G_TYPE_OBJECT)

static void 
gibbon_clip_reader_init (GibbonClipReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CLIP_READER, GibbonClipReaderPrivate);

        self->priv->handler = NULL;
        self->priv->data = NULL;
}

static void
gibbon_clip_reader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_clip_reader_parent_class)->finalize(object);
}

static void
gibbon_clip_reader_class_init (GibbonClipReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonClipReaderPrivate));

        object_class->finalize = gibbon_clip_reader_finalize;
}

/**
 * gibbon_clip_reader_new:
 * @handler: Handler to call on arrival of new data.
 * @data: Argument for @handler or %NULL.
 *
 * Creates a new #GibbonClipReader.
 *
 * Returns: The newly created #GibbonClipReader or %NULL in case of failure.
 */
GibbonClipReader *
gibbon_clip_reader_new (int (*handler) (gpointer, GSList *, const gchar *),
                        gpointer data)
{
        GibbonClipReader *self = g_object_new (GIBBON_TYPE_CLIP_READER, NULL);

        self->priv->handler = handler;
        self->priv->data = data;

        return self;
}

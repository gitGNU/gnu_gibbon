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
 * SECTION:gibbon-java-fibs-reader
 * @short_description: Read JavaFIBS internal format.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchReader for reading match files in the JavaFIBS internal
 * format.!
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-reader.h"

typedef struct _GibbonJavaFIBSReaderPrivate GibbonJavaFIBSReaderPrivate;
struct _GibbonJavaFIBSReaderPrivate {
        /* FIXME! Replace with the real structure of the private data! */
        gchar *dummy;
};

#define GIBBON_JAVA_FIBS_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSReader, gibbon_java_fibs_reader, GIBBON_TYPE_MATCH_READER)

static void 
gibbon_java_fibs_reader_init (GibbonJavaFIBSReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate);

        /* FIXME! Initialize private data! */
        self->priv->dummy = NULL;
}

static void
gibbon_java_fibs_reader_finalize (GObject *object)
{
        GibbonJavaFIBSReader *self = GIBBON_JAVA_FIBS_READER (object);

        /* FIXME! Free private data! */
        if (self->priv->dummy)
                g_free (self->priv->dummy);
        self->priv->dummy = NULL;

        G_OBJECT_CLASS (gibbon_java_fibs_reader_parent_class)->finalize(object);
}

static void
gibbon_java_fibs_reader_class_init (GibbonJavaFIBSReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class = GIBBON_MATCH_READER_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gibbon_match_reader_class->do_this = gibbon_java_fibs_reader_do_this; */
        
        g_type_class_add_private (klass, sizeof (GibbonJavaFIBSReaderPrivate));

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GibbonJavaFIBSReader_do_that; */

        object_class->finalize = gibbon_java_fibs_reader_finalize;
}

/**
 * gibbon_java_fibs_reader_new:
 * @dummy: The argument.
 *
 * Creates a new #GibbonJavaFIBSReader.
 *
 * Returns: The newly created #GibbonJavaFIBSReader or %NULL in case of failure.
 */
GibbonJavaFIBSReader *
gibbon_java_fibs_reader_new (/* FIXME! Argument list! */ const gchar *dummy)
{
        GibbonJavaFIBSReader *self = g_object_new (GIBBON_TYPE_JAVA_FIBS_READER, NULL);

        /* FIXME! Initialize private data! */
        /* self->priv->dummy = g_strdup (dummy); */

        return self;
}

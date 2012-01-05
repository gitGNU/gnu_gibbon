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

#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-reader.h"

typedef struct _GibbonJavaFIBSReaderPrivate GibbonJavaFIBSReaderPrivate;
struct _GibbonJavaFIBSReaderPrivate {
        gchar *filename;
        gchar *you;
};

#define GIBBON_JAVA_FIBS_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSReader, gibbon_java_fibs_reader, GIBBON_TYPE_MATCH_READER)

static void 
gibbon_java_fibs_reader_init (GibbonJavaFIBSReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate);

        self->priv->filename = NULL;
        self->priv->you = NULL;
}

static void
gibbon_java_fibs_reader_finalize (GObject *object)
{
        GibbonJavaFIBSReader *self = GIBBON_JAVA_FIBS_READER (object);

        g_free (self->priv->filename);
        g_free (self->priv->you);

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
 * @path: Filename to read
 * @you: Player name to replace for JavaFIBS' "You"
 *
 * Creates a new #GibbonJavaFIBSReader.
 *
 * Returns: The newly created #GibbonJavaFIBSReader or %NULL in case of failure.
 */
GibbonJavaFIBSReader *
gibbon_java_fibs_reader_new (const gchar *filename, const gchar *you)
{
        GibbonJavaFIBSReader *self = g_object_new (GIBBON_TYPE_JAVA_FIBS_READER, NULL);
        FILE *in;
        extern FILE *gibbon_java_fibs_lexer_in;
        extern gibbon_java_fibs_parser_parse ();

        g_return_val_if_fail (filename != NULL, NULL);

        self->priv->filename = g_strdup (filename);
        if (you)
                self->priv->you = g_strdup (you);

        in = fopen (filename, "rb");
        if (!in)
                return self;

        gibbon_java_fibs_lexer_in = in;

        do {
                gibbon_java_fibs_parser_parse ();
        } while (!feof (in));

        fclose (in);

        return self;
}

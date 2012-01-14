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
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "gibbon-java-fibs-reader.h"

typedef struct _GibbonJavaFIBSReaderPrivate GibbonJavaFIBSReaderPrivate;
struct _GibbonJavaFIBSReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        const gchar *filename;
        GibbonMatch *match;
};

static GibbonJavaFIBSReader *instance = NULL;

#define GIBBON_JAVA_FIBS_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSReader, gibbon_java_fibs_reader, GIBBON_TYPE_MATCH_READER)

static GibbonMatch *gibbon_java_fibs_reader_parse (GibbonMatchReader *match_reader,
                                                   const gchar *filename);

static void 
gibbon_java_fibs_reader_init (GibbonJavaFIBSReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate);

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;

        /* Per parser-instance data.  */
        self->priv->filename = NULL;
        self->priv->match = NULL;
}

static void
gibbon_java_fibs_reader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_java_fibs_reader_parent_class)->finalize(object);
}

static void
gibbon_java_fibs_reader_class_init (GibbonJavaFIBSReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_java_fibs_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonJavaFIBSReaderPrivate));

        object_class->finalize = gibbon_java_fibs_reader_finalize;
}

/**
 * gibbon_java_fibs_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonJavaFIBSReader.
 *
 * Returns: The newly created #GibbonJavaFIBSReader.
 */
GibbonJavaFIBSReader *
gibbon_java_fibs_reader_new (GibbonMatchReaderErrorFunc yyerror,
                             gpointer user_data)
{
        GibbonJavaFIBSReader *self = g_object_new (GIBBON_TYPE_JAVA_FIBS_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;

        return self;
}

static GibbonMatch *
gibbon_java_fibs_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonJavaFIBSReader *self;
        FILE *in;
        extern FILE *gibbon_java_fibs_lexer_in;
        extern int gibbon_java_fibs_parser_parse ();

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (_self), NULL);
        self = GIBBON_JAVA_FIBS_READER (_self);

        gdk_threads_enter ();
        if (instance) {
                g_critical ("Another instance of GibbonJavaFIBSReader is"
                            " currently active!");
                gdk_threads_leave ();
                return NULL;
        }
        instance = self;
        gdk_threads_leave ();

        self->priv->filename = filename;
        self->priv->match = NULL;

        if (filename)
                in = fopen (filename, "rb");
        else
                in = stdin;
        if (in) {
                gibbon_java_fibs_lexer_in = in;

                gibbon_java_fibs_parser_parse ();
                if (filename)
                        fclose (in);
        } else {
                gibbon_java_fibs_reader_yyerror (strerror (errno));
        }

        self->priv->filename = NULL;

        gdk_threads_enter ();
        if (!instance || instance != self) {
                if (self->priv->match)
                        g_object_unref (self->priv->match);
                self->priv->match = NULL;
                g_critical ("Another instance of GibbonJavaFIBSReader has"
                            " reset this one!");
                gdk_threads_leave ();
                return NULL;
        }
        instance = NULL;
        gdk_threads_leave ();

        return self->priv->match;
}

void
gibbon_java_fibs_reader_yyerror (const gchar *msg)
{
        gchar *full_msg;
        const gchar *filename;
        extern int gibbon_java_fibs_lexer_get_lineno ();
        int lineno;

        if (!instance || !GIBBON_IS_JAVA_FIBS_READER (instance)) {
                g_critical ("gibbon_java_fibs_reader_yyerror() called without"
                            " an instance");
                return;
        }

        /* FIXME! Get yylineno! */
        if (instance->priv->filename)
                filename = instance->priv->filename;
        else
                filename = _("[standard input]");

        lineno = gibbon_java_fibs_lexer_get_lineno ();

        if (lineno)
                full_msg = g_strdup_printf ("%s:%d: %s", filename, lineno, msg);
        else
                full_msg = g_strdup_printf ("%s: %s", filename, msg);

        if (instance->priv->yyerror)
                instance->priv->yyerror (instance->priv->user_data, full_msg);
        else
                g_printerr ("%s\n", full_msg);

        g_free (full_msg);
}

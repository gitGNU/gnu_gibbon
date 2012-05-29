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
 * SECTION:gibbon-gmd-reader
 * @short_description: Read GMD (Gibbon Match Dump) format.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchReader for reading match files in the GMD format.!
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "gibbon-gmd-reader-priv.h"

#include "gibbon-game.h"
#include "gibbon-game-action.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"
#include "gibbon-resign.h"
#include "gibbon-reject.h"
#include "gibbon-accept.h"

typedef struct _GibbonGMDReaderPrivate GibbonGMDReaderPrivate;
struct _GibbonGMDReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        const gchar *filename;
        GibbonMatch *match;

        GSList *names;

        gchar *white;
};

GibbonGMDReader *_gibbon_gmd_reader_instance = NULL;

#define GIBBON_GMD_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GMD_READER, GibbonGMDReaderPrivate))

G_DEFINE_TYPE (GibbonGMDReader, gibbon_gmd_reader, GIBBON_TYPE_MATCH_READER)

static GibbonMatch *gibbon_gmd_reader_parse (GibbonMatchReader *match_reader,
                                                   const gchar *filename);
static gboolean gibbon_gmd_reader_add_action (GibbonGMDReader *self,
                                                    const gchar *name,
                                                    GibbonGameAction *action);

static void 
gibbon_gmd_reader_init (GibbonGMDReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GMD_READER, GibbonGMDReaderPrivate);

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;

        /* Per parser-instance data.  */
        self->priv->filename = NULL;
        self->priv->match = NULL;
        self->priv->names = NULL;
}

static void
gibbon_gmd_reader_finalize (GObject *object)
{
        GibbonGMDReader *self = GIBBON_GMD_READER (object);

        _gibbon_gmd_reader_free_names (self);

        G_OBJECT_CLASS (gibbon_gmd_reader_parent_class)->finalize(object);
}

static void
gibbon_gmd_reader_class_init (GibbonGMDReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_gmd_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonGMDReaderPrivate));

        object_class->finalize = gibbon_gmd_reader_finalize;
}

/**
 * gibbon_gmd_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonGMDReader.
 *
 * Returns: The newly created #GibbonGMDReader.
 */
GibbonGMDReader *
gibbon_gmd_reader_new (GibbonMatchReaderErrorFunc yyerror,
                       gpointer user_data)
{
        GibbonGMDReader *self = g_object_new (GIBBON_TYPE_GMD_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;

        return self;
}

static GibbonMatch *
gibbon_gmd_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonGMDReader *self;
        FILE *in;
        extern FILE *gibbon_gmd_lexer_in;
        extern int gibbon_gmd_parser_parse ();
        int parse_status;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (_self), NULL);
        self = GIBBON_GMD_READER (_self);

        gdk_threads_enter ();
        if (_gibbon_gmd_reader_instance) {
                g_critical ("Another instance of GibbonGMDReader is"
                            " currently active!");
                gdk_threads_leave ();
                return NULL;
        }
        _gibbon_gmd_reader_instance = self;
        gdk_threads_leave ();

        self->priv->filename = filename;
        if (self->priv->match)
                g_object_unref (self->priv->match);
        self->priv->match = gibbon_match_new (NULL, NULL, 0, FALSE);
        _gibbon_gmd_reader_free_names (self);
        g_free (self->priv->white);
        self->priv->white = NULL;

        if (filename)
                in = fopen (filename, "rb");
        else
                in = stdin;
        if (in) {
                gibbon_gmd_lexer_in = in;

                parse_status = gibbon_gmd_parser_parse ();
                if (filename)
                        fclose (in);
                if (parse_status) {
                        if (self->priv->match)
                                g_object_unref (self->priv->match);
                        self->priv->match = NULL;
                        g_free (self->priv->white);
                        self->priv->white = NULL;
                }
        } else {
                _gibbon_gmd_reader_yyerror (strerror (errno));
        }

        self->priv->filename = NULL;

        gdk_threads_enter ();
        if (!_gibbon_gmd_reader_instance
             || _gibbon_gmd_reader_instance != self) {
                if (self->priv->match)
                        g_object_unref (self->priv->match);
                self->priv->match = NULL;
                _gibbon_gmd_reader_free_names (self);
                g_free (self->priv->white);
                self->priv->white = NULL;
                g_critical ("Another instance of GibbonGMDReader has"
                            " reset this one!");
                gdk_threads_leave ();
                return NULL;
        }
        _gibbon_gmd_reader_instance = NULL;
        gdk_threads_leave ();

        if (self->priv->white)
                gibbon_match_set_white (self->priv->match, self->priv->white);
        g_free (self->priv->white);
        self->priv->white = NULL;

        return self->priv->match;
}

void
_gibbon_gmd_reader_yyerror (const gchar *msg)
{
        gchar *full_msg;
        const gchar *filename;
        extern int gibbon_gmd_lexer_get_lineno ();
        int lineno;
        GibbonGMDReader *instance = _gibbon_gmd_reader_instance;

        if (!instance || !GIBBON_IS_GMD_READER (instance)) {
                g_critical ("gibbon_gmd_reader_yyerror() called without"
                            " an instance");
                return;
        }

        if (instance->priv->filename)
                filename = instance->priv->filename;
        else
                filename = _("[standard input]");

        lineno = gibbon_gmd_lexer_get_lineno ();

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


gchar *
_gibbon_gmd_reader_alloc_name (GibbonGMDReader *self,
                                     const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), NULL);

        self->priv->names = g_slist_prepend (self->priv->names,
                                             g_strdup (name));

        return self->priv->names->data;
}

void
_gibbon_gmd_reader_free_names (GibbonGMDReader *self)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));

        g_slist_foreach (self->priv->names, (GFunc) g_free, NULL);
        g_slist_free (self->priv->names);
        self->priv->names = NULL;
}

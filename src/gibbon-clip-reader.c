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
 * This class pre-processes the output from FIBS and translated it into
 * simple syntax trees.
 */

#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip-reader.h"
#include "gibbon-clip-reader-priv.h"
#include "gibbon-util.h"

typedef struct _GibbonCLIPReaderPrivate GibbonCLIPReaderPrivate;
struct _GibbonCLIPReaderPrivate {
        void *yyscanner;
};

#define GIBBON_CLIP_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderPrivate))

G_DEFINE_TYPE (GibbonCLIPReader, gibbon_clip_reader, G_TYPE_OBJECT)

static void 
gibbon_clip_reader_init (GibbonCLIPReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderPrivate);

        self->priv->yyscanner = NULL;
}

static void
gibbon_clip_reader_finalize (GObject *object)
{
        GibbonCLIPReader *self = GIBBON_CLIP_READER (object);

        if (self->priv->yyscanner)
                gibbon_clip_lexer_lex_destroy (self->priv->yyscanner);

        G_OBJECT_CLASS (gibbon_clip_reader_parent_class)->finalize(object);
}

static void
gibbon_clip_reader_class_init (GibbonCLIPReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonCLIPReaderPrivate));

        object_class->finalize = gibbon_clip_reader_finalize;
}

/**
 * gibbon_clip_reader_new:
 *
 * Creates a new #GibbonCLIPReader.
 *
 * Returns: The newly created #GibbonCLIPReader or %NULL in case of failure.
 */
GibbonCLIPReader *
gibbon_clip_reader_new ()
{
        GibbonCLIPReader *self = g_object_new (GIBBON_TYPE_CLIP_READER, NULL);

        if (gibbon_clip_lexer_lex_init_extra (self, &self->priv->yyscanner)) {
                g_error (_("Error creating tokenizer: %s!"),
                         strerror (errno));
                /* NOTREACHED */
                return NULL;
        }

        return self;
}

GSList *
gibbon_clip_reader_parse (GibbonCLIPReader *self, const gchar *line)
{
        GSList *result = NULL;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), NULL);
        g_return_val_if_fail (line != NULL, NULL);

        gibbon_clip_lexer_current_buffer (self->priv->yyscanner, line);

        return result;
}

void
gibbon_clip_reader_yyerror (void *scanner, const gchar *msg)
{
        if (gibbon_debug ("clip-parser"))
                g_printerr ("%s\n", msg);
}

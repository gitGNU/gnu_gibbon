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

/**
 * SECTION:gsgf-stone-backgammon
 * @short_description: Definitions for a stone in Backgammon
 *
 * Representation of one single stone in Backgammon.  The class is mostly
 * internal.  You will not have to use it yourself.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFStoneBackgammonPrivate GSGFStoneBackgammonPrivate;
struct _GSGFStoneBackgammonPrivate {
        gint stone;
};

#define GSGF_STONE_BACKGAMMON_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                         GSGF_TYPE_STONE_BACKGAMMON,        \
                                         GSGFStoneBackgammonPrivate))

G_DEFINE_TYPE(GSGFStoneBackgammon, gsgf_stone_backgammon, GSGF_TYPE_STONE)

static gboolean gsgf_stone_backgammon_write_stream (const GSGFValue *self,
                                                    GOutputStream *out,
                                                    gsize *bytes_written,
                                                    GCancellable *cancellable,
                                                    GError **error);

static void
gsgf_stone_backgammon_init(GSGFStoneBackgammon *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GSGF_TYPE_STONE_BACKGAMMON,
                                                  GSGFStoneBackgammonPrivate);

        self->priv->stone = 0;
}

static void
gsgf_stone_backgammon_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_stone_backgammon_parent_class)->finalize(object);
}

static void
gsgf_stone_backgammon_class_init(GSGFStoneBackgammonClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *value_class = GSGF_VALUE_CLASS (klass);

        value_class->write_stream = gsgf_stone_backgammon_write_stream;

        g_type_class_add_private(klass, sizeof(GSGFStoneBackgammonPrivate));

        object_class->finalize = gsgf_stone_backgammon_finalize;
}

/**
 * gsgf_stone_backgammon_new:
 * @stone: The point as a number between 0 and 25.
 *
 * Creates a new #GSGFStoneBackgammon.
 *
 * Returns: The new #GSGFStoneBackgammon.
 */
GSGFStoneBackgammon *
gsgf_stone_backgammon_new (gint stone)
{
        GSGFStoneBackgammon *self;

        g_return_val_if_fail(stone >= 0 && stone <= 25, NULL);

        self = g_object_new(GSGF_TYPE_STONE_BACKGAMMON, NULL);
        self->priv->stone = stone;

        return self;
}

/**
 * gsgf_stone_backgammon_new_from_raw:
 * @raw: The #GSGFRaw to parse.
 * @i: The index into @raw.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFStoneBackgammon from a #GSGFRaw.
 *
 * Returns: The new #GSGFStoneBackgammon.
 */
GSGFStoneBackgammon *
gsgf_stone_backgammon_new_from_raw (const GSGFRaw *raw, gsize i, GError **error)
{
        const gchar* string;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        string = gsgf_raw_get_value(raw, i);
        if (!string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_STONE,
                            _("Empty stone"));
                return NULL;
        }

        if (string[0] < 'a' || string[0] > 'z' || string[1]) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_STONE,
                                _("Invalid stone syntax"));
                return NULL;
        }

        return gsgf_stone_backgammon_new((gint) string[0] - 'a');
}

/**
 * gsgf_stone_backgammon_get_stone:
 * @self: The #GSGFStoneBackgammon to query.
 *
 * Get the number (0-25) associated with a #GSGFStoneBackgammon.
 *
 * Returns: The number (0-25) associated with @self.
 */
gint
gsgf_stone_backgammon_get_stone(const GSGFStoneBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_STONE_BACKGAMMON(self), 0);

        return (guint) self->priv->stone;
}

static gboolean
gsgf_stone_backgammon_write_stream (const GSGFValue *_self,
                                    GOutputStream *out, gsize *bytes_written,
                                    GCancellable *cancellable, GError **error)
{
        GSGFStoneBackgammon *self = GSGF_STONE_BACKGAMMON (_self);
        gchar buf;

        *bytes_written = 0;

        buf = self->priv->stone + 'a';
        if (!g_output_stream_write_all(out, &buf, 1,
                                       bytes_written,
                                       cancellable, error))
                return FALSE;

        return TRUE;
}

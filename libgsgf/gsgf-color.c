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
 * SECTION:gsgf-color
 * @short_description: Primitive type for an SGF color.
 * Since: 0.1.0
 *
 * A #GSGFColor encapsulates an SGF color, that is either black or white.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFColorPrivate GSGFColorPrivate;
struct _GSGFColorPrivate {
        GSGFColorEnum color;
};

#define GSGF_COLOR_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_COLOR, GSGFColorPrivate))

G_DEFINE_TYPE (GSGFColor, gsgf_color, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_color_write_stream (const GSGFValue *self,
                                         GOutputStream *out,
                                         gsize *bytes_written,
                                         GCancellable *cancellable,
                                         GError **error);

static void 
gsgf_color_init (GSGFColor *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_COLOR, GSGFColorPrivate);

        self->priv->color = GSGF_COLOR_BLACK;
}

static void
gsgf_color_finalize (GObject *object)
{
        G_OBJECT_CLASS (gsgf_color_parent_class)->finalize(object);
}

static void
gsgf_color_class_init (GSGFColorClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *gsgf_value_class = GSGF_VALUE_CLASS (klass);

        gsgf_value_class->write_stream = gsgf_color_write_stream;
        
        g_type_class_add_private(klass, sizeof (GSGFColorPrivate));

        object_class->finalize = gsgf_color_finalize;
}

/**
 * gsgf_color_new:
 * @color: A #GSGFColorEnum.
 *
 * Creates a new #GSGFColor.
 *
 * Returns: The new #GSGFColor.
 */
GSGFColor *
gsgf_color_new (GSGFColorEnum color)
{
        GSGFColor *self = g_object_new (GSGF_TYPE_COLOR, NULL);

        self->priv->color = color;

        return self;
}

/**
 * gsgf_color_get_color:
 * @self: The #GSGFColor.
 *
 * Get the encapsulated color.
 *
 * Returns: The encapsulated #GSGFColorEnum.
 */
GSGFColorEnum
gsgf_color_get_color (const GSGFColor *self)
{
        if (!GSGF_IS_COLOR (self))
                g_return_val_if_fail (GSGF_IS_COLOR (self), GSGF_COLOR_WHITE);

        return self->priv->color;
}

/**
 * gsgf_color_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFColor from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * The @raw parameter should contain the color as a string.  Both abbreviated
 * ("B" or "W") and spelled out ("black" or "white") colors are accepted.
 * Case does not matter.
 *
 * Returns: The new #GSGFColor or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_color_new_from_raw(const GSGFRaw *raw, const GSGFFlavor *flavor,
                        const GSGFProperty *property, GError **error)
{
        const gchar *string;
        GSGFColorEnum color;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (flavor), NULL, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), NULL, error);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                             _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value(raw, 0);

        if ((string[0] == 'B' || string[0] == 'b') && !string[1]) {
                color = GSGF_COLOR_BLACK;
        } else if ((string[0] == 'W' || string[0] == 'w') && !string[1]) {
                   color = GSGF_COLOR_WHITE;
        } else if ((string[0] == 'B' || string[0] == 'b')
                   && (string[1] == 'L' || string[1] == 'l')
                   && (string[2] == 'A' || string[2] == 'a')
                   && (string[3] == 'C' || string[3] == 'c')
                   && (string[4] == 'K' || string[4] == 'k')
                   && !string[5]) {
                        color = GSGF_COLOR_BLACK;
        } else if ((string[0] == 'W' || string[0] == 'w')
                   && (string[1] == 'H' || string[1] == 'h')
                   && (string[2] == 'I' || string[2] == 'i')
                   && (string[3] == 'T' || string[3] == 't')
                   && (string[4] == 'E' || string[4] == 'e')
                   && !string[5]) {
                        color = GSGF_COLOR_WHITE;
        } else {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Unrecognized color identifier '%s'"),
                             string);
                return NULL;
        }

        return GSGF_COOKED_VALUE (gsgf_color_new (color));
}

static gboolean
gsgf_color_write_stream (const GSGFValue *_self,
                         GOutputStream *out, gsize *bytes_written,
                         GCancellable *cancellable, GError **error)
{
        GSGFColor *self = GSGF_COLOR (_self);
        gchar buffer[2];

        *bytes_written = 0;

        if (self->priv->color == GSGF_COLOR_BLACK)
                buffer[0] = 'B';
        else
                buffer[0] = 'W';
        buffer[1] = 0;

        if (!g_output_stream_write_all(out, buffer, 1,
                                       bytes_written,
                                       cancellable, error))
                return FALSE;

        return TRUE;
}

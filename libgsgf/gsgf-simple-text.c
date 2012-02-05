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
 * SECTION:gsgf-simple-text
 * @short_description: Strong primitive type for SGF simple texts.
 *
 * A #GSGFSimpleText encapsulates an SGF simple text.  Its main purpose is 
 * to allow for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

G_DEFINE_TYPE(GSGFSimpleText, gsgf_simple_text, GSGF_TYPE_TEXT)

static void
gsgf_simple_text_init(GSGFSimpleText *self)
{
}

static void
gsgf_simple_text_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_simple_text_parent_class)->finalize(object);
}

static void
gsgf_simple_text_class_init(GSGFSimpleTextClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gsgf_simple_text_finalize;
}

/**
 * gsgf_simple_text_new:
 * @value: The value to store.
 *
 * Creates a new #GSGFSimpleText and stores a copy of @value.
 *
 * Returns: The new #GSGFSimpleText.
 */
GSGFSimpleText *
gsgf_simple_text_new (const gchar *value)
{
        GSGFSimpleText *self = g_object_new (GSGF_TYPE_SIMPLE_TEXT, NULL);

        gsgf_text_set_value (GSGF_TEXT (self), value, TRUE, NULL);

        return self;
}

/**
 * gsgf_simple_text_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFSimpleText from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFSimpleText or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_simple_text_new_from_raw (const GSGFRaw *raw, const GSGFFlavor *flavor,
                               const GSGFProperty *property, GError **error)
{
        gsize list_length;
        gchar *value;

        gsgf_return_val_if_fail (GSGF_IS_RAW(raw), NULL, error);

        list_length = gsgf_raw_get_number_of_values(raw);

        if (!list_length) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Property without a value!"));
                return NULL;
        } else if (list_length != 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("A simple text property may only have one"
                            		" value, not %u!"),
                            (unsigned) list_length);
                return NULL;
        }

        value = gsgf_raw_get_value(raw, 0);

        return GSGF_COOKED_VALUE(gsgf_simple_text_new(value));
}

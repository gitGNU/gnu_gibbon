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
 * SECTION:gsgf-empty
 * @short_description: Strong primitive type for SGF emptys.
 *
 * A #GSGFEmpty encapsulates an SGF integer.  Its main purpose is to allow
 * for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <errno.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

G_DEFINE_TYPE(GSGFEmpty, gsgf_empty, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_empty_write_stream (const GSGFValue *self,
                                         GOutputStream *out,
                                         gsize *bytes_written,
                                         GCancellable *cancellable,
                                         GError **error);

static void
gsgf_empty_init(GSGFEmpty *self)
{
}

static void
gsgf_empty_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_empty_parent_class)->finalize(object);
}

static void
gsgf_empty_class_init(GSGFEmptyClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *value_class = GSGF_VALUE_CLASS (klass);

        value_class->write_stream = gsgf_empty_write_stream;

        object_class->finalize = gsgf_empty_finalize;
}

/**
 * gsgf_empty_new:
 *
 * Creates a new #GSGFEmpty.
 *
 * Returns: The new #GSGFEmpty.
 */
GSGFEmpty *
gsgf_empty_new ()
{
        GSGFEmpty *self = g_object_new(GSGF_TYPE_EMPTY, NULL);

        return self;
}

/**
 * gsgf_empty_new_from_raw:
 * @raw: A #GSGFRaw with one single, empty value.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFEmpty from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFEmpty or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_empty_new_from_raw(const GSGFRaw *raw, const GSGFFlavor *flavor,
                         const GSGFProperty *property, GError **error)
{
        const gchar *string;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("Only one value allowed for property `%s'"),
                            gsgf_property_get_id (property));
                return NULL;
        }
        string = gsgf_raw_get_value(raw, 0);
        if (string[0]) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Empty value required for property `%s'"),
                            gsgf_property_get_id (property));
                return NULL;
        }

        return GSGF_COOKED_VALUE(gsgf_empty_new());
}

static gboolean
gsgf_empty_write_stream (const GSGFValue *_self,
                         GOutputStream *out, gsize *bytes_written,
                         GCancellable *cancellable, GError **error)
{
        *bytes_written = 0;

        return TRUE;
}

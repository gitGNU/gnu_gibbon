/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-property
 * @short_description: An SGF property.
 *
 * A #GSGFProperty has a name (its identifier) and an associated list of values.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

struct _GSGFPropertyPrivate {
        gchar *id;

        GSGFCookedValue *value;
};

#define GSGF_PROPERTY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_PROPERTY,           \
                                      GSGFPropertyPrivate))
G_DEFINE_TYPE (GSGFProperty, gsgf_property, G_TYPE_OBJECT)

static void
gsgf_property_init(GSGFProperty *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_PROPERTY,
                        GSGFPropertyPrivate);

        self->priv->id = NULL;
        self->priv->value = NULL;
}

static void
gsgf_property_finalize(GObject *object)
{
        GSGFProperty *property = GSGF_PROPERTY (object);

        if (property->priv->id)
                g_free(property->priv->id);
        property->priv->id = NULL;

        if (property->priv->value) {
                g_object_unref(property->priv->value);
                property->priv->value = NULL;
        }

        G_OBJECT_CLASS (gsgf_property_parent_class)->finalize(object);
}

static void
gsgf_property_class_init(GSGFPropertyClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFPropertyPrivate));

        object_class->finalize = gsgf_property_finalize;
}

/**
 * gsgf_property_new:
 *
 * Build an empty #GSGFProperty in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFProperty.
 */
GSGFProperty *
_gsgf_property_new(const gchar *id)
{
        GSGFProperty *self = g_object_new(GSGF_TYPE_PROPERTY, NULL);

        self->priv->id = g_strdup(id);
        self->priv->value = GSGF_COOKED_VALUE(gsgf_raw_new(NULL));

        return self;
}

gboolean
_gsgf_property_write_stream(const GSGFProperty *self,
                            GOutputStream *out, gsize *bytes_written,
                            GCancellable *cancellable, GError **error)
{
        return gsgf_cooked_value_write_stream(self->priv->value,
                                              out, bytes_written,
                                              cancellable, error);
}

gboolean
_gsgf_property_add_value(GSGFProperty *property, const gchar *value)
{
g_print("Property %p (%s) add value %s.\n",
property, property->priv->id, value);
        _gsgf_raw_add_value(GSGF_RAW(property->priv->value), value);

        return TRUE;
}

/**
 * gsgf_property_get_value:
 *
 * @property: the #GSGFProperty.
 * @index: Index of the element.
 *
 * Retrieve the value of a property.
 *
 * Returns: Returns the value as a #GSGFCookedValue or %NULL f %index is out of range.
 */
GSGFCookedValue *
gsgf_property_get_value(const GSGFProperty *property)
{
        return property->priv->value;
}

gboolean
_gsgf_property_convert(GSGFProperty *self, const gchar *charset, GError **error)
{
        return _gsgf_raw_convert(GSGF_RAW(self->priv->value), charset, error);
}

gboolean
_gsgf_property_apply_flavor(GSGFProperty *self, const GSGFFlavor *flavor, GError **error)
{
        GSGFCookedValue *cooked;

        if (error)
                *error = NULL;

        if (!_gsgf_flavor_get_cooked_value(flavor, self->priv->id,
                                           GSGF_RAW(self->priv->value),
                                           &cooked, error)) {
                g_object_unref(self->priv->value);
                self->priv->value = cooked;
        }

        return TRUE;
}

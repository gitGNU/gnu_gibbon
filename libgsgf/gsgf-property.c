/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

typedef struct _GSGFPropertyPrivate GSGFPropertyPrivate;
struct _GSGFPropertyPrivate {
        gchar *id;

        GSGFCookedValue *value;

        GSGFNode *node;
};

#define GSGF_PROPERTY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_PROPERTY,           \
                                      GSGFPropertyPrivate))

static void gsgf_component_iface_init (GSGFComponentIface *iface);
G_DEFINE_TYPE_WITH_CODE (GSGFProperty, gsgf_property, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSGF_TYPE_COMPONENT,
                                                gsgf_component_iface_init))
static gboolean gsgf_property_write_stream (const GSGFComponent *self,
                                            GOutputStream *out,
                                            gsize *bytes_written,
                                            GCancellable *cancellable,
                                            GError **error);

static void
gsgf_property_init(GSGFProperty *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_PROPERTY,
                        GSGFPropertyPrivate);

        self->priv->id = NULL;
        self->priv->value = NULL;
        self->priv->node = NULL;
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

static void
gsgf_component_iface_init (GSGFComponentIface *iface)
{
        iface->write_stream = gsgf_property_write_stream;
}

/**
 * _gsgf_property_new:
 * @id: The id of the property.
 * @node: The node containing this property.
 *
 * Build an empty #GSGFProperty in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFProperty.
 */
GSGFProperty *
_gsgf_property_new(const gchar *id, GSGFNode *node)
{
        GSGFProperty *self;

        g_return_val_if_fail(id != NULL, NULL);
        g_return_val_if_fail(GSGF_IS_NODE(node), NULL);

        self = g_object_new(GSGF_TYPE_PROPERTY, NULL);

        self->priv->id = g_strdup(id);
        self->priv->value = GSGF_COOKED_VALUE(gsgf_raw_new(NULL));
        self->priv->node = node;

        return self;
}

static gboolean
gsgf_property_write_stream (const GSGFComponent *_self,
                            GOutputStream *out, gsize *bytes_written,
                            GCancellable *cancellable, GError **error)
{
        GSGFProperty *self;
        gsize written_here;

        g_return_val_if_fail(GSGF_IS_PROPERTY(_self), FALSE);
        g_return_val_if_fail(G_IS_OUTPUT_STREAM(out), FALSE);

        self = GSGF_PROPERTY (_self);

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, "[", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }
        *bytes_written += written_here;

        if (!gsgf_cooked_value_write_stream(self->priv->value,
                                            out, &written_here,
                                            cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }
        *bytes_written += written_here;

        if (!g_output_stream_write_all(out, "]", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }
        *bytes_written += written_here;

        return TRUE;
}

gboolean
_gsgf_property_add_value(GSGFProperty *property, const gchar *value)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE);
        g_return_val_if_fail(value != NULL, FALSE);

        _gsgf_raw_add_value(GSGF_RAW(property->priv->value), value);

        return TRUE;
}

/**
 * gsgf_property_get_value:
 * @property: the #GSGFProperty.
 *
 * Retrieve the value of a property.
 *
 * Returns: Returns the value as a #GSGFCookedValue.
 */
GSGFCookedValue *
gsgf_property_get_value(const GSGFProperty *property)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), NULL);

        return property->priv->value;
}

/**
 * gsgf_property_get_id:
 * @property: the #GSGFProperty.
 *
 * Retrieve the id of a property.
 *
 * Returns: Returns the id of the property.
 */
const gchar *
gsgf_property_get_id(const GSGFProperty *property)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), NULL);

        return property->priv->id;
}

/**
 * gsgf_property_get_node:
 * @property: the #GSGFProperty.
 *
 * Retrieve the #GSGFNode that this #GSGFProperty belongs to.
 *
 * Returns: Returns the #GSGFNode.
 */
GSGFNode *
gsgf_property_get_node(const GSGFProperty *property)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), NULL);

        return property->priv->node;
}

gboolean
_gsgf_property_convert(GSGFProperty *self, const gchar *charset, GError **error)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(self), FALSE);
        g_return_val_if_fail(charset != NULL, FALSE);

        return _gsgf_raw_convert(GSGF_RAW(self->priv->value), charset, error);
}

gboolean
_gsgf_property_apply_flavor(GSGFProperty *self, const GSGFFlavor *flavor, GError **error)
{
        GSGFCookedValue *cooked;

        g_return_val_if_fail(GSGF_IS_PROPERTY(self), FALSE);
        g_return_val_if_fail(GSGF_IS_FLAVOR(flavor), FALSE);

        if (gsgf_flavor_get_cooked_value (flavor, self,
                                          GSGF_RAW(self->priv->value),
                                          &cooked, error)) {
                g_object_unref(self->priv->value);
                self->priv->value = cooked;
        }

        return TRUE;
}

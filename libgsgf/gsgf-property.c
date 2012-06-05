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

        GSGFValue *value;

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
static gboolean gsgf_property_convert (GSGFComponent *self,
                                       const gchar *charset, GError **error);
static gboolean gsgf_property_cook (GSGFComponent *self,
                                    GSGFComponent **culprit,
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
        iface->cook = gsgf_property_cook;
        iface->_convert = gsgf_property_convert;
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
        self->priv->value = GSGF_VALUE (gsgf_raw_new (NULL));
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

        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (_self), FALSE, error);
        gsgf_return_val_if_fail (G_IS_OUTPUT_STREAM (out), FALSE, error);

        self = GSGF_PROPERTY (_self);

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, "[", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }
        *bytes_written += written_here;

        if (!gsgf_value_write_stream (self->priv->value,
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

        gsgf_raw_add_value(GSGF_RAW(property->priv->value), value);

        return TRUE;
}

/**
 * gsgf_property_get_value:
 * @self: the #GSGFProperty.
 *
 * Retrieve the value of a property.
 *
 * Returns: Returns the value as a #GSGFValue.
 */
GSGFValue *
gsgf_property_get_value (const GSGFProperty *self)
{
        g_return_val_if_fail (GSGF_IS_PROPERTY (self), NULL);

        return self->priv->value;
}

/**
 * gsgf_property_get_id:
 * @self: the #GSGFProperty.
 *
 * Retrieve the id of a property.
 *
 * Returns: (transfer none): Returns the id of the property.
 */
const gchar *
gsgf_property_get_id(const GSGFProperty *self)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(self), NULL);

        return self->priv->id;
}

/**
 * gsgf_property_get_node:
 * @self: the #GSGFProperty.
 *
 * Retrieve the #GSGFNode that this #GSGFProperty belongs to.
 *
 * Returns: Returns the #GSGFNode.
 */
GSGFNode *
gsgf_property_get_node(const GSGFProperty *self)
{
        g_return_val_if_fail(GSGF_IS_PROPERTY(self), NULL);

        return self->priv->node;
}

static gboolean
gsgf_property_convert (GSGFComponent *_self, const gchar *charset, GError **error)
{
        GSGFProperty *self;

        gsgf_return_val_if_fail (GSGF_IS_PROPERTY(_self), FALSE, error);
        gsgf_return_val_if_fail (charset != NULL, FALSE, error);

        self = GSGF_PROPERTY (_self);

        return _gsgf_raw_convert (GSGF_RAW (self->priv->value), charset, error);
}

static gboolean
gsgf_property_cook (GSGFComponent *_self, GSGFComponent **culprit, GError **error)
{
        GSGFCookedValue *cooked;
        GSGFProperty *self;
        const GSGFFlavor *flavor;

        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (_self), FALSE, error);

        self = GSGF_PROPERTY (_self);

        flavor = gsgf_node_get_flavor (self->priv->node);

        if (gsgf_flavor_get_cooked_value (flavor, self,
                                          GSGF_RAW(self->priv->value),
                                          &cooked, error)) {
                if (cooked) {
                        g_object_unref(self->priv->value);
                        self->priv->value = GSGF_VALUE (cooked);
                }
        } else {
                if (culprit)
                        *culprit = _self;

                return FALSE;
        }

        return TRUE;
}

/**
 * gsgf_property_set_value:
 * @self: The #GSGFProperty.
 * @value: The value to set.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Set the value of a property.  An old value will be deleted if present.
 *
 * If the value is a #GSGFRaw it will be automatically cooked.  If it is
 * already cooked it is assumed that you know what you are doing.
 *
 * The value is owned by the #GSGFProperty after you added it.  You should
 * not g_object_unref() it.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_property_set_value (GSGFProperty *self, GSGFValue *value, GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_VALUE (value), FALSE, error);

        if (self->priv->value)
                g_object_unref (self->priv->value);
        self->priv->value = value;

        if (!GSGF_IS_COOKED_VALUE (value)
            && !gsgf_property_cook (GSGF_COMPONENT (self), NULL, error))
                return FALSE;

        return TRUE;
}

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

struct _GSGFPropertyPrivate {
        gchar *id;
        GList *values;
        GSGFCookedValue (*value_constructor) (const gchar *value);
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
        self->priv->values = NULL;
}

static void
gsgf_property_finalize(GObject *object)
{
        GSGFProperty *property = GSGF_PROPERTY (object);

        if (property->priv->id)
                g_free(property->priv->id);
        property->priv->id = NULL;

        if (property->priv->values) {
                g_list_foreach(property->priv->values, 
                              (GFunc) g_object_unref, NULL);
                g_list_free(property->priv->values);
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

        return self;
}

gboolean
_gsgf_property_write_stream(const GSGFProperty *self,
                            GOutputStream *out, gsize *bytes_written,
                            GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;
        GSGFCookedValue *value;

        *bytes_written = 0;

        iter = self->priv->values;

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Attempt to write empty property"));
                return FALSE;
        }

        while (iter) {
                if (!g_output_stream_write_all(out, "[", 1, &written_here,
                                               cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                *bytes_written += written_here;

                value = GSGF_COOKED_VALUE(iter->data);
                if (!gsgf_cooked_value_write_stream(GSGF_COOKED_VALUE(iter->data),
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

                iter = iter->next;
        }

        return TRUE;
}

/**
 * _gsgf_property_add_value:
 *
 * @property: The property that the value is added to.
 * @text: The value itself.
 *
 * Add an (textual!) value to a #GSGFProperty.
 *
 * Possible errors depend on the particular flavor of SGF you expect.
 *
 * Returns: %TRUE on success, %FALSE on failure.
 */
gboolean
_gsgf_property_add_value(GSGFProperty *property, const gchar *_value)
{
        GSGFRaw* value = gsgf_raw_new(_value);

        property->priv->values = g_list_append(property->priv->values, value);

        return TRUE;
}

/**
 * gsgf_property_get_raw:
 *
 * @property: the #GSGFProperty.
 * @index: Index of the element.
 *
 * Retrieve the value of a property.
 *
 * Returns: Returns the value as a #GSGFCookedValue or %NULL f %index is out of range.
 */
GSGFCookedValue *
gsgf_property_get_value(const GSGFProperty *property, gsize i)
{
        gpointer value = g_list_nth_data(property->priv->values, i);

        if (!value)
                return NULL;

        return GSGF_COOKED_VALUE(value);
}

gboolean
_gsgf_property_convert(GSGFProperty *self, const gchar *charset, GError **error)
{
        gchar *converted;
        gsize bytes_written;
        GList *iter;
        GSGFRaw *value;

        if (error)
                *error = NULL;

        iter = self->priv->values;

        while (iter) {
                value = GSGF_RAW(iter->data);

                converted = g_convert(gsgf_raw_get_value(value), -1, "UTF-8", charset,
                                      NULL, &bytes_written, NULL);
                if (!converted)
                        return FALSE;

                gsgf_raw_set_value(value, converted, FALSE);

                iter = iter->next;
        }

        return TRUE;
}

gboolean
_gsgf_property_apply_flavor(GSGFProperty *self, const GSGFFlavor *flavor, GError **error)
{
        if (error)
                *error = NULL;

        return NULL;
}

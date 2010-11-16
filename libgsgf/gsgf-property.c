/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
        GList *values;
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

        self->priv->values = NULL;
}

static void
gsgf_property_finalize(GObject *object)
{
        GSGFProperty *property = GSGF_PROPERTY (object);

        if (property->priv->values) {
                g_list_foreach(property->priv->values, (GFunc) g_free,
                               NULL);
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
gsgf_property_new()
{
        GSGFProperty *self = g_object_new(GSGF_TYPE_PROPERTY, NULL);

        return self;
}

gboolean
_gsgf_property_write_stream(const GSGFProperty *self,
                            GOutputStream *out, gsize *bytes_written,
                            GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;

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

                if (!g_output_stream_write_all(out, iter->data, strlen(iter->data),
                                               &written_here, cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;

                if (!g_output_stream_write_all(out, "]", 1, &written_here,
                                               cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;
        }

        return TRUE;
}

/**
 * gsgf_property_add_value:
 *
 * @property: The property that the value is added to.
 * @text: The value itself.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Add an (textual!) value to a #GSGFProperty.
 *
 * Possible errors depend on the particular flavor of SGF you expect.
 *
 * Returns: %TRUE on success, %FALSE on failure.
 */
gboolean
gsgf_property_add_value(GSGFProperty *property, const gchar *value, GError **error)
{
        *error = NULL;

        property->priv->values = g_list_append(property->priv->values, g_strdup(value));
}

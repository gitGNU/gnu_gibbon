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
 * SECTION:gsgf-list_of
 * @short_description: ListOfd data in SGF files.
 *
 * A #GSGFListOf is a list of #GSGFCookedValue objects.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFListOfPrivate {
        GList *values;
};

#define GSGF_LIST_OF_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_LIST_OF,           \
                                      GSGFListOfPrivate))

G_DEFINE_TYPE(GSGFListOf, gsgf_list_of, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_list_of_write_stream(const GSGFCookedValue *self,
                                      GOutputStream *out, gsize *bytes_written,
                                      GCancellable *cancellable, GError **error);

static void
gsgf_list_of_init(GSGFListOf *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_LIST_OF,
                        GSGFListOfPrivate);

        self->priv->values = NULL;
}

static void
gsgf_list_of_finalize(GObject *object)
{
        GSGFListOf *self = GSGF_LIST_OF(object);

        if (self->priv->values) {
                g_list_foreach(self->priv->values, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->values);
        }

        G_OBJECT_CLASS (gsgf_list_of_parent_class)->finalize(object);
}

static void
gsgf_list_of_class_init(GSGFListOfClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS(klass);
        GSGFCookedValueClass *gsgf_cooked_value_class = GSGF_COOKED_VALUE_CLASS(klass);

        g_type_class_add_private(klass, sizeof(GSGFListOfPrivate));

        gsgf_cooked_value_class->write_stream = gsgf_list_of_write_stream;

        object_class->finalize = gsgf_list_of_finalize;
}

/**
 * gsgf_list_of_new:
 * @value: The first value to store.
 * @Varargs: Other optional values, terminated by %NULL.
 *
 * Creates a new #GSGFListOf from a list of #GSGFCookedValue objects.  The
 * stored items are "hijacked" and are now considered property of the list_ofd
 * object, and you must not g_object_unref() them yourself.
 *
 * Returns: The new #GSGFListOf.
 */
GSGFListOf *
gsgf_list_of_new (GSGFCookedValue *value, ...)
{
        GSGFListOf *self;
        GList *values;
        va_list args;

        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), NULL);

        values = g_list_append(NULL, value);
        va_start(args, value);

        while (1) {
                value = va_arg(args, GSGFListOf *);
                if (!value)
                        break;
                if (!GSGF_IS_COOKED_VALUE(value)) {
                        g_list_free(values);
                        /* This will always fail on purpose.  We just want the
                         * error message to be printed in a consistent way.
                         */
                        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), NULL);
                }
                values = g_list_append(values, value);
        }

        va_end(args);

        self = g_object_new(GSGF_TYPE_LIST_OF, NULL);

        self->priv->values = values;

        return self;
}

/**
 * gsgf_list_of_get_value:
 * @self: The #GSGFListOf object.
 * @i: Position in the property list.
 *
 * Retrieve the value stored at position @i.
 *
 * Returns: The value stored at position @i or %NULL.
 */
GSGFCookedValue *
gsgf_list_of_get_value(const GSGFListOf *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_LIST_OF(self), NULL);

        return GSGF_COOKED_VALUE(g_list_nth_data(self->priv->values, i));
}

static gboolean
gsgf_list_of_write_stream(const GSGFCookedValue *_self,
                      GOutputStream *out, gsize *bytes_written,
                      GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;
        GSGFCookedValue *value;
        GSGFListOf *self = GSGF_LIST_OF(_self);

        *bytes_written = 0;

        iter = self->priv->values;

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Attempt to write empty property"));
                return FALSE;
        }

        while (iter) {
                value = GSGF_COOKED_VALUE(iter->data);
                if (!gsgf_cooked_value_write_stream(value, out, &written_here,
                                                    cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                *bytes_written += written_here;

                iter = iter->next;

                if (iter) {
                        if (!g_output_stream_write_all(out, ":", 1, &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                return FALSE;
                        }
                        *bytes_written += written_here;
                }
        }

        return TRUE;
}

/**
 * gsgf_list_of_get_number_of_values:
 * @self: The #GSGFListOf object.
 *
 * Get the number of items.
 *
 * Returns: The number of values stored.
 */
gsize
gsgf_list_of_get_number_of_values (const GSGFListOf *self)
{
        g_return_val_if_fail(GSGF_IS_LIST_OF(self), 0);

        return g_list_length(self->priv->values);
}

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
 * SECTION:gsgf-compose
 * @short_description: Composed data in SGF files.
 *
 * A #GSGFCompose is a list of #GSGFCookedValue objects.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFComposePrivate GSGFComposePrivate;
struct _GSGFComposePrivate {
        GList *values;
};

#define GSGF_COMPOSE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COMPOSE,           \
                                      GSGFComposePrivate))

G_DEFINE_TYPE(GSGFCompose, gsgf_compose, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_compose_write_stream (const GSGFValue *self,
                                           GOutputStream *out,
                                           gsize *bytes_written,
                                           GCancellable *cancellable,
                                           GError **error);

static void
gsgf_compose_init(GSGFCompose *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_COMPOSE,
                        GSGFComposePrivate);

        self->priv->values = NULL;
}

static void
gsgf_compose_finalize(GObject *object)
{
        GSGFCompose *self = GSGF_COMPOSE(object);

        if (self->priv->values) {
                g_list_foreach(self->priv->values, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->values);
        }

        G_OBJECT_CLASS (gsgf_compose_parent_class)->finalize(object);
}

static void
gsgf_compose_class_init(GSGFComposeClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *gsgf_value_class = GSGF_VALUE_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GSGFComposePrivate));

        gsgf_value_class->write_stream = gsgf_compose_write_stream;

        object_class->finalize = gsgf_compose_finalize;
}

/**
 * gsgf_compose_new:
 * @value: The first value to store.
 * @...: Other optional values, terminated by %NULL.
 *
 * Creates a new #GSGFCompose from a list of #GSGFCookedValue objects.  The
 * stored items are "hijacked" and are now considered property of the composed
 * object, and you must not g_object_unref() them yourself.
 *
 * Note that you must pass cooked values!  Passing a #GSGFRaw would not make
 * sense since a #GSGFRaw can actually be a list of values.
 *
 * Returns: The new #GSGFCompose.
 */
GSGFCompose *
gsgf_compose_new (GSGFCookedValue *value, ...)
{
        GSGFCompose *self;
        GList *values;
        va_list args;

        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), NULL);

        values = g_list_append(NULL, value);
        va_start(args, value);

        while (1) {
                value = va_arg(args, GSGFCookedValue *);
                if (!value)
                        break;
                if (!GSGF_IS_COOKED_VALUE(value)) {
                        va_end (args);
                        g_list_free(values);
                        /* This will always fail on purpose.  We just want the
                         * error message to be printed in a consistent way.
                         */
                        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), NULL);
                }
                values = g_list_append(values, value);
        }

        va_end(args);

        self = g_object_new(GSGF_TYPE_COMPOSE, NULL);

        self->priv->values = values;

        return self;
}

/**
 * gsgf_compose_get_value:
 * @self: The #GSGFCompose object.
 * @i: Position in the property list.
 *
 * Retrieve the value stored at position @i.
 *
 * Returns: The value stored at position @i or %NULL.
 */
GSGFCookedValue *
gsgf_compose_get_value(const GSGFCompose *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_COMPOSE(self), NULL);

        return GSGF_COOKED_VALUE(g_list_nth_data(self->priv->values, i));
}

static gboolean
gsgf_compose_write_stream (const GSGFValue *_self,
                           GOutputStream *out, gsize *bytes_written,
                           GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;
        GSGFValue *value;
        GSGFCompose *self = GSGF_COMPOSE (_self);

        *bytes_written = 0;

        iter = self->priv->values;

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Attempt to write empty property"));
                return FALSE;
        }

        while (iter) {
                value = GSGF_VALUE(iter->data);
                if (!gsgf_value_write_stream(value, out, &written_here,
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
 * gsgf_compose_get_number_of_values:
 * @self: The #GSGFCompose object.
 *
 * Get the number of items.
 *
 * Returns: The number of values stored.
 */
gsize
gsgf_compose_get_number_of_values (const GSGFCompose *self)
{
        g_return_val_if_fail(GSGF_IS_COMPOSE(self), 0);

        return g_list_length(self->priv->values);
}

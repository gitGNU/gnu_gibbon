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
 * SECTION:gsgf-number
 * @short_description: Strong primitive type for SGF numbers.
 *
 * A #GSGFNumber encapsulates an SGF integer.  Its main purpose is to allow
 * for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <errno.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFNumberPrivate GSGFNumberPrivate;
struct _GSGFNumberPrivate {
        gint64 value;
};

#define GSGF_NUMBER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_NUMBER,           \
                                      GSGFNumberPrivate))

G_DEFINE_TYPE(GSGFNumber, gsgf_number, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_number_write_stream (const GSGFValue *self,
                                          GOutputStream *out,
                                          gsize *bytes_written,
                                          GCancellable *cancellable,
                                          GError **error);

static void
gsgf_number_init(GSGFNumber *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_NUMBER,
                        GSGFNumberPrivate);

        self->priv->value = 0;
}

static void
gsgf_number_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_number_parent_class)->finalize(object);
}

static void
gsgf_number_class_init(GSGFNumberClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *value_class = GSGF_VALUE_CLASS(klass);

        g_type_class_add_private(klass, sizeof(GSGFNumberPrivate));

        value_class->write_stream = gsgf_number_write_stream;

        object_class->finalize = gsgf_number_finalize;
}

/**
 * gsgf_number_new:
 * @value: The value to store.
 *
 * Creates a new #GSGFNumber.
 *
 * Returns: The new #GSGFNumber.
 */
GSGFNumber *
gsgf_number_new (gint64 value)
{
        GSGFNumber *self = g_object_new(GSGF_TYPE_NUMBER, NULL);

        self->priv->value = value;

        return self;
}

/**
 * gsgf_number_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFNumber from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFNumber or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_number_new_from_raw(const GSGFRaw *raw, const GSGFFlavor *flavor,
                         const GSGFProperty *property, GError **error)
{
        gchar *endptr;
        gint64 value;
        const gchar *string;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value(raw, 0);

        /* _gsgf_real_new implicitely resets errno.  We do the same explicitely.  */
        errno = 0;

        value = g_ascii_strtoll(string, &endptr, 012);

        if (errno) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s': %s"), string, strerror(errno));
                return NULL;
        }

        if (endptr == string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s'"), string);
                return NULL;
        }

        if (*endptr) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Trailing garbage after number in '%s'"), string);
                return NULL;
        }

        return GSGF_COOKED_VALUE(gsgf_number_new(value));
}

/**
 * gsgf_number_set_value:
 * @self: The #GSGFNumber.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFNumber.
 */
void
gsgf_number_set_value(GSGFNumber *self, gint64 value)
{
        g_return_if_fail(GSGF_IS_NUMBER(self));

        self->priv->value = value;
}

/**
 * gsgf_number_get_value:
 * @self: The #GSGFNumber.
 *
 * Retrieve the value stored in a #GSGFNumber.
 *
 * Returns: the value stored.
 */
gint64
gsgf_number_get_value(const GSGFNumber *self)
{
        g_return_val_if_fail(GSGF_IS_NUMBER(self), 0);

        return self->priv->value;
}

static gboolean
gsgf_number_write_stream (const GSGFValue *_self,
                          GOutputStream *out, gsize *bytes_written,
                          GCancellable *cancellable, GError **error)
{
        GSGFNumber *self = GSGF_NUMBER(_self);
        gchar *value;

        *bytes_written = 0;

        value = g_strdup_printf("%lld",
        		                (long long int) gsgf_number_get_value(self));
        if (!g_output_stream_write_all(out, value, strlen(value),
                                       bytes_written,
                                       cancellable, error)) {
                g_free (value);
                return FALSE;
        }

        g_free(value);

        return TRUE;
}

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
 * SECTION:gsgf-text
 * @short_description: Strong primitive type for SGF texts.
 *
 * A #GSGFText encapsulates an SGF text.  Its main purpose is
 * to allow for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFTextPrivate {
        gchar *value;
};

#define GSGF_TEXT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_TEXT,           \
                                      GSGFTextPrivate))

G_DEFINE_TYPE(GSGFText, gsgf_text, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_text_write_stream(const GSGFCookedValue *self,
                                       GOutputStream *out, gsize *bytes_written,
                                       GCancellable *cancellable, GError **error);

static void
gsgf_text_init(GSGFText *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_TEXT,
                        GSGFTextPrivate);

        self->priv->value = NULL;
}

static void
gsgf_text_finalize(GObject *object)
{
        GSGFText *self = GSGF_TEXT(object);

        if (self->priv->value)
                g_free(self->priv->value);
        self->priv->value = NULL;

        G_OBJECT_CLASS (gsgf_text_parent_class)->finalize(object);
}

static void
gsgf_text_class_init(GSGFTextClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS(klass);
        GSGFCookedValueClass *gsgf_cooked_value_class = GSGF_COOKED_VALUE_CLASS(klass);

        gsgf_cooked_value_class = GSGF_COOKED_VALUE_CLASS(klass);

        g_type_class_add_private(klass, sizeof(GSGFTextPrivate));

        gsgf_cooked_value_class->write_stream = gsgf_text_write_stream;

        object_class->finalize = gsgf_text_finalize;
}

/**
 * gsgf_text_new:
 * @value: The value to store.
 *
 * Creates a new #GSGFText and stores a copy of @value.
 *
 * Returns: The new #GSGFText.
 */
GSGFText *
gsgf_text_new (const gchar *value)
{
        GSGFText *self = g_object_new(GSGF_TYPE_TEXT, NULL);

        self->priv->value = g_strdup(value);

        return self;
}

/**
 * gsgf_text_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFText from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFText or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_text_new_from_raw (const GSGFRaw *raw, GError **error)
{
        gsize list_length = gsgf_raw_get_number_of_values(raw);
        gchar *value;

        if (!list_length) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Property without a value!"));
                return NULL;
        } else if (list_length != 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("A text property may only have one value, not %u!"),
                            list_length);
                return NULL;
        }

        value = gsgf_raw_get_value(raw, 0);

        return GSGF_COOKED_VALUE(gsgf_text_new(value));
}

/**
 * gsgf_text_set_value:
 * @self: The #GSGFText.
 * @value: The new value to store.
 * @copy: Flag that indicates whether to create a copy of the data.
 *
 * Stores a new value in a #GSGFText.  If @copy is %TRUE, a copy is
 * stored.  If it is %FALSE the @value is stored directly.
 */
void
gsgf_text_set_value(GSGFText *self, const gchar *value,
                    gboolean copy)
{
        if (self->priv->value)
                g_free(self->priv->value);

        if (copy)
                self->priv->value = g_strdup(value);
        else
                self->priv->value = (gchar *) value;
}

/**
 * gsgf_text_get_value:
 * @self: The #GSGFText.
 *
 * Retrieve the value stored in a #GSGFText.
 *
 * Returns: the value stored.
 */
gchar *
gsgf_text_get_value(const GSGFText *self)
{
        return self->priv->value;
}

static gboolean
gsgf_text_write_stream(const GSGFCookedValue *_self,
                       GOutputStream *out, gsize *bytes_written,
                       GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GSGFText *self = GSGF_TEXT(_self);

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, "[", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }
        *bytes_written += written_here;

        if (!g_output_stream_write_all(out, self->priv->value,
                                       strlen(self->priv->value),
                                       bytes_written,
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

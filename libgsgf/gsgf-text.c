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
 * @short_description: Strong primitive type for SGF simple texts.
 *
 * A #GSGFText encapsulates an SGF simple text.  Its main purpose is 
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

G_DEFINE_TYPE (GSGFText, gsgf_text, G_TYPE_OBJECT)

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
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFTextPrivate));

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
 * gsgf_text_set_value:
 * @self: The #GSGFText.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFText.  The value is copied first.
 */
void
gsgf_text_set_value(GSGFText *self, const gchar *value)
{
        if (self->priv->value)
                g_free(self->priv->value);

        self->priv->value = g_strdup(value);
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

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
 * SECTION:gsgf-simple_text
 * @short_description: Strong primitive type for SGF simple texts.
 *
 * A #GSGFSimpleText encapsulates an SGF simple text.  Its main purpose is 
 * to allow for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFSimpleTextPrivate {
        gchar *value;
};

#define GSGF_SIMPLE_TEXT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_SIMPLE_TEXT,           \
                                      GSGFSimpleTextPrivate))

G_DEFINE_TYPE (GSGFSimpleText, gsgf_simple_text, G_TYPE_OBJECT)

static void
gsgf_simple_text_init(GSGFSimpleText *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_SIMPLE_TEXT,
                        GSGFSimpleTextPrivate);

        self->priv->value = NULL;
}

static void
gsgf_simple_text_finalize(GObject *object)
{
        GSGFSimpleText *self = GSGF_SIMPLE_TEXT(object);

        if (self->priv->value)
                g_free(self->priv->value);
        return NULL;

        G_OBJECT_CLASS (gsgf_simple_text_parent_class)->finalize(object);
}

static void
gsgf_simple_text_class_init(GSGFSimpleTextClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFSimpleTextPrivate));

        object_class->finalize = gsgf_simple_text_finalize;
}

/**
 * gsgf_simple_text_new:
 * @value: The value to store.
 *
 * Creates a new #GSGFSimpleText and stores a copy of @value.
 *
 * Returns: The new #GSGFSimpleText.
 */
GSGFSimpleText *
gsgf_simple_text_new (const gchar *value)
{
        GSGFSimpleText *self = g_object_new(GSGF_TYPE_SIMPLE_TEXT, NULL);

        self->priv->value = g_strdup(value);

        return self;
}

/**
 * gsgf_simple_text_set_value:
 * @self: The #GSGFSimpleText.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFSimpleText.  The value is copied first.
 */
void
gsgf_simple_text_set_value(GSGFSimpleText *self, const gchar *value)
{
        if (self->priv->value)
                g_free(self->priv->value);

        self->priv->value = g_strdup(value);
}

/**
 * gsgf_simple_text_get_value:
 * @self: The #GSGFSimpleText.
 *
 * Retrieve the value stored in a #GSGFSimpleText.
 *
 * Returns: the value stored.
 */
gchar *
gsgf_simple_text_get_value(const GSGFSimpleText *self)
{
        return self->priv->value;
}

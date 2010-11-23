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
 * SECTION:gsgf-raw
 * @short_description: Raw, unqualified data in SGF files.
 *
 * A #GSGFRaw is the only #GSGFCookedValue that is not a cooked value;
 * the name sort of suggests that.  You should never use a #GSGFRaw
 * directly.  It is internally used, when parsing data before it
 * gets cooked.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFRawPrivate {
        gchar *value;
};

#define GSGF_RAW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_RAW,           \
                                      GSGFRawPrivate))

G_DEFINE_TYPE (GSGFRaw, gsgf_raw, G_TYPE_OBJECT)

static void
gsgf_raw_init(GSGFRaw *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_RAW,
                        GSGFRawPrivate);

        self->priv->value = NULL;
}

static void
gsgf_raw_finalize(GObject *object)
{
        GSGFRaw *self = GSGF_RAW(object);

        if (self->priv->value)
                g_free(self->priv->value);
        self->priv->value = NULL;

        G_OBJECT_CLASS (gsgf_raw_parent_class)->finalize(object);
}

static void
gsgf_raw_class_init(GSGFRawClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFRawPrivate));

        object_class->finalize = gsgf_raw_finalize;
}

/**
 * gsgf_raw_new:
 * @value: The value to store.
 *
 * Creates a new #GSGFRaw and stores a copy of @value.
 *
 * Returns: The new #GSGFRaw.
 */
GSGFRaw *
gsgf_raw_new (const gchar *value)
{
        GSGFRaw *self = g_object_new(GSGF_TYPE_RAW, NULL);

        self->priv->value = g_strdup(value);

        return self;
}

/**
 * gsgf_raw_set_value:
 * @self: The #GSGFRaw.
 * @value: The new value to store.
 * @copy: Flag that indicates whether to create a copy of the data.
 *
 * Stores a new value in a #GSGFRaw.  If @copy is %TRUE, a copy is
 * stored.  If it is %FALSE the @value is stored directly.
 */
void
gsgf_raw_set_value(GSGFRaw *self, const gchar *value, gboolean copy)
{
        if (self->priv->value)
                g_free(self->priv->value);

        if (copy)
                self->priv->value = g_strdup(value);
        else
                self->priv->value = value;
}

/**
 * gsgf_raw_get_value:
 * @self: The #GSGFRaw.
 *
 * Retrieve the value stored in a #GSGFRaw.
 *
 * Returns: the value stored.
 */
gchar *
gsgf_raw_get_value(const GSGFRaw *self)
{
        return self->priv->value;
}

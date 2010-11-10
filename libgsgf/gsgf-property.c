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

enum gsgf_property_type {
        GSGF_PROPERTY_TEXT,
        GSGF_PROPERTY_NUMBER,
        GSGF_PROPERTY_REAL,
};

struct _GSGFPropertyValue {
        enum gsgf_property_type type;
        union gsgf_property_union {
               gdouble real;
               gint number;
               gchar *text;
        } value;
};

struct _GSGFPropertyPrivate {
        gchar *id;
        GList *values;
};

#define GSGF_PROPERTY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_PROPERTY,           \
                                      GSGFPropertyPrivate))
G_DEFINE_TYPE (GSGFProperty, gsgf_property, G_TYPE_OBJECT)

static void gsgf_property_value_free(struct _GSGFPropertyValue *value);

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
                g_list_foreach(property->priv->values, (GFunc) gsgf_property_value_free,
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
gsgf_property_new(const gchar *id)
{
        GSGFProperty *self = g_object_new(GSGF_TYPE_PROPERTY, NULL);

        self->priv->id = g_strdup(id);

        return self;
}

static void
gsgf_property_value_free(struct _GSGFPropertyValue *value)
{
        if (value->type == GSGF_PROPERTY_TEXT)
                g_free(value->value.text);
}

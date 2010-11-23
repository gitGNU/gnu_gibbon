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
 * SECTION:gsgf-number
 * @short_description: Strong primitive type for SGF numbers.
 *
 * A #GSGFNumber encapsulates an SGF integer.  Its main purpose is to allow
 * for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFNumberPrivate {
        gint64 value;
};

#define GSGF_NUMBER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_NUMBER,           \
                                      GSGFNumberPrivate))

G_DEFINE_TYPE (GSGFNumber, gsgf_number, G_TYPE_OBJECT)

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

        g_type_class_add_private(klass, sizeof(GSGFNumberPrivate));

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
 * gsgf_number_set_value:
 * @self: The #GSGFNumber.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFNumber.
 */
void
gsgf_number_set_value(GSGFNumber *self, gint64 value)
{
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
        return self->priv->value;
}

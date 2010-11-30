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
 * SECTION:gsgf-real
 * @short_description: Strong primitive type for SGF floating point numbers.
 *
 * A #GSGFReal encapsulates an SGF integer.  Its main purpose is to allow
 * for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFRealPrivate {
        gdouble value;
};

#define GSGF_REAL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_REAL,           \
                                      GSGFRealPrivate))

G_DEFINE_TYPE (GSGFReal, gsgf_real, GSGF_TYPE_COOKED_VALUE)

static GRegex *double_pattern = NULL;

static void
gsgf_real_init(GSGFReal *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_REAL,
                        GSGFRealPrivate);

        self->priv->value = 0;
}

static void
gsgf_real_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_real_parent_class)->finalize(object);
}

static void
gsgf_real_class_init(GSGFRealClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFRealPrivate));

        double_pattern = g_regex_new("^[+-]?[0-9]+(?:\\.[0-9]+)?$", 0, 0, NULL);

        object_class->finalize = gsgf_real_finalize;
}

/**
 * gsgf_real_new:
 * @value: The value to store as a string or %NULL.
 *
 * Creates a new #GSGFReal.
 *
 * Returns: The new #GSGFReal.
 */
GSGFReal *
gsgf_real_new(gdouble value)
{
        GSGFReal *self = g_object_new(GSGF_TYPE_REAL, NULL);

        self->priv->value = value;

        return self;
}

GSGFReal *
_gsgf_real_new(const gchar *string, GError **error)
{
        GSGFReal *self;
        gchar *endptr;
        gdouble value;

        if (error)
                *error = NULL;

        if (!double_pattern)
                double_pattern = g_regex_new("^[+-]?[0-9]+(?:\\.[0-9]+)?$", 0, 0, NULL);

        if (!g_regex_match(double_pattern, string, 0, NULL)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s'"), string);
                return NULL;
        }

        value = g_ascii_strtod(string, &endptr);

        return gsgf_real_new(value);
}

/**
 * gsgf_real_set_value:
 * @self: The #GSGFReal.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFReal.
 */
void
gsgf_real_set_value(GSGFReal *self, gdouble value)
{
        self->priv->value = value;
}

/**
 * gsgf_real_get_value:
 * @self: The #GSGFReal.
 *
 * Retrieve the value stored in a #GSGFReal.
 *
 * Returns: the value stored.
 */
gdouble
gsgf_real_get_value(const GSGFReal *self)
{
        return self->priv->value;
}

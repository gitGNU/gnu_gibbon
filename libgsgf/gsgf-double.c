/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gsgf-double
 * @short_description: An integer number that is either one or two.
 * Since: 0.1.0
 *
 * Emphasizable properties.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

G_DEFINE_TYPE (GSGFDouble, gsgf_double, GSGF_TYPE_NUMBER)

static void 
gsgf_double_init (GSGFDouble *self)
{
}

static void
gsgf_double_finalize (GObject *object)
{
        G_OBJECT_CLASS (gsgf_double_parent_class)->finalize(object);
}

static void
gsgf_double_class_init (GSGFDoubleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gsgf_double_finalize;
}

/**
 * gsgf_double_new:
 * @grade: "How" much double.
 *
 * Create a #GSGFDouble.
 *
 * Returns: The newly created #GSGFDouble.
 */
GSGFDouble *
gsgf_double_new (GSGFDoubleEnum grade)
{
        GSGFDouble *self = g_object_new (GSGF_TYPE_DOUBLE, NULL);

        if (grade == GSGF_DOUBLE_VERY)
                gsgf_number_set_value (GSGF_NUMBER (self), 2);
        else
                gsgf_number_set_value (GSGF_NUMBER (self), 1);

        return self;
}

/**
 * gsgf_double_new_from_raw:
 * @raw: A #GSGFRaw containing the value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFDouble from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFDouble or %NULL in case of an error.
 */
GSGFCookedValue *gsgf_double_new_from_raw(const GSGFRaw* raw,
                                          const GSGFFlavor *flavor,
                                          const struct _GSGFProperty *property,
                                          GError **error)
{
        GSGFDouble *self;
        const gchar *string;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("Only one value allowed for property `%s'"),
                            gsgf_property_get_id (property));
                return NULL;
        }

        string = gsgf_raw_get_value(raw, 0);
        if (string[0] < '1' || string[0] > '2' || string[1]) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_DOUBLE_OUT_OF_RANGE,
                            _("SGF double must be 1 or 2, not `%s'"),
                            string);
                return NULL;
        }

        self = gsgf_double_new (string[0] == '1' ?
                                GSGF_DOUBLE_NORMAL : GSGF_DOUBLE_VERY);

        return GSGF_COOKED_VALUE (self);
}


/**
 * gsgf_double_set_value:
 * @self: The #GSGFDouble to change.
 * @value: The #GSGFDoubleEnum containing the value that should be stored.
 *
 * Store a #GSGFDoubleEnum in a #GSGFDouble.
 */
void
gsgf_double_set_value (GSGFDouble *self, GSGFDoubleEnum value)
{
        g_return_if_fail (GSGF_IS_DOUBLE (self));

        gsgf_number_set_value (GSGF_NUMBER (self), value);
}

/**
 * gsgf_double_get_value:
 * @self: The #GSGFDouble to examine.
 *
 * Get the value of a #GSGFDouble.
 *
 * Store a #GSGFDoubleEnum in a #GSGFDouble.
 */
GSGFDoubleEnum
gsgf_double_get_value (const GSGFDouble *self)
{
        g_return_val_if_fail (GSGF_IS_DOUBLE (self), GSGF_DOUBLE_NORMAL);

        if (2 == gsgf_number_get_value (GSGF_NUMBER (self)))
                return GSGF_DOUBLE_VERY;
        else
                return GSGF_DOUBLE_NORMAL;
}

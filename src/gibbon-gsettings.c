/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-gsettings.h"

GVariant * 
gibbon_gsettings_bind_string_to_port (const GValue *value,
                                      const GVariantType *variant_type,
                                      gpointer user_data)
{
        GVariant *variant;

        gchar *s = pango_trim_string (g_value_get_string (value));
        char *endptr;
        long unsigned int port;

        port = strtol (s, &endptr, 10);
        if (port && port <= 65535 && *s && !*endptr) {
                variant = g_variant_new_uint16 (port);
        } else {
                variant = g_variant_new_uint16 (0);
        }

        g_free (s);

        return variant;
}

gboolean
gibbon_gsettings_bind_port_to_string (GValue *value,
                                      GVariant *variant,
                                      gpointer user_data)
{
        guint16 port = g_variant_get_uint16 (variant);

        g_value_take_string (value, g_strdup_printf ("%u", port));

        return TRUE;
}

GVariant * 
gibbon_gsettings_bind_trimmed_string (const GValue *value,
                                      const GVariantType *variant_type,
                                      gpointer user_data)
{
        gchar *s = pango_trim_string (g_value_get_string (value));
        GVariant *variant = g_variant_new_string (s);

        g_free (s);

        return variant;
}


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
 * SECTION:gsgf-property-handler
 * @short_description: Constraints for a #GSGFProperty.
 *
 * This class is internal to libgsgf.  You only have to bother about it,
 * when extending libgsgf.
 *
 * A #GSGFPropertyHandler defines constraints for SGF properties.  The
 * methods stored in the handler are called, whenever the property gets
 * updated.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

/**
 * GSGFPropertyHandler:
 *
 * A handler for a specific #GSGFProperty.
 */
GType
gsgf_property_handler_get_type(void)
{
        static volatile gsize type_volatile = 0;

        if (g_once_init_enter(&type_volatile)) {
                GType type = g_boxed_type_register_static(
                        g_intern_static_string("GSGFPropertyHandler"),
                        (GBoxedCopyFunc) gsgf_property_handler_copy,
                        (GBoxedFreeFunc) gsgf_property_handler_destroy);
                g_once_init_leave(&type_volatile, type);
        }

        return type_volatile;
}

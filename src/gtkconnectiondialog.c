/* This file is part of Gibbon
 * Copyright (C) 2009 Guido Flohr
 * 
 * Gibbon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n.h>

#include "gtkconnectiondialog.h"

GtkWidget *connection_dialog(GtkBuilder *builder)
{
        GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                                "connection_dialog"));
        if (!window) {
                g_print (_("Internal error: Cannot find widget '%s'!\n"),
                           "connection_dialog");
                return NULL;
        }
        
        gtk_builder_connect_signals (builder, NULL);          
        g_object_unref (G_OBJECT (builder));

        return window;
}

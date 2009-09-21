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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon.h"
#include "gui.h"

G_MODULE_EXPORT void 
on_window_destroy (GtkObject *object, gpointer user_data)
{
        gtk_main_quit();
}

G_MODULE_EXPORT void
toggle_connect (GtkObject *object, gpointer user_data)
{
        if (gibbon_connection_disconnected (connection))
                gtk_dialog_run (GTK_DIALOG (connection_dialog));
	else
		gibbon_connection_disconnect (connection);
}

G_MODULE_EXPORT void
on_connect_menu_item_activate (GtkObject *object, gpointer user_data)
{
        if (gibbon_connection_disconnected (connection))
                gtk_dialog_run (GTK_DIALOG (connection_dialog));
	else
		g_print (_("Already connected.\n"));
}

G_MODULE_EXPORT void
on_disconnect_menu_item_activate (GtkObject *object, gpointer user_data)
{
        if (!gibbon_connection_disconnected (connection))
                gibbon_connection_disconnect (connection);
	else
		g_print (_("Not connected.\n"));
}


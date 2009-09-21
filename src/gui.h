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

#ifndef _GIBBON_GUI_H
#define _GIBBON_GUI_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

extern GtkBuilder *builder;

extern GtkWidget *window;
extern GtkWidget *connection_dialog;

extern gint init_gui (const gchar *builder_filename);
extern const gchar *get_trimmed_entry_text (const gchar *id);
extern const gchar *get_entry_text (const gchar *id);
extern void display_error (const gchar *message, ...) G_GNUC_PRINTF (1, 2);

#endif

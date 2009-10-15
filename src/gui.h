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
#include <gconf/gconf-client.h>

#include "gibbon-player-list.h"
#include "game.h"
#include "gibbon-board-editor.h"

#define GIBBON_GCONF_PREFIX "/apps/gibbon/"
#define GIBBON_GCONF_PREFS_PREFIX "/apps/gibbon/preferences/"
#define GIBBON_GCONF_SERVER_PREFS_PREFIX "/apps/gibbon/preferences/server/"

extern GtkBuilder *builder;

extern GtkWidget *window;
extern GtkWidget *connection_dialog;
extern GConfClient *conf_client;
extern GibbonPlayerList *players;
extern GibbonBoardEditor *editor;

extern gint init_gui (const gchar *builder_filename);
extern const gchar *get_trimmed_entry_text (const gchar *id);
extern const gchar *get_entry_text (const gchar *id);
extern void display_error (const gchar *message, ...) G_GNUC_PRINTF (1, 2);
extern void set_state_connecting (void);
extern void set_state_disconnected (void);
extern void set_position (const struct GibbonPosition *pos);

G_MODULE_EXPORT void html_server_output_cb (GObject *emitter, 
                                            const gchar *html);

#endif

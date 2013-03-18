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

#ifndef _GIBBON_SETTINGS_H
# define _GIBBON_SETTINGS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#define GIBBON_SCHEMA "bg.gibbon"

#define GIBBON_PREFS_SCHEMA GIBBON_SCHEMA ".preferences"

#define GIBBON_PREFS_SERVER_SCHEMA GIBBON_PREFS_SCHEMA ".server"
#define GIBBON_PREFS_SERVER_HOST "host"
#define GIBBON_PREFS_SERVER_LOGIN "login"
#define GIBBON_PREFS_SERVER_PASSWORD "password"
#define GIBBON_PREFS_SERVER_PORT "port"
#define GIBBON_PREFS_SERVER_SAVE_PASSWORD "save-password"
#define GIBBON_PREFS_SERVER_ADDRESS "address"

#define GIBBON_PREFS_DEBUG_SCHEMA GIBBON_PREFS_SCHEMA ".debugging"
#define GIBBON_PREFS_DEBUG_TIMESTAMPS "timestamps"
#define GIBBON_PREFS_DEBUG_FIBS "server-communication"
#define GIBBON_PREFS_DEBUG_LOGFILE "logfile"

#define GIBBON_PREFS_MATCH_SCHEMA GIBBON_PREFS_SCHEMA ".match"
#define GIBBON_PREFS_MATCH_AUTO_SWAP "auto-swap"
#define GIBBON_PREFS_MATCH_LENGTH "length"
#define GIBBON_PREFS_MATCH_SHOW_EQUITY "show-equity"

#define GIBBON_DATA_SCHEMA GIBBON_SCHEMA ".data"

#define GIBBON_DATA_RECENT_SCHEMA GIBBON_DATA_SCHEMA ".recent"
#define GIBBON_DATA_COMMANDS "commands"
#define GIBBON_DATA_MAX_COMMANDS "max-commands"

G_BEGIN_DECLS

GVariant *gibbon_settings_bind_string_to_port (const GValue *value,
                                               const GVariantType *type,
                                               gpointer user_data);
gboolean gibbon_settings_bind_port_to_string (GValue *value,
                                              GVariant *variant,
                                              gpointer user_data);
GVariant *gibbon_settings_bind_trimmed_string (const GValue *value,
                                               const GVariantType *type,
                                               gpointer user_data);

guint gibbon_settings_get_uint (GSettings *settings, const gchar *key);
gboolean gibbon_settings_set_string_list (GSettings *settings, const gchar *key,
                                          GSList *list);
gboolean gibbon_settings_set_uint (GSettings *settings, const gchar *key,
                                   guint value);

G_END_DECLS

#endif

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

#ifndef _GIBBON_SERVER_CONSOLE_H
# define _GIBBON_SERVER_CONSOLE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"

#define GIBBON_TYPE_SERVER_CONSOLE \
        (gibbon_server_console_get_type ())
#define GIBBON_SERVER_CONSOLE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_SERVER_CONSOLE, \
                GibbonServerConsole))
#define GIBBON_SERVER_CONSOLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsoleClass))
#define GIBBON_IS_SERVER_CONSOLE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_SERVER_CONSOLE))
#define GIBBON_IS_SERVER_CONSOLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_SERVER_CONSOLE))
#define GIBBON_SERVER_CONSOLE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsoleClass))

/**
 * GibbonServerConsole:
 *
 * One instance of a #GibbonServerConsole.  All properties are private.
 **/
typedef struct _GibbonServerConsole GibbonServerConsole;
struct _GibbonServerConsole
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonServerConsolePrivate *priv;
};

/**
 * GibbonServerConsoleClass:
 *
 * Class representing the text output area of the FIBS server.!
 **/
typedef struct _GibbonServerConsoleClass GibbonServerConsoleClass;
struct _GibbonServerConsoleClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_server_console_get_type (void) G_GNUC_CONST;

GibbonServerConsole *gibbon_server_console_new (GibbonApp *app);

void gibbon_server_console_print_raw (GibbonServerConsole *self,
                                      const gchar *string);
void gibbon_server_console_print_login (GibbonServerConsole *self,
                                        const gchar *string);
void gibbon_server_console_print_info (GibbonServerConsole *self,
                                       const gchar *string);
void gibbon_server_console_print_output (GibbonServerConsole *self,
                                         const gchar *string);
void gibbon_server_console_print_input (GibbonServerConsole *self,
                                        const gchar *string);

#endif

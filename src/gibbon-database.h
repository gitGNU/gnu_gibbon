/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_DATABASE_H
# define _GIBBON_DATABASE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"

#define GIBBON_TYPE_DATABASE \
        (gibbon_database_get_type ())
#define GIBBON_DATABASE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_DATABASE, \
                GibbonDatabase))
#define GIBBON_DATABASE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_DATABASE, GibbonDatabaseClass))
#define GIBBON_IS_DATABASE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_DATABASE))
#define GIBBON_IS_DATABASE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_DATABASE))
#define GIBBON_DATABASE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_DATABASE, GibbonDatabaseClass))

/**
 * GibbonDatabase:
 *
 * One instance of a #GibbonDatabase.  All properties are private.
 */
typedef struct _GibbonDatabase GibbonDatabase;
struct _GibbonDatabase
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonDatabasePrivate *priv;
};

/**
 * GibbonDatabaseClass:
 *
 * Gibbon database.
 */
typedef struct _GibbonDatabaseClass GibbonDatabaseClass;
struct _GibbonDatabaseClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_database_get_type (void) G_GNUC_CONST;

GibbonDatabase *gibbon_database_new (GibbonApp *app, const gchar *path);

gint gibbon_database_update_server (GibbonDatabase *self,
                                    const gchar *hostname, guint port);
gboolean gibbon_database_update_account (GibbonDatabase *self,
                                         guint server_id, const gchar *login);
gboolean gibbon_database_update_user (GibbonDatabase *self,
                                      guint server_id, const gchar *login,
                                      gdouble rating, gint experience);

#endif

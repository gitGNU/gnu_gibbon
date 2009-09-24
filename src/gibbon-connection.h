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

#ifndef _GIBBON_CONNECTION_H
#define _GIBBON_CONNECTION_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_CONNECTION             (gibbon_connection_get_type ())
#define GIBBON_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CONNECTION, GibbonConnection))
#define GIBBON_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_CONNECTION, GibbonConnectionClass))
#define GIBBON_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_CONNECTION))
#define GIBBON_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_CONNECTION))
#define GIBBON_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_CONNECTION, GibbonConnectionClass))

typedef struct _GibbonConnectionClass   GibbonConnectionClass;
typedef struct _GibbonConnection        GibbonConnection;
typedef struct _GibbonConnectionPrivate GibbonConnectionPrivate;

struct _GibbonConnectionClass
{
        GObjectClass parent_class;
};

GType gibbon_connection_get_type (void) G_GNUC_CONST;

struct _GibbonConnection
{
        GObject parent_instance;
        GibbonConnectionPrivate *priv;
};

GibbonConnection *gibbon_connection_new (void);

const gchar *gibbon_connection_get_hostname (GibbonConnection *connection);
void gibbon_connection_set_hostname (GibbonConnection *connection, 
                                     const gchar *host);
guint gibbon_connection_get_port (GibbonConnection *connection);
void gibbon_connection_set_port (GibbonConnection *connection, guint port);
const gchar *gibbon_connection_get_password (GibbonConnection *connection);
void gibbon_connection_set_password (GibbonConnection *connection, 
                                     const gchar *password);
const gchar *gibbon_connection_get_login (GibbonConnection *connection);
void gibbon_connection_set_login (GibbonConnection *connection, 
                                  const gchar *login);

void gibbon_connection_connect (GibbonConnection *connection);
void gibbon_connection_disconnect (GibbonConnection *connection);

G_END_DECLS

#endif

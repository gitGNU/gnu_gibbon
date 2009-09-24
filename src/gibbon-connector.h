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

#ifndef _GIBBON_CONNECTOR_H
#define _GIBBON_CONNECTOR_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_CONNECTOR             (gibbon_connector_get_type ())
#define GIBBON_CONNECTOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CONNECTOR, GibbonConnector))
#define GIBBON_CONNECTOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_CONNECTOR, GibbonConnectorClass))
#define GIBBON_IS_CONNECTOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_CONNECTOR))
#define GIBBON_IS_CONNECTOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_CONNECTOR))
#define GIBBON_CONNECTOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_CONNECTOR, GibbonConnectorClass))

typedef struct _GibbonConnectorClass   GibbonConnectorClass;
typedef struct _GibbonConnector        GibbonConnector;
typedef struct _GibbonConnectorPrivate GibbonConnectorPrivate;

struct _GibbonConnectorClass
{
        GObjectClass parent_class;
};

GType gibbon_connector_get_type (void) G_GNUC_CONST;

struct _GibbonConnector
{
        GObject parent_instance;
        GibbonConnectorPrivate *priv;
};

enum GibbonConnectorState {
        GIBBON_CONNECTOR_INITIAL,
        GIBBON_CONNECTOR_RESOLVING,
        GIBBON_CONNECTOR_CONNECTING,
        GIBBON_CONNECTOR_CONNECTED,
        GIBBON_CONNECTOR_CANCELLED,
        GIBBON_CONNECTOR_ERROR
};

GibbonConnector *gibbon_connector_new (const gchar *hostname, guint port);
gboolean gibbon_connector_connect (GibbonConnector *self);
void gibbon_connector_cancel (GibbonConnector *self);
enum GibbonConnectorState gibbon_connector_get_state (GibbonConnector *self);
const gchar *gibbon_connector_error (GibbonConnector *self);
gint gibbon_connector_steal_socket (GibbonConnector *self);

G_END_DECLS

#endif

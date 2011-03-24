/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_SESSION_H
#define _GIBBON_SESSION_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_SESSION             (gibbon_session_get_type ())
#define GIBBON_SESSION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_SESSION, GibbonSession))
#define GIBBON_SESSION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_SESSION, GibbonSessionClass))
#define GIBBON_IS_SESSION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_SESSION))
#define GIBBON_IS_SESSION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_SESSION))
#define GIBBON_SESSION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_SESSION, GibbonSessionClass))

typedef struct _GibbonSessionClass   GibbonSessionClass;
typedef struct _GibbonSession        GibbonSession;
typedef struct _GibbonSessionPrivate GibbonSessionPrivate;

struct _GibbonSessionClass
{
        GObjectClass parent_class;
};

GType gibbon_session_get_type (void) G_GNUC_CONST;

struct _GibbonSession
{
        GObject parent_instance;
        GibbonSessionPrivate *priv;
};

GibbonSession *gibbon_session_new (GibbonApp *app,
                                   GibbonConnection *connection);
gint gibbon_session_process_server_line (GibbonSession *self,
                                         const gchar *line);
G_END_DECLS

#endif

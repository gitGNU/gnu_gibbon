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

#ifndef _GIBBON_NETWORKER_H
#define _GIBBON_NETWORKER_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_NETWORKER             (gibbon_networker_get_type ())
#define GIBBON_NETWORKER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_NETWORKER, GibbonNetworker))
#define GIBBON_NETWORKER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_NETWORKER, GibbonNetworkerClass))
#define GIBBON_IS_NETWORKER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_NETWORKER))
#define GIBBON_IS_NETWORKER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_NETWORKER))
#define GIBBON_NETWORKER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_NETWORKER, GibbonNetworkerClass))

typedef struct _GibbonNetworkerClass   GibbonNetworkerClass;
typedef struct _GibbonNetworker        GibbonNetworker;
typedef struct _GibbonNetworkerPrivate GibbonNetworkerPrivate;

struct _GibbonNetworkerClass
{
        GObjectClass parent_class;
};

GType gibbon_networker_get_type (void) G_GNUC_CONST;

struct _GibbonNetworker
{
        GObject parent_instance;
        GibbonNetworkerPrivate *priv;
};

GibbonNetworker *gibbon_networker_new (const gchar *hostname);

const gchar *gibbon_networker_get_hostname (GibbonNetworker *self);

G_END_DECLS

#endif

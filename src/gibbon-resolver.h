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

#ifndef _GIBBON_RESOLVER_H
#define _GIBBON_RESOLVER_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_RESOLVER             (gibbon_resolver_get_type ())
#define GIBBON_RESOLVER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_RESOLVER, GibbonResolver))
#define GIBBON_RESOLVER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_RESOLVER, GibbonResolverClass))
#define GIBBON_IS_RESOLVER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_RESOLVER))
#define GIBBON_IS_RESOLVER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_RESOLVER))
#define GIBBON_RESOLVER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_RESOLVER, GibbonResolverClass))

typedef struct _GibbonResolverClass   GibbonResolverClass;
typedef struct _GibbonResolver        GibbonResolver;
typedef struct _GibbonResolverPrivate GibbonResolverPrivate;

struct _GibbonResolverClass
{
        GObjectClass parent_class;
};

GType gibbon_resolver_get_type (void) G_GNUC_CONST;

struct _GibbonResolver
{
        GObject parent_instance;
        GibbonResolverPrivate *priv;
};

GibbonResolver *gibbon_resolver_new (const gchar *hostname);
gint gibbon_resolver_resolve (GibbonResolver *self);

const gchar *gibbon_resolver_get_hostname (GibbonResolver *self);

G_END_DECLS

#endif

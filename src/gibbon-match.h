/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_MATCH_H
#define _GIBBON_MATCH_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_MATCH             (gibbon_match_get_type ())
#define GIBBON_MATCH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH, GibbonMatch))
#define GIBBON_MATCH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_MATCH, GibbonMatchClass))
#define GIBBON_IS_MATCH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_MATCH))
#define GIBBON_IS_MATCH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_MATCH))
#define GIBBON_MATCH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_MATCH, GibbonMatchClass))

typedef struct _GibbonMatchClass   GibbonMatchClass;
typedef struct _GibbonMatch        GibbonMatch;
typedef struct _GibbonMatchPrivate GibbonMatchPrivate;

struct _GibbonMatchClass
{
        GObjectClass parent_class;
};

GType gibbon_match_get_type (void) G_GNUC_CONST;

struct _GibbonMatch
{
        GObject parent_instance;
        GibbonMatchPrivate *priv;
};

GibbonMatch *gibbon_match_new ();

G_END_DECLS

#endif

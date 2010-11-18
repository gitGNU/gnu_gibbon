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

#include <gtk/gtk.h>

#include "gibbon-match.h"

struct _GibbonMatchPrivate {
        guint move_number;
};

#define GIBBON_MATCH_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_MATCH,           \
                                      GibbonMatchPrivate))
G_DEFINE_TYPE (GibbonMatch, gibbon_match, G_TYPE_OBJECT);

static void
gibbon_match_init (GibbonMatch *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GIBBON_TYPE_MATCH,
                                                  GibbonMatchPrivate);
        
        /* self->priv->... = NULL */
}

static void
gibbon_match_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_parent_class)->finalize (object);
}

static void
gibbon_match_class_init (GibbonMatchClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonMatchPrivate));

        object_class->finalize = gibbon_match_finalize;
}

GibbonMatch *
gibbon_match_new ()
{
         GibbonMatch *self = g_object_new (GIBBON_TYPE_MATCH, NULL);

         self->priv->move_number = 0;
         
         return self;
}

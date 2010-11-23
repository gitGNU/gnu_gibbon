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

#include <gtk/gtk.h>

#include "gibbon-game.h"

struct _GibbonGamePrivate {
        guint move_number;
};

#define GIBBON_GAME_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_GAME,           \
                                      GibbonGamePrivate))
G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT);

static void
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GIBBON_TYPE_GAME,
                                                  GibbonGamePrivate);
        
        /* self->priv->... = NULL */
}

static void
gibbon_game_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_game_parent_class)->finalize (object);
}

static void
gibbon_game_class_init (GibbonGameClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonGamePrivate));

        object_class->finalize = gibbon_game_finalize;
}

GibbonGame *
gibbon_game_new ()
{
         GibbonGame *self = g_object_new (GIBBON_TYPE_GAME, NULL);

         self->priv->move_number = 0;
         
         return self;
}

/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

/**
 * SECTION:gibbon-move
 * @short_description: A backgammon move in FIBS
 *
 * Since: 0.1.1
 *
 * A complete backgammon move.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-move.h"
#include "gibbon-movement.h"

typedef struct _GibbonMovePrivate GibbonMovePrivate;
struct _GibbonMovePrivate {
        GSList *movements;
};

#define GIBBON_MOVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MOVE, GibbonMovePrivate))

G_DEFINE_TYPE (GibbonMove, gibbon_move, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_move_init (GibbonMove *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MOVE, GibbonMovePrivate);

        self->priv->movements = NULL;
}

static void
gibbon_move_finalize (GObject *object)
{
        GibbonMove *self = GIBBON_MOVE (object);

        if (self->priv->movements) {
                /* FIXME! Free movements.  */
        }

        G_OBJECT_CLASS (gibbon_move_parent_class)->finalize(object);
}

static void
gibbon_move_class_init (GibbonMoveClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonMovePrivate));

        object_class->finalize = gibbon_move_finalize;
}

GibbonMove *
gibbon_move_new (guint die1, guint die2, ...)
{
        GibbonMove *self = g_object_new (GIBBON_TYPE_MOVE, NULL);

        return self;
}

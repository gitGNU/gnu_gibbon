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

/**
 * SECTION:gsgf-move-backgammon
 * @short_description: Definitions for a move in Backgammon
 *
 * Representation of one single move in Backgammon
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

G_DEFINE_TYPE(GSGFMoveBackgammon, gsgf_move_backgammon, GSGF_TYPE_MOVE)

static void
gsgf_move_backgammon_init(GSGFMoveBackgammon *self)
{
}

static void
gsgf_move_backgammon_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_move_backgammon_parent_class)->finalize(object);
}

static void
gsgf_move_backgammon_class_init(GSGFMoveBackgammonClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gsgf_move_backgammon_finalize;
}

/**
 * gsgf_move_backgammon_new:
 *
 * Creates a new #GSGFMoveBackgammon.
 *
 * Returns: The new #GSGFMoveBackgammon.
 */
GSGFMoveBackgammon *
gsgf_move_backgammon_new (void)
{
        GSGFMoveBackgammon *self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        return self;
}


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

#ifndef _GIBBON_GAME_H
# define _GIBBON_GAME_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include <libgsgf/gsgf.h>
#include "gibbon-match.h"

#define GIBBON_TYPE_GAME \
        (gibbon_game_get_type ())
#define GIBBON_GAME(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GAME, \
                GibbonGame))
#define GIBBON_GAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GAME, GibbonGameClass))
#define GIBBON_IS_GAME(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GAME))
#define GIBBON_IS_GAME_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GAME))
#define GIBBON_GAME_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GAME, GibbonGameClass))

/**
 * GibbonGame:
 *
 * One instance of a #GibbonGame.  All properties are private.
 **/
typedef struct _GibbonGame GibbonGame;
struct _GibbonGame
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonGamePrivate *priv;
};

/**
 * GibbonGameClass:
 *
 * Class representing one game in a backgammon match!
 **/
typedef struct _GibbonGameClass GibbonGameClass;
struct _GibbonGameClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_game_get_type (void) G_GNUC_CONST;

GibbonGame *gibbon_game_new (GibbonMatch *match, GSGFGameTree *game_tree);

#endif

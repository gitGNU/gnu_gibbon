/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_MATCH_LIST_H
# define _GIBBON_MATCH_LIST_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#include "gibbon-match.h"
#include "gibbon-game.h"

#define GIBBON_TYPE_MATCH_LIST \
        (gibbon_match_list_get_type ())
#define GIBBON_MATCH_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH_LIST, \
                GibbonMatchList))
#define GIBBON_MATCH_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH_LIST, GibbonMatchListClass))
#define GIBBON_IS_MATCH_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH_LIST))
#define GIBBON_IS_MATCH_LIST_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH_LIST))
#define GIBBON_MATCH_LIST_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH_LIST, GibbonMatchListClass))

enum {
        GIBBON_MATCH_LIST_COL_SIDE = 0,
        GIBBON_MATCH_LIST_COL_PLAYER,
        GIBBON_MATCH_LIST_COL_MOVENO,
        GIBBON_MATCH_LIST_COL_ROLL,
        GIBBON_MATCH_LIST_COL_ROLL_ACTION,
        GIBBON_MATCH_LIST_COL_LUCK,
        GIBBON_MATCH_LIST_COL_LUCK_TYPE,
        GIBBON_MATCH_LIST_COL_MOVE,
        GIBBON_MATCH_LIST_COL_MOVE_ACTION,
        GIBBON_MATCH_LIST_COL_MOVE_BADNESS,
        GIBBON_MATCH_LIST_N_COLUMNS
};

/**
 * GibbonMatchList:
 *
 * One instance of a #GibbonMatchList.  All properties are private.
 */
typedef struct _GibbonMatchList GibbonMatchList;
struct _GibbonMatchList
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchListPrivate *priv;
};

/**
 * GibbonMatchListClass:
 *
 * Class representing a match listing.
 */
typedef struct _GibbonMatchListClass GibbonMatchListClass;
struct _GibbonMatchListClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_match_list_get_type (void) G_GNUC_CONST;

GibbonMatchList *gibbon_match_list_new (void);
void gibbon_match_list_on_new_match (GibbonMatchList *self,
                                     const GibbonMatch *match);

GtkListStore *gibbon_match_list_get_games_store (const GibbonMatchList *self);
GtkListStore *gibbon_match_list_get_moves_store (const GibbonMatchList *self);

void gibbon_match_list_set_active_game (GibbonMatchList *self, gint active);
gint gibbon_match_list_get_active_game (const GibbonMatchList *self);

void gibbon_match_list_add_game (GibbonMatchList *self, const GibbonGame *game);
gboolean gibbon_match_list_add_action (GibbonMatchList *self,
                                       const GibbonGame *game,
                                       gint action_no);

#endif

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

#ifndef _GIBBON_GAME_LIST_VIEW_H
# define _GIBBON_GAME_LIST_VIEW_H

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match-list.h"

#define GIBBON_TYPE_GAME_LIST_VIEW \
        (gibbon_game_list_view_get_type ())
#define GIBBON_GAME_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GAME_LIST_VIEW, \
                GibbonGameListView))
#define GIBBON_GAME_LIST_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GAME_LIST_VIEW, GibbonGameListViewClass))
#define GIBBON_IS_GAME_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GAME_LIST_VIEW))
#define GIBBON_IS_GAME_LIST_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GAME_LIST_VIEW))
#define GIBBON_GAME_LIST_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GAME_LIST_VIEW, GibbonGameListViewClass))

/**
 * GibbonGameListView:
 *
 * One instance of a #GibbonGameListView.  All properties are private.
 **/
typedef struct _GibbonGameListView GibbonGameListView;
struct _GibbonGameListView
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonGameListViewPrivate *priv;
};

/**
 * GibbonGameListViewClass:
 *
 * Visual representation of the game list!
 **/
typedef struct _GibbonGameListViewClass GibbonGameListViewClass;
struct _GibbonGameListViewClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_game_list_view_get_type (void) G_GNUC_CONST;

GibbonGameListView *gibbon_game_list_view_new (GtkComboBox *combo,
                                               GibbonMatchList *list);

#endif

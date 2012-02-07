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

#ifndef _GIBBON_PLAYER_LIST_VIEW_H
# define _GIBBON_PLAYER_LIST_VIEW_H

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"
#include "gibbon-player-list.h"

#define GIBBON_TYPE_PLAYER_LIST_VIEW \
        (gibbon_player_list_view_get_type ())
#define GIBBON_PLAYER_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_PLAYER_LIST_VIEW, \
                GibbonPlayerListView))
#define GIBBON_PLAYER_LIST_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_PLAYER_LIST_VIEW, GibbonPlayerListViewClass))
#define GIBBON_IS_PLAYER_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_PLAYER_LIST_VIEW))
#define GIBBON_IS_PLAYER_LIST_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_PLAYER_LIST_VIEW))
#define GIBBON_PLAYER_LIST_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_PLAYER_LIST_VIEW, GibbonPlayerListViewClass))

/**
 * GibbonPlayerListView:
 *
 * One instance of a #GibbonPlayerListView.  All properties are private.
 **/
typedef struct _GibbonPlayerListView GibbonPlayerListView;
struct _GibbonPlayerListView
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonPlayerListViewPrivate *priv;
};

/**
 * GibbonPlayerListViewClass:
 *
 * Visual representation of the player list!
 **/
typedef struct _GibbonPlayerListViewClass GibbonPlayerListViewClass;
struct _GibbonPlayerListViewClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_player_list_view_get_type (void) G_GNUC_CONST;

GibbonPlayerListView *gibbon_player_list_view_new (GibbonApp *app,
                                                   GibbonPlayerList *players);

#endif

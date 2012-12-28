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

#ifndef _GIBBON_MOVE_LIST_VIEW_H
# define _GIBBON_MOVE_LIST_VIEW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#include "gibbon-match-list.h"

#define GIBBON_TYPE_MOVE_LIST_VIEW \
        (gibbon_move_list_view_get_type ())
#define GIBBON_MOVE_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MOVE_LIST_VIEW, \
                GibbonMoveListView))
#define GIBBON_MOVE_LIST_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewClass))
#define GIBBON_IS_MOVE_LIST_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MOVE_LIST_VIEW))
#define GIBBON_IS_MOVE_LIST_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MOVE_LIST_VIEW))
#define GIBBON_MOVE_LIST_VIEW_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MOVE_LIST_VIEW, GibbonMoveListViewClass))

/**
 * GibbonMoveListView:
 *
 * One instance of a #GibbonMoveListView.  All properties are private.
 */
typedef struct _GibbonMoveListView GibbonMoveListView;
struct _GibbonMoveListView
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMoveListViewPrivate *priv;
};

/**
 * GibbonMoveListViewClass:
 *
 * Class for #GibbonMoveListView.
 */
typedef struct _GibbonMoveListViewClass GibbonMoveListViewClass;
struct _GibbonMoveListViewClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_move_list_view_get_type (void) G_GNUC_CONST;

GibbonMoveListView *gibbon_move_list_view_new (GtkTreeView *view,
                                               const GibbonMatchList *list);
void gibbon_move_list_view_on_new_match (GibbonMoveListView *self,
                                         GibbonMatch *match);

#endif

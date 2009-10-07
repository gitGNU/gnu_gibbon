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

#ifndef _GIBBON_PLAYER_LIST_H
#define _GIBBON_PLAYER_LIST_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_PLAYER_LIST             (gibbon_player_list_get_type ())
#define GIBBON_PLAYER_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_PLAYER_LIST, GibbonPlayerList))
#define GIBBON_PLAYER_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_PLAYER_LIST, GibbonPlayerListClass))
#define GIBBON_IS_PLAYER_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_PLAYER_LIST))
#define GIBBON_IS_PLAYER_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_PLAYER_LIST))
#define GIBBON_PLAYER_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_PLAYER_LIST, GibbonPlayerListClass))

typedef struct _GibbonPlayerListClass   GibbonPlayerListClass;
typedef struct _GibbonPlayerList        GibbonPlayerList;
typedef struct _GibbonPlayerListPrivate GibbonPlayerListPrivate;

struct _GibbonPlayerListClass
{
        GObjectClass parent_class;
};

GType gibbon_player_list_get_type (void) G_GNUC_CONST;

struct _GibbonPlayerList
{
        GObject parent_instance;
        GibbonPlayerListPrivate *priv;
};

enum {
        GIBBON_PLAYER_LIST_COL_NAME = 0,
        GIBBON_PLAYER_LIST_COL_AVAILABLE,
        GIBBON_PLAYER_LIST_COL_RATING,
        GIBBON_PLAYER_LIST_COL_EXPERIENCE,
        GIBBON_PLAYER_LIST_COL_OPPONENT,
        GIBBON_PLAYER_LIST_COL_WATCHING,
        GIBBON_PLAYER_LIST_N_COLUMNS,
};

GibbonPlayerList *gibbon_player_list_new (void);
void gibbon_player_list_connect_view (GibbonPlayerList *self, 
                                      GtkTreeView *view);

void gibbon_player_list_clear (GibbonPlayerList *self);
void gibbon_player_list_set (GibbonPlayerList *self, 
                             const gchar *player_name,
                             gboolean available,
                             gdouble rating,
                             guint experience,
                             gchar *opponent,
                             gchar *watching);

G_END_DECLS

#endif

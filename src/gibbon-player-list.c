/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-player-list.h"

struct _GibbonPlayerListPrivate {
        GHashTable *hash;
        GtkListStore *store;
        GtkTreeModel *model;
};

struct GibbonPlayer {
        GtkTreeIter iter;
        
        guint experience;
        gdouble rating;
};

static GType gibbon_player_list_column_types[GIBBON_PLAYER_LIST_N_COLUMNS];

#define GIBBON_PLAYER_LIST_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  GIBBON_TYPE_PLAYER_LIST,           \
                                  GibbonPlayerListPrivate))
                                             
G_DEFINE_TYPE (GibbonPlayerList, gibbon_player_list, G_TYPE_OBJECT);

static void free_player_name (gpointer name);
static void free_player (gpointer player);
/* FIXME! Should go into a utility module!  */
static gint compare_utf8_string (GtkTreeModel *model,
                                 GtkTreeIter *a,
                                 GtkTreeIter *b,
                                 gpointer user_data);


static void
gibbon_player_list_init (GibbonPlayerList *self)
{
        GtkListStore *store;
        GtkTreeModel *model;
        
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_PLAYER_LIST, 
                                                  GibbonPlayerListPrivate);

        self->priv->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  free_player_name,
                                                  free_player);

        store = gtk_list_store_new (GIBBON_PLAYER_LIST_N_COLUMNS, 
                                    G_TYPE_STRING,
                                    G_TYPE_BOOLEAN,
                                    G_TYPE_DOUBLE, 
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING);
        self->priv->store = store;
        
        model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (store));
        self->priv->model = model;
        
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store), 
                GIBBON_PLAYER_LIST_COL_NAME,
                compare_utf8_string, 
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_NAME), 
                NULL);
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                              GIBBON_PLAYER_LIST_COL_NAME, 
                                              GTK_SORT_ASCENDING);
}

static void
gibbon_player_list_finalize (GObject *object)
{
        GibbonPlayerList *self = GIBBON_PLAYER_LIST (object);

        if (self->priv->hash)
                g_hash_table_destroy (self->priv->hash);
        self->priv->hash = NULL;
        
        if (self->priv->model)
                g_object_unref (self->priv->model);
        self->priv->model = NULL;
        self->priv->store = NULL;

        G_OBJECT_CLASS (gibbon_player_list_parent_class)->finalize (object);
}

static void
gibbon_player_list_class_init (GibbonPlayerListClass *klass)
{
        GObjectClass* parent_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonPlayerListPrivate));

        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_NAME] = 
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_RATING] = 
                G_TYPE_DOUBLE;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_EXPERIENCE] = 
                G_TYPE_UINT;
                
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_player_list_finalize;
}

GibbonPlayerList *
gibbon_player_list_new ()
{
        GibbonPlayerList *self = g_object_new (GIBBON_TYPE_PLAYER_LIST, NULL);

        return self;
}

static void
free_player_name (gpointer name)
{
        if (name)
                g_free (name);
}

static void
free_player (gpointer _player)
{
        struct GibbonPlayer *player = (struct GibbonPlayer *) _player;
        
        if (player) {
                g_free (_player);
        }
}

void
gibbon_player_list_set (GibbonPlayerList *self, 
                        const gchar *name,
                        gboolean available,
                        gdouble rating,
                        guint experience,
                        gchar *opponent,
                        gchar *watching)
{
        struct GibbonPlayer *player;
                        
        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (name);
        
        player = g_hash_table_lookup (self->priv->hash, name);
        if (!player) {
                player = g_malloc0 (sizeof *player);
                g_hash_table_insert (self->priv->hash, g_strdup (name), player);
                gtk_list_store_append (self->priv->store, 
                                       &player->iter);
        }

        player->rating = rating;
        player->experience = experience;
        
        gtk_list_store_set (self->priv->store,
                            &player->iter,
                            GIBBON_PLAYER_LIST_COL_NAME, name,
                            GIBBON_PLAYER_LIST_COL_AVAILABLE, available,
                            GIBBON_PLAYER_LIST_COL_RATING, rating,
                            GIBBON_PLAYER_LIST_COL_EXPERIENCE, experience,
                            GIBBON_PLAYER_LIST_COL_OPPONENT, opponent,
                            GIBBON_PLAYER_LIST_COL_WATCHING, watching,
                            -1);
}

void
gibbon_player_list_connect_view (GibbonPlayerList *self, GtkTreeView *view)
{
        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        gtk_tree_view_set_model (view, self->priv->model);
}

void
gibbon_player_list_clear (GibbonPlayerList *self)
{
        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        
        g_hash_table_remove_all (self->priv->hash);
        gtk_list_store_clear (self->priv->store);
}

static gint 
compare_utf8_string (GtkTreeModel *model,
                     GtkTreeIter *a, GtkTreeIter *b,
                     gpointer user_data)
{
        gchar *str_a = NULL;
        gchar *str_b = NULL;
        gchar *key_a;
        gchar *key_b;
        
        gint result;
        
        gint col = GPOINTER_TO_INT(user_data);

        gtk_tree_model_get (model, a, col, &str_a, -1);
        key_a = g_utf8_collate_key (str_a, -1);
        
        gtk_tree_model_get (model, b, col, &str_b, -1);
        key_b = g_utf8_collate_key (str_b, -1);

        result = strcmp (key_a, key_b);
        
        g_free (str_a);
        g_free (str_b);
        
        return result;
}

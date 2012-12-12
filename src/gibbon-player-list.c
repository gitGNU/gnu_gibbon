/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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
#include "gibbon-reliability.h"

struct _GibbonPlayerListPrivate {
        GHashTable *hash;
        GtkListStore *store;
};

struct GibbonPlayer {
        GtkTreeIter iter;
        
        guint experience;
        gdouble rating;
        gboolean use_backslash_u;
};

static GType gibbon_player_list_column_types[GIBBON_PLAYER_LIST_N_COLUMNS];

#define GIBBON_PLAYER_LIST_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                       GIBBON_TYPE_PLAYER_LIST,           \
                                       GibbonPlayerListPrivate))

static gint gibbon_player_list_compare_status (GtkTreeModel *model,
                                               GtkTreeIter *a,
                                               GtkTreeIter *b,
                                               gpointer user_data);
static gint gibbon_player_list_compare_country (GtkTreeModel *model,
                                                GtkTreeIter *a,
                                                GtkTreeIter *b,
                                                gpointer user_data);
static gint gibbon_player_list_compare_reliability (GtkTreeModel *model,
                                                    GtkTreeIter *a,
                                                    GtkTreeIter *b,
                                                    gpointer user_data);

G_DEFINE_TYPE (GibbonPlayerList, gibbon_player_list, G_TYPE_OBJECT);

static void
gibbon_player_list_init (GibbonPlayerList *self)
{
        GtkListStore *store;
        
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_PLAYER_LIST, 
                                                  GibbonPlayerListPrivate);

        self->priv->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free,
                                                  g_free);

        store = gtk_list_store_new (GIBBON_PLAYER_LIST_N_COLUMNS, 
                                    G_TYPE_STRING,
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    G_TYPE_DOUBLE, 
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    GDK_TYPE_PIXBUF,
                                    GIBBON_TYPE_RELIABILITY,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    GIBBON_TYPE_COUNTRY,
                                    GDK_TYPE_PIXBUF,
                                    G_TYPE_STRING);
        self->priv->store = store;
        
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store), 
                GIBBON_PLAYER_LIST_COL_NAME,
                gibbon_compare_string_column,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_NAME), 
                NULL);
        /*
         * Initially sort by name.  FIXME! We should save the last sorting
         * in the user preferences.
         */
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
                                              GIBBON_PLAYER_LIST_COL_NAME, 
                                              GTK_SORT_ASCENDING);

        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_AVAILABLE,
                gibbon_player_list_compare_status,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_AVAILABLE),
                NULL);
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_COUNTRY,
                gibbon_player_list_compare_country,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_COUNTRY),
                NULL);
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_RATING,
                gibbon_compare_double_column,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_RATING),
                NULL);
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                gibbon_compare_uint_column,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_EXPERIENCE),
                NULL);
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_CLIENT,
                gibbon_compare_string_column,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_CLIENT),
                NULL);
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store),
                GIBBON_PLAYER_LIST_COL_RELIABILITY,
                gibbon_player_list_compare_reliability,
                GINT_TO_POINTER (GIBBON_PLAYER_LIST_COL_RELIABILITY),
                NULL);
}

static void
gibbon_player_list_finalize (GObject *object)
{
        GibbonPlayerList *self = GIBBON_PLAYER_LIST (object);

        if (self->priv->hash)
                g_hash_table_destroy (self->priv->hash);
        
        if (self->priv->store)
                g_object_unref (self->priv->store);

        G_OBJECT_CLASS (gibbon_player_list_parent_class)->finalize (object);
}

static void
gibbon_player_list_class_init (GibbonPlayerListClass *klass)
{
        GObjectClass* parent_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonPlayerListPrivate));

        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_NAME] = 
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_NAME_WEIGHT] =
                G_TYPE_INT;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_AVAILABLE] =
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_RATING] = 
                G_TYPE_DOUBLE;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_EXPERIENCE] = 
                G_TYPE_UINT;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_CLIENT] =
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_CLIENT_ICON] =
                GDK_TYPE_PIXBUF;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_RELIABILITY] =
                GIBBON_TYPE_RELIABILITY;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_OPPONENT] =
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_WATCHING] =
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_HOSTNAME] =
                G_TYPE_STRING;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_COUNTRY] =
                GIBBON_TYPE_COUNTRY;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_COUNTRY_ICON] =
                GDK_TYPE_PIXBUF;
        gibbon_player_list_column_types[GIBBON_PLAYER_LIST_COL_EMAIL] =
                G_TYPE_STRING;
                
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_player_list_finalize;
}

GibbonPlayerList *
gibbon_player_list_new ()
{
        GibbonPlayerList *self = g_object_new (GIBBON_TYPE_PLAYER_LIST, NULL);

        return self;
}

void
gibbon_player_list_set (GibbonPlayerList *self, 
                        const gchar *name,
                        gboolean has_saved,
                        gboolean available,
                        gdouble rating,
                        guint experience,
                        gdouble reliability,
                        guint confidence,
                        const gchar *opponent,
                        const gchar *watching,
                        const gchar *client,
                        const GdkPixbuf *client_icon,
                        const gchar *hostname,
                        const GibbonCountry *country,
                        const gchar *email)
{
        struct GibbonPlayer *player;
        const gchar *version_string = NULL;
        const gchar *stock_id;
        GibbonReliability rel;
        const GdkPixbuf *country_icon;
        gint name_weight = has_saved ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (name);
        
        rel.value = reliability;
        rel.confidence = confidence;

        player = g_hash_table_lookup (self->priv->hash, name);
        if (!player) {
                player = g_malloc0 (sizeof *player);
                g_hash_table_insert (self->priv->hash, g_strdup (name), player);
                gtk_list_store_append (self->priv->store, 
                                       &player->iter);
        }

        player->rating = rating;
        player->experience = experience;
        player->use_backslash_u = FALSE;

        if (client) {
                if (strncmp ("BGOnline v", client, 10) == 0)
                        version_string = client + 10;
                else if (strncmp ("Padgammon v", client, 11) == 0)
                        version_string = client + 11;

                if (version_string) {
                        if ((version_string[0] == '1'
                             && version_string[1] == '.')
                            || (version_string[0] == '2'
                                && version_string[1] == '.'
                                && version_string[2] == '0'))
                                player->use_backslash_u = TRUE;
                }
        }

        if (available) {
                stock_id = GTK_STOCK_YES;
        } else {
                if (opponent && *opponent)
                        stock_id = GTK_STOCK_NO;
                else
                        stock_id = GTK_STOCK_STOP;
        }

        country_icon = gibbon_country_get_pixbuf (country);
        gtk_list_store_set (self->priv->store,
                            &player->iter,
                            GIBBON_PLAYER_LIST_COL_NAME, name,
                            GIBBON_PLAYER_LIST_COL_NAME_WEIGHT, name_weight,
                            GIBBON_PLAYER_LIST_COL_AVAILABLE, stock_id,
                            GIBBON_PLAYER_LIST_COL_RATING, rating,
                            GIBBON_PLAYER_LIST_COL_EXPERIENCE, experience,
                            GIBBON_PLAYER_LIST_COL_RELIABILITY, &rel,
                            GIBBON_PLAYER_LIST_COL_OPPONENT, opponent,
                            GIBBON_PLAYER_LIST_COL_WATCHING, watching,
                            GIBBON_PLAYER_LIST_COL_CLIENT, client,
                            GIBBON_PLAYER_LIST_COL_CLIENT_ICON, client_icon,
                            GIBBON_PLAYER_LIST_COL_HOSTNAME, hostname,
                            GIBBON_PLAYER_LIST_COL_COUNTRY, country,
                            GIBBON_PLAYER_LIST_COL_COUNTRY_ICON, country_icon,
                            GIBBON_PLAYER_LIST_COL_EMAIL, email,
                            -1);
}

void
gibbon_player_list_connect_view (GibbonPlayerList *self, GtkTreeView *view)
{
        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        gtk_tree_view_set_model (view, GTK_TREE_MODEL (self->priv->store));
}

void
gibbon_player_list_clear (GibbonPlayerList *self)
{
        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        
        g_hash_table_remove_all (self->priv->hash);
        gtk_list_store_clear (self->priv->store);
}

gboolean
gibbon_player_list_exists (const GibbonPlayerList *self, const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST (self), FALSE);

        return g_hash_table_lookup (self->priv->hash, name) ? FALSE : TRUE;
}

void
gibbon_player_list_remove (GibbonPlayerList *self,
                           const gchar *name)
{
        struct GibbonPlayer *player = g_hash_table_lookup (self->priv->hash,
                                                           name);
        GtkTreeIter iter;

        if (!player)
                return;

        iter = player->iter;

        gtk_list_store_remove (self->priv->store, &iter);

        (void) g_hash_table_remove (self->priv->hash, name);
}

gchar *
gibbon_player_list_get_opponent (const GibbonPlayerList *self,
                                 const gchar *name)
{
        struct GibbonPlayer *player = g_hash_table_lookup (self->priv->hash,
                                                           name);
        GtkTreeIter iter;
        gchar *opponent;

        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST (self), NULL);

        if (!player)
                return NULL;

        iter = player->iter;

        gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                            GIBBON_PLAYER_LIST_COL_OPPONENT, &opponent,
                            -1);

        if (!*opponent) {
                g_free (opponent);
                return NULL;
        }
        return opponent;
}

gboolean
gibbon_player_list_get_available (const GibbonPlayerList *self,
                                  const gchar *name)
{
        struct GibbonPlayer *player;
        GtkTreeIter iter;
        gchar *status;
        gboolean available;

        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST (self), FALSE);

        player = g_hash_table_lookup (self->priv->hash, name);
        if (!player)
                return FALSE;

        iter = player->iter;

        gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                            GIBBON_PLAYER_LIST_COL_AVAILABLE, &status,
                            -1);

        if (g_strcmp0 ("gtk-yes", status))
                available = FALSE;
        else
                available = TRUE;

        return available;
}

GtkListStore *
gibbon_player_list_get_store (GibbonPlayerList *self)
{
        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST (self), NULL);

        return self->priv->store;
}

gboolean
gibbon_player_list_get_iter (GibbonPlayerList *self, const gchar *name,
                             GtkTreeIter *iter)
{
        struct GibbonPlayer *player;

        g_return_val_if_fail (GIBBON_IS_PLAYER_LIST (self), FALSE);

        player = g_hash_table_lookup (self->priv->hash, name);
        if (!player)
                return FALSE;

        *iter = player->iter;

        return TRUE;
}

void
gibbon_player_list_update_country (GibbonPlayerList *self,
                                   const gchar *hostname,
                                   const GibbonCountry *country)
{
        GtkTreeIter iter;
        gboolean valid;
        gchar *stored_hostname;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (GIBBON_IS_COUNTRY (country));

        valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->priv->store),
                                               &iter);
        while (valid) {
                gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                                    GIBBON_PLAYER_LIST_COL_HOSTNAME,
                                    &stored_hostname,
                                    -1);

                if (0 == g_strcmp0 (hostname, stored_hostname)) {
                        gtk_list_store_set (self->priv->store,
                                            &iter,
                                            GIBBON_PLAYER_LIST_COL_COUNTRY,
                                            country,
                                            GIBBON_PLAYER_LIST_COL_COUNTRY_ICON,
                                            gibbon_country_get_pixbuf (country),
                                            -1);
                }
                valid = gtk_tree_model_iter_next (
                                GTK_TREE_MODEL (self->priv->store),
                                &iter);
        }
}

void
gibbon_player_list_update_has_saved (GibbonPlayerList *self, const gchar *who,
                                     gboolean has_saved)
{
        GtkTreeIter iter;
        gint weight;

        g_return_if_fail (GIBBON_IS_PLAYER_LIST (self));
        g_return_if_fail (who != NULL);

        /*
         * Silently fail, if player is not known.
         */
        if (!gibbon_player_list_get_iter (self, who, &iter))
                return;

        weight = has_saved ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
        gtk_list_store_set (self->priv->store, &iter,
                            GIBBON_PLAYER_LIST_COL_NAME_WEIGHT, weight,
                           -1);
}

gint
gibbon_player_list_compare_status (GtkTreeModel *model,
                                   GtkTreeIter *a, GtkTreeIter *b,
                                   gpointer user_data)
{
        gchar *str_a = NULL;
        gchar *str_b = NULL;
        guint key_a, key_b;

        gint result;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &str_a, -1);
        if (!g_strcmp0 (str_a, GTK_STOCK_YES))
                key_a = 2;
        else if (!g_strcmp0 (str_a, GTK_STOCK_NO))
                key_a = 1;
        else
                key_a = 0;

        gtk_tree_model_get (model, b, col, &str_b, -1);
        if (!g_strcmp0 (str_b, GTK_STOCK_YES))
                key_b = 2;
        else if (!g_strcmp0 (str_b, GTK_STOCK_NO))
                key_b = 1;
        else
                key_b = 0;

        result = key_a - key_b;

        g_free (str_a);
        g_free (str_b);

        return result;
}

gint
gibbon_player_list_compare_country (GtkTreeModel *model,
                                   GtkTreeIter *a, GtkTreeIter *b,
                                   gpointer user_data)
{
        GibbonCountry *country_a = NULL;
        GibbonCountry *country_b = NULL;
        const gchar *str_a;
        const gchar *str_b;
        gchar *key_a;
        gchar *key_b;

        gint result;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &country_a, -1);
        str_a = gibbon_country_get_name (country_a);
        key_a = g_utf8_collate_key (str_a, -1);

        gtk_tree_model_get (model, b, col, &country_b, -1);
        str_b = gibbon_country_get_name (country_b);
        key_b = g_utf8_collate_key (str_b, -1);

        result = g_strcmp0 (key_a, key_b);

        g_object_unref (country_a);
        g_object_unref (country_b);
        g_free (key_a);
        g_free (key_b);

        return result;
}

gint
gibbon_player_list_compare_reliability (GtkTreeModel *model,
                                        GtkTreeIter *a, GtkTreeIter *b,
                                        gpointer user_data)
{
        GibbonReliability *rel_a = NULL;
        GibbonReliability *rel_b = NULL;
        gdouble value_a, value_b;
        guint grouping_a, grouping_b;
        guint confidence_a, confidence_b;
        gint factor_a = -1;
        gint factor_b = -1;

        gint result;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &rel_a, -1);
        value_a = rel_a->value;
        if (value_a >= 0.95) {
                grouping_a = 3;
                factor_a = +1;
        } else if (value_a >= 0.85)
                grouping_a = 2;
        else if (value_a >= 0.65)
                grouping_a = 1;
        else
                grouping_a = 0;
        confidence_a = factor_a * rel_a->confidence;

        gtk_tree_model_get (model, b, col, &rel_b, -1);
        value_b = rel_b->value;
        if (value_b >= 0.95) {
                grouping_b = 3;
                factor_b = +1;
        } else if (value_b >= 0.85)
                grouping_b = 2;
        else if (value_b >= 0.65)
                grouping_b = 1;
        else
                grouping_b = 0;
        confidence_b = factor_b * rel_b->confidence;

        if (grouping_a < grouping_b)
                result = -1;
        else if (grouping_a > grouping_b)
                result = +1;
        else if (!confidence_a && confidence_b && grouping_b < 3)
                result = +1;
        else if (!confidence_a && confidence_b)
                result = -1;
        else if (!confidence_b && confidence_a && grouping_a < 3)
                result = -1;
        else if (!confidence_b && confidence_a)
                result = +1;
        else if (confidence_a < confidence_b)
                result = -1;
        else if (confidence_a > confidence_b)
                result = +1;
        else
                result = 0;

        gibbon_reliability_free (rel_a);
        gibbon_reliability_free (rel_b);

        return result;
}

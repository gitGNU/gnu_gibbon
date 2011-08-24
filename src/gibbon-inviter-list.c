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

#include "gibbon-inviter-list.h"
#include "gibbon-reliability.h"

struct _GibbonInviterListPrivate {
        GHashTable *hash;
        GtkListStore *store;
        GtkTreeModel *model;
};

struct GibbonInviter {
        GtkTreeIter iter;
        
        gint saved_count;
};

static GType gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_N_COLUMNS];

#define GIBBON_INVITER_LIST_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                       GIBBON_TYPE_INVITER_LIST,           \
                                       GibbonInviterListPrivate))
                                             
G_DEFINE_TYPE (GibbonInviterList, gibbon_inviter_list, G_TYPE_OBJECT);

static void free_inviter_name (gpointer name);
static void free_inviter (gpointer inviter);
/* FIXME! Should go into a utility module!  */
static gint compare_utf8_string (GtkTreeModel *model,
                                 GtkTreeIter *a,
                                 GtkTreeIter *b,
                                 gpointer user_data);


static void
gibbon_inviter_list_init (GibbonInviterList *self)
{
        GtkListStore *store;
        GtkTreeModel *model;
        
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_INVITER_LIST, 
                                                  GibbonInviterListPrivate);

        self->priv->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  free_inviter_name,
                                                  free_inviter);

        store = gtk_list_store_new (GIBBON_INVITER_LIST_N_COLUMNS, 
                                    G_TYPE_STRING,
                                    G_TYPE_DOUBLE,
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    GDK_TYPE_PIXBUF,
                                    GIBBON_TYPE_RELIABILITY,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    GIBBON_TYPE_COUNTRY,
                                    GDK_TYPE_PIXBUF,
                                    G_TYPE_STRING,
                                    G_TYPE_BOOLEAN);
        self->priv->store = store;
        
        model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (store));
        self->priv->model = model;
        
        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (store), 
                GIBBON_INVITER_LIST_COL_NAME,
                compare_utf8_string, 
                GINT_TO_POINTER (GIBBON_INVITER_LIST_COL_NAME), 
                NULL);
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                              GIBBON_INVITER_LIST_COL_NAME, 
                                              GTK_SORT_ASCENDING);
}

static void
gibbon_inviter_list_finalize (GObject *object)
{
        GibbonInviterList *self = GIBBON_INVITER_LIST (object);

        if (self->priv->hash)
                g_hash_table_destroy (self->priv->hash);
        
        if (self->priv->model)
                g_object_unref (self->priv->model);

        G_OBJECT_CLASS (gibbon_inviter_list_parent_class)->finalize (object);
}

static void
gibbon_inviter_list_class_init (GibbonInviterListClass *klass)
{
        GObjectClass* parent_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonInviterListPrivate));

        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_NAME] = 
                G_TYPE_STRING;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_RATING] = 
                G_TYPE_DOUBLE;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_EXPERIENCE] = 
                G_TYPE_UINT;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_CLIENT] =
                G_TYPE_STRING;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_CLIENT_ICON] =
                GDK_TYPE_PIXBUF;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_RELIABILITY] =
                GIBBON_TYPE_RELIABILITY;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_SAVED_COUNT] =
                G_TYPE_INT;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_HOSTNAME] =
                G_TYPE_STRING;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_COUNTRY] =
                GIBBON_TYPE_COUNTRY;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_COUNTRY_ICON] =
                GDK_TYPE_PIXBUF;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_EMAIL] =
                G_TYPE_STRING;
        gibbon_inviter_list_column_types[GIBBON_INVITER_LIST_COL_UPDATING_SAVEDCOUNT] =
                G_TYPE_BOOLEAN;
                
        G_OBJECT_CLASS (parent_class)->finalize = gibbon_inviter_list_finalize;
}

GibbonInviterList *
gibbon_inviter_list_new ()
{
        GibbonInviterList *self = g_object_new (GIBBON_TYPE_INVITER_LIST, NULL);

        return self;
}

static void
free_inviter_name (gpointer name)
{
        if (name)
                g_free (name);
}

static void
free_inviter (gpointer _inviter)
{
        struct GibbonInviter *inviter = (struct GibbonInviter *) _inviter;
        
        if (inviter) {
                g_free (_inviter);
        }
}

void
gibbon_inviter_list_set (GibbonInviterList *self, 
                        const gchar *name,
                        gdouble rating,
                        guint experience,
                        gdouble reliability,
                        guint confidence,
                        const gchar *client,
                        const GdkPixbuf *client_icon,
                        const gchar *hostname,
                        const GibbonCountry *country,
                        const gchar *email)
{
        struct GibbonInviter *inviter;
        GibbonReliability rel;
        const GdkPixbuf *country_icon;

        g_return_if_fail (GIBBON_IS_INVITER_LIST (self));
        g_return_if_fail (name);
        
        rel.value = reliability;
        rel.confidence = confidence;

        inviter = g_hash_table_lookup (self->priv->hash, name);
        if (!inviter) {
                inviter = g_malloc0 (sizeof *inviter);
                g_hash_table_insert (self->priv->hash, g_strdup (name), inviter);
                gtk_list_store_append (self->priv->store, 
                                       &inviter->iter);
                inviter->saved_count = -1;
        }

        country_icon = gibbon_country_get_pixbuf (country);
        gtk_list_store_set (self->priv->store,
                            &inviter->iter,
                            GIBBON_INVITER_LIST_COL_NAME, name,
                            GIBBON_INVITER_LIST_COL_RATING, rating,
                            GIBBON_INVITER_LIST_COL_EXPERIENCE, experience,
                            GIBBON_INVITER_LIST_COL_CLIENT, client,
                            GIBBON_INVITER_LIST_COL_CLIENT_ICON, client_icon,
                            GIBBON_INVITER_LIST_COL_RELIABILITY, &rel,
                            GIBBON_INVITER_LIST_COL_HOSTNAME, hostname,
                            GIBBON_INVITER_LIST_COL_COUNTRY, country,
                            GIBBON_INVITER_LIST_COL_COUNTRY_ICON, country_icon,
                            GIBBON_INVITER_LIST_COL_EMAIL, email,
                            GIBBON_INVITER_LIST_COL_UPDATING_SAVEDCOUNT, TRUE,
                            -1);
}

void
gibbon_inviter_list_connect_view (GibbonInviterList *self, GtkTreeView *view)
{
        g_return_if_fail (GIBBON_IS_INVITER_LIST (self));
        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        gtk_tree_view_set_model (view, self->priv->model);
}

void
gibbon_inviter_list_clear (GibbonInviterList *self)
{
        g_return_if_fail (GIBBON_IS_INVITER_LIST (self));
        
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

gboolean
gibbon_inviter_list_exists (const GibbonInviterList *self, const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_INVITER_LIST (self), FALSE);

        return (gboolean) g_hash_table_lookup (self->priv->hash, name);
}

void
gibbon_inviter_list_remove (GibbonInviterList *self,
                           const gchar *name)
{
        struct GibbonInviter *inviter = g_hash_table_lookup (self->priv->hash,
                                                           name);
        GtkTreeIter iter;

        if (!inviter)
                return;

        iter = inviter->iter;

        gtk_list_store_remove (self->priv->store, &iter);

        (void) g_hash_table_remove (self->priv->hash, name);
}

gint
gibbon_inviter_list_get_saved_count (const GibbonInviterList *self,
                                     const gchar *name)
{
        struct GibbonInviter *player;

        g_return_val_if_fail (GIBBON_IS_INVITER_LIST (self), -1);

        player = g_hash_table_lookup (self->priv->hash, name);
        if (!player)
                return -1;

        return player->saved_count;
}


void
gibbon_inviter_list_set_saved_count (GibbonInviterList *self,
                                     const gchar *name, gint count)
{
        gchar *stringified;

        struct GibbonInviter *player;

        g_return_if_fail (GIBBON_IS_INVITER_LIST (self));

        player = g_hash_table_lookup (self->priv->hash, name);
        g_return_if_fail (player != NULL);

        player->saved_count = count;

        if (count < 0) {
                stringified = NULL;
        } else {
                stringified = g_strdup_printf ("%d", count);
        }

        gtk_list_store_set (self->priv->store,
                            &player->iter,
                            GIBBON_INVITER_LIST_COL_SAVED_COUNT, stringified,
                            GIBBON_INVITER_LIST_COL_UPDATING_SAVEDCOUNT,
                                    stringified ? FALSE : TRUE,
                            -1);
}

void
gibbon_inviter_list_update_country (GibbonInviterList *self,
                                    const gchar *hostname,
                                    const GibbonCountry *country)
{
        GtkTreeIter iter;
        gboolean valid;
        gchar *stored_hostname;

        g_return_if_fail (GIBBON_IS_INVITER_LIST (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (GIBBON_IS_COUNTRY (country));

        valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->priv->store),
                                               &iter);
        while (valid) {
                gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                                    GIBBON_INVITER_LIST_COL_HOSTNAME,
                                    &stored_hostname,
                                    -1);

                if (0 == g_strcmp0 (hostname, stored_hostname)) {
                        gtk_list_store_set (self->priv->store,
                                            &iter,
                                            GIBBON_INVITER_LIST_COL_COUNTRY,
                                            country,
                                            GIBBON_INVITER_LIST_COL_COUNTRY_ICON,
                                            gibbon_country_get_pixbuf (country),
                                            -1);
                }
                valid = gtk_tree_model_iter_next (
                                GTK_TREE_MODEL (self->priv->store),
                                &iter);
        }
}

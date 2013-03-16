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

#include <stdlib.h>

#include <glib-object.h>
#include <glib/gi18n.h>

#include "gibbon-util.h"
#include "gibbon-analysis-move.h"

/* FIXME! These lists should  be retrieved online!  */
static gboolean initialized = FALSE;
static struct GibbonUtilBotInfo {
        const gchar *hostname;
        guint port;
        const gchar *login;
} playing_bots[] = {
                { "fibs.com", 4321, "bonehead" },
                { "fibs.com", 4321, "BlunderBot" },
                { "fibs.com", 4321, "BlunderBot_II" },
                { "fibs.com", 4321, "BlunderBot_III" },
                { "fibs.com", 4321, "BlunderBot_IV" },
                { "fibs.com", 4321, "BlunderBot_IX" },
                { "fibs.com", 4321, "BlunderBot_V" },
                { "fibs.com", 4321, "BlunderBot_VI" },
                { "fibs.com", 4321, "BlunderBot_VII" },
                { "fibs.com", 4321, "BlunderBot_VIII" },
                { "fibs.com", 4321, "BlunderBot_X" },
                { "fibs.com", 4321, "GammonBot" },
                { "fibs.com", 4321, "GammonBot_II" },
                { "fibs.com", 4321, "GammonBot_III" },
                { "fibs.com", 4321, "GammonBot_IV" },
                { "fibs.com", 4321, "GammonBot_IX" },
                { "fibs.com", 4321, "GammonBot_V" },
                { "fibs.com", 4321, "GammonBot_VI" },
                { "fibs.com", 4321, "GammonBot_VII" },
                { "fibs.com", 4321, "GammonBot_VIII" },
                { "fibs.com", 4321, "GammonBot_X" },
                { "fibs.com", 4321, "GammonBot_XI" },
                { "fibs.com", 4321, "GammonBot_XII" },
                { "fibs.com", 4321, "GammonBot_XIII" },
                { "fibs.com", 4321, "GammonBot_XIV" },
                { "fibs.com", 4321, "GammonBot_XIX" },
                { "fibs.com", 4321, "GammonBot_XV" },
                { "fibs.com", 4321, "GammonBot_XVI" },
                { "fibs.com", 4321, "GammonBot_XVII" },
                { "fibs.com", 4321, "GammonBot_XVIII" },
                { "fibs.com", 4321, "GammonBot_XX" },
                { "fibs.com", 4321, "MonteCarlo" },
                { "fibs.com", 4321, "PhaedrusBot" }
};

static struct GibbonUtilBotInfo daemons[] = {
                { "fibs.com", 4321, "MissManners" },
                { "fibs.com", 4321, "monitor" },
                { "fibs.com", 4321, "RepBotNG" },
                { "fibs.com", 4321, "RoboCop" },
                { "fibs.com", 4321, "TourneyBot" },
};

static int gibbon_util_compare_bot_info (const void *p1, const void *p2);

gchar **
gibbon_strsplit_ws (const gchar *string)
{
        gchar **vector = NULL;
        GSList *list = NULL;
        GSList *iter;
        gsize i, num_tokens = 0;
        const gchar *start;
        const gchar *ptr;
        gchar *p;

        if (!string) {
                vector = g_new (gchar *, num_tokens + 1);
                vector[0] = NULL;
                return vector;
        }

        ptr = string;

        while (1) {
                while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n'
                       || *ptr == '\v' || *ptr == '\f' || *ptr == '\r')
                        ++ptr;
                if (!*ptr)
                        break;
                start = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n'
                       && *ptr != '\v' && *ptr != '\f' && *ptr != '\r')
                        ++ptr;
                list = g_slist_prepend (list, g_strndup (start, ptr - start));
                ++num_tokens;
        }

        vector = g_new (gchar *, num_tokens + 1);
        iter = list;
        for (i = 0; i < num_tokens; ++i) {
                vector[num_tokens - i - 1] = iter->data;
                iter = iter->next;
        }
        vector[num_tokens] = NULL;
        if (num_tokens) {
                /* Remove trailing whitespace.  */
                p = vector[num_tokens - 1];
                while (*p && *p != ' ' && *p != '\t' && *p != '\n'
                       && *ptr != '\v' && *ptr != '\f' && *p != '\r')
                        ++p;
                *p = 0;
        }

        g_slist_free (list);

        return vector;
}

gchar **
gibbon_strsplit_set (const gchar *string, const gchar *set, gint max_tokens)
{
        gchar **vector = NULL;
        GSList *list = NULL;
        GSList *iter;
        gsize i, num_tokens = 0;
        const gchar *start;
        const gchar *ptr;
        gchar lookup[256];
        gchar *p;

        if (!string) {
                vector = g_new (gchar *, num_tokens + 1);
                vector[0] = NULL;
                return vector;
        }

        memset (lookup, 0, sizeof lookup);
        ptr = set;
        while (*ptr) {
                lookup[(guchar) *ptr] = 1;
                ++ptr;
        }

        ptr = string;

        while (1) {
                while (lookup[(guchar) *ptr])
                        ++ptr;
                if (!*ptr)
                        break;
                if (num_tokens + 1 >= max_tokens) {
                        ++num_tokens;
                        list = g_slist_prepend (list, g_strdup (ptr));
                        break;
                }
                start = ptr;
                while (*ptr && !lookup[(guchar) *ptr])
                        ++ptr;
                list = g_slist_prepend (list, g_strndup (start, ptr - start));

                ++num_tokens;
        }

        vector = g_new (gchar *, num_tokens + 1);
        iter = list;
        for (i = 0; i < num_tokens; ++i) {
                vector[num_tokens - i - 1] = iter->data;
                iter = iter->next;
        }
        if (num_tokens && num_tokens <= max_tokens) {
                /* Remove trailing delimiters.  */
                p = vector[num_tokens - 1] + strlen (vector[num_tokens - 1]);
                --p;
                while (lookup[(guchar) *p])
                        *p-- = 0;
        }
        vector[num_tokens] = NULL;

        g_slist_free (list);

        return vector;
}

const gchar *
gibbon_skip_ws_tokens (const gchar *string, const gchar * const * const tokens,
                       gsize num)
{
        gsize i;
        gsize length;
        const gchar *previous;
        const gchar *retval;

        retval = string;

        while (*retval == ' ' || *retval == '\t' || *retval == '\n'
               || *retval == '\v' || *retval == '\f' || *retval == '\r')
                ++retval;

        if (!*retval)
                return NULL;

        for (i = 0; i < num; ++i) {
                if (!tokens[i])
                        return NULL;
                length = strlen (tokens[i]);
                if (strncmp (tokens[i], retval, length))
                        return NULL;
                retval += length;
                while (*retval == ' ' || *retval == '\t' || *retval == '\n'
                       || *retval == '\v' || *retval == '\f' || *retval == '\r')
                        ++retval;
        }

        if (!*retval)
                return NULL;

        do {
                previous = retval - 1;
                if (*previous != ' ' && *previous != '\t' && *previous != '\n'
                    && *previous != '\v' && *previous != '\f' && *previous != '\r')
                        break;
                --retval;
        } while (1);
        if (*retval == ' ' || *retval == '\t' || *retval == '\n'
            || *retval == '\v' || *retval == '\f' || *retval == '\r')
                ++retval;

        return retval;
}

enum GibbonClientType
gibbon_get_client_type (const gchar *client_name, const gchar *user_name,
                        const gchar *host_name, guint port)
{
        struct GibbonUtilBotInfo info;

        if (!client_name)
                return GibbonClientRegular;

        if (0 == strncmp ("OdesysMobileR", client_name, 13))
                return GibbonClientMobile;
        if (0 == strncmp ("BGOnline ", client_name, 9))
                return GibbonClientMobile;
        if (0 == strncmp ("Gibbon ", client_name, 7))
                return GibbonClientGibbon;

        if (!initialized) {
                initialized = TRUE;
                qsort (playing_bots, (sizeof playing_bots) / (sizeof playing_bots[0]),
                       sizeof playing_bots[0], gibbon_util_compare_bot_info);
                qsort (daemons, (sizeof daemons) / (sizeof daemons[0]),
                       sizeof daemons[0], gibbon_util_compare_bot_info);
        }

        info.hostname = host_name;
        info.port = port;
        info.login = user_name;
        if (bsearch (&info, playing_bots,
                     (sizeof playing_bots) / (sizeof playing_bots[0]),
                     sizeof playing_bots[0],
                     gibbon_util_compare_bot_info))
                return GibbonClientBot;
        if (bsearch (&info, daemons,
                     (sizeof daemons) / (sizeof daemons[0]),
                     sizeof daemons[0],
                     gibbon_util_compare_bot_info))
                return GibbonClientDaemon;

        return GibbonClientRegular;
}

static
int gibbon_util_compare_bot_info (const void *p1, const void *p2)
{
        int retval;

        struct GibbonUtilBotInfo *i1 = (struct GibbonUtilBotInfo *) p1;
        struct GibbonUtilBotInfo *i2 = (struct GibbonUtilBotInfo *) p2;

        retval = g_strcmp0 (i1->login, i2->login);
        if (retval)
                return retval;

        if (i1->port < i2->port)
                return -1;
        else if (i1->port > i2->port)
                return +1;

        return g_strcmp0 (i1->hostname, i2->hostname);
}

void
gibbon_safe_object_unref (gpointer data)
{
        if (data)
                g_object_unref (data);
}

gchar *
gibbon_trim (gchar *string)
{
        gchar *ptr;
        size_t l;

        while (' ' == *string || ('\011' <= *string && '\015' >= *string))
                ++string;

        l = strlen (string);
        if (!l)
                return string;

        ptr = string + l;
        --ptr;

        while (' ' == *ptr || ('\011' <= *ptr && '\015' >= *ptr))
                --ptr;
        ++ptr;
        *ptr = 0;

        return string;
}

gboolean
gibbon_chareq (const char *str1, const char *str2)
{
        if (!str1 || !str1[0] || str1[1]
            || !str2 || !str2[0] || str2[1])
                return FALSE;
        return str1[0] ==  str2[0];
}

gdouble
gibbon_money_equity (const gdouble p[5])
{
        return 2.0f * p[GIBBON_ANALYSIS_MOVE_PWIN] - 1.0f
                        + p[GIBBON_ANALYSIS_MOVE_PWIN_GAMMON]
                        + p[GIBBON_ANALYSIS_MOVE_PWIN_BACKGAMMON]
                        - p[GIBBON_ANALYSIS_MOVE_PLOSE_GAMMON]
                        - p[GIBBON_ANALYSIS_MOVE_PLOSE_BACKGAMMON];

}

#define GIBBON_CHUNK_SIZE 8192
gboolean
gibbon_slurp_file (const gchar *path, char **buffer, gsize *bytes_read_out,
                   GCancellable *cancellable, GError **error)
{
        gsize bytes_allocated = 0;
        gsize bytes_read = 0;
        gssize chunk_size;
        GFile *file = g_file_new_for_path (path);
        GInputStream *fstream = G_INPUT_STREAM (g_file_read (file, NULL, error));

        *buffer = NULL;

        g_object_unref (file);

        if (!fstream)
                return FALSE;

        do {
                if (bytes_read <= bytes_allocated) {
                        bytes_allocated += GIBBON_CHUNK_SIZE;
                        *buffer = g_realloc (*buffer, bytes_allocated);
                }
                chunk_size = g_input_stream_read (fstream, *buffer + bytes_read,
                                                  GIBBON_CHUNK_SIZE,
                                                  cancellable, error);
                if (chunk_size > 0) {
                        bytes_read += chunk_size;
                }
        } while (chunk_size > 0);

        g_object_unref (fstream);

        if (chunk_size < 0) {
                g_free (*buffer);
                return FALSE;
        }

        *buffer = g_realloc (*buffer, bytes_read);

        if (bytes_read_out)
                *bytes_read_out = bytes_read;

        return TRUE;
}

GQuark
gibbon_error_quark (void)
{
        return g_quark_from_static_string ("gibbon-error-quark");
}

gint
gibbon_compare_string_column (GtkTreeModel *model,
                              GtkTreeIter *a, GtkTreeIter *b,
                              gpointer user_data)
{
        gchar *str_a = NULL;
        gchar *str_b = NULL;
        gchar *key_a;
        gchar *key_b;

        gint result;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &str_a, -1);
        key_a = g_utf8_collate_key (str_a, -1);

        gtk_tree_model_get (model, b, col, &str_b, -1);
        key_b = g_utf8_collate_key (str_b, -1);

        result = g_ascii_strcasecmp (key_a, key_b);

        g_free (str_a);
        g_free (str_b);
        g_free (key_a);
        g_free (key_b);

        return result;
}

gint
gibbon_compare_double_column (GtkTreeModel *model,
                              GtkTreeIter *a, GtkTreeIter *b,
                              gpointer user_data)
{
        gdouble dbl_a = 0.0f, dbl_b = 0.0f;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &dbl_a, -1);
        gtk_tree_model_get (model, b, col, &dbl_b, -1);

        if (dbl_a < dbl_b)
                return -1;
        else if (dbl_a > dbl_b)
                return +1;
        else
                return 0;
}

gint
gibbon_compare_uint_column (GtkTreeModel *model,
                              GtkTreeIter *a, GtkTreeIter *b,
                              gpointer user_data)
{
        guint u_a = 0, u_b = 0.0f;

        gint col = GPOINTER_TO_INT (user_data);

        gtk_tree_model_get (model, a, col, &u_a, -1);
        gtk_tree_model_get (model, b, col, &u_b, -1);

        if (u_a < u_b)
                return -1;
        else if (u_a > u_b)
                return +1;
        else
                return 0;
}

gboolean
gibbon_debug (const gchar *realm)
{
        const gchar *gibbon_debug = g_getenv ("GIBBON_DEBUG");
        gchar **tokens;
        guint i, num_tokens;
        gboolean debug = FALSE;

        g_return_val_if_fail (realm != NULL, FALSE);
        g_return_val_if_fail (*realm != 0, FALSE);

        if (!gibbon_debug)
                return FALSE;

        tokens = g_strsplit_set (gibbon_debug, " \t,;:", -1);
        num_tokens = g_strv_length (tokens);
        for (i = 0; i < num_tokens; ++i) {
                if (!g_strcmp0 (realm, tokens[i])) {
                        debug = TRUE;
                        break;
                } else if (!g_strcmp0 ("all", tokens[i])) {
                        debug = TRUE;
                        break;
                }
                g_strfreev (tokens);
        }

        return debug;
}

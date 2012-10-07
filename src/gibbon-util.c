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

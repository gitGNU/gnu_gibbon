/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

/* This is a parser for inbound FIBS messages.  It is _not_
 * complete.  Only the messages that are meaningful to Gibbon are handled,
 * everything else is just passed through.  For example, Gibbon never
 * issues "whois" commands, and therefore does not bother parsing the
 * output of that command.  Instead, it will be just printed to the server
 * console as an unhandled string which is exactly what we want because
 * the command must have been manually sent manually by the user.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include <errno.h>

#include "gibbon-clip.h"
#include "gibbon-util.h"

static gboolean gibbon_clip_parse_clip (const gchar *line,
                                        gchar **tokens,
                                        GSList **result);
static gboolean gibbon_clip_parse_clip_welcome (const gchar *line,
                                                gchar **tokens,
                                                GSList **result);
static gboolean gibbon_clip_parse_clip_own_info (const gchar *line,
                                                 gchar **tokens,
                                                 GSList **result);
static gboolean gibbon_clip_parse_clip_who_info (const gchar *line,
                                                 gchar **tokens,
                                                 GSList **result);
static gboolean gibbon_clip_parse_clip_somebody_message (const gchar *line,
                                                         gchar **tokens,
                                                         GSList **result);
static gboolean gibbon_clip_parse_clip_somebody_something (const gchar *line,
                                                           gchar **tokens,
                                                           GSList **result);
static gboolean gibbon_clip_parse_clip_message (const gchar *line,
                                                gchar **tokens,
                                                GSList **result);
static gboolean gibbon_clip_parse_clip_somebody (const gchar *line,
                                                 gchar **tokens,
                                                 GSList **result);
static gboolean gibbon_clip_parse_clip_something (const gchar *line,
                                                  gchar **tokens,
                                                  GSList **result);

static gboolean gibbon_clip_parse_error (const gchar *line,
                                         gchar **tokens,
                                         GSList **result);

static gboolean gibbon_clip_parse_board (const gchar *line,
                                         gchar **tokens,
                                         GSList **result);

static GSList *gibbon_clip_alloc_int (GSList *list, enum GibbonClipType type,
                                      gint64 value);
static GSList *gibbon_clip_alloc_double (GSList *list, enum GibbonClipType type,
                                         gdouble value);
static GSList *gibbon_clip_alloc_string (GSList *list, enum GibbonClipType type,
                                         const gchar *value);
static gboolean gibbon_clip_extract_integer (const gchar *str, gint64 *result,
                                             const gchar *what,
                                             gint64 lower, gint64 upper);
static gboolean gibbon_clip_extract_double (const gchar *str, gdouble *result,
                                            const gchar *what,
                                            gdouble lower, gdouble upper);
static gboolean gibbon_clip_parse_chop (gchar *str, gchar c);

GSList *
gibbon_clip_parse (const gchar *line)
{
        gboolean success = FALSE;
        GSList *result = NULL;
        gchar **tokens = gibbon_strsplit_ws (line);
        gchar *first = tokens[0];

        if (first[0] >= '1' && first[0] <= '9' && !first[1])
                success = gibbon_clip_parse_clip (line, tokens, &result);
        else if (first[0] == '1' && first[1] >= '0' && first[1] <= '9'
                 && !first[2])
                success = gibbon_clip_parse_clip (line, tokens, &result);

        switch (first[0]) {
        case '*':
                if ('*' == first[1] && !first[2])
                        success = gibbon_clip_parse_error (line, tokens,
                                                           &result);
                        break;
        case 'b':
                if (0 == strncmp ("board:", first, 6))
                        success = gibbon_clip_parse_board (line, tokens,
                                                           &result);
                break;
        }

        g_strfreev (tokens);
        if (!success) {
                gibbon_clip_free_result (result);
                return NULL;
        }

        return g_slist_reverse (result);
}

static gboolean
gibbon_clip_parse_clip (const gchar *line, gchar **tokens,
                        GSList **result)
{
        enum GibbonClipCode code;
        gchar *first = tokens[0];

        code = first[0] - '0';
        if (first[1]) {
                code *= 10;
                code += first[1] - '0';
        }

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                               (gint64) code);

        switch (code) {
                case GIBBON_CLIP_CODE_WELCOME:
                        return gibbon_clip_parse_clip_welcome (line, tokens,
                                                               result);
                case GIBBON_CLIP_CODE_OWN_INFO:
                        return gibbon_clip_parse_clip_own_info (line, tokens,
                                                                result);
                case GIBBON_CLIP_CODE_WHO_INFO:
                        return gibbon_clip_parse_clip_who_info (line, tokens,
                                                                result);
                case GIBBON_CLIP_CODE_LOGIN:
                case GIBBON_CLIP_CODE_LOGOUT:
                        return gibbon_clip_parse_clip_somebody_message (line,
                                                                        tokens,
                                                                        result);
                case GIBBON_CLIP_CODE_SAYS:
                case GIBBON_CLIP_CODE_SHOUTS:
                case GIBBON_CLIP_CODE_KIBITZES:
                case GIBBON_CLIP_CODE_WHISPERS:
                case GIBBON_CLIP_CODE_YOU_SAY:
                        return gibbon_clip_parse_clip_somebody_something (line,
                                                                          tokens,
                                                                          result);
                case GIBBON_CLIP_CODE_MESSAGE:
                        return gibbon_clip_parse_clip_message (line, tokens,
                                                               result);
                case GIBBON_CLIP_CODE_MESSAGE_DELIVERED:
                case GIBBON_CLIP_CODE_MESSAGE_SAVED:
                        return gibbon_clip_parse_clip_somebody (line,
                                                                tokens,
                                                                result);
                case GIBBON_CLIP_CODE_YOU_SHOUT:
                case GIBBON_CLIP_CODE_YOU_KIBITZ:
                case GIBBON_CLIP_CODE_YOU_WHISPER:
                        return gibbon_clip_parse_clip_something (line, tokens,
                                                                 result);
                case GIBBON_CLIP_CODE_MOTD:
                case GIBBON_CLIP_CODE_MOTD_END:
                case GIBBON_CLIP_CODE_WHO_INFO_END:
                        if (1 == g_strv_length (tokens))
                                return TRUE;
                default:
                        break;
        }

        return FALSE;
}

static gboolean
gibbon_clip_parse_clip_welcome (const gchar *line, gchar **tokens,
                                GSList **result)
{
        gint64 timestamp;

        if (4 != g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);
        if (!gibbon_clip_extract_integer (tokens[2], &timestamp,
                                          "login timestamp",
                                          G_MININT, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_TIMESTAMP,
                                         timestamp);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[3]);

        return TRUE;
}


static gboolean
gibbon_clip_parse_clip_own_info (const gchar *line, gchar **tokens,
                                 GSList **result)
{
        gint64 i;
        gdouble d;

        if (22 != g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        if (!gibbon_clip_extract_integer (tokens[2], &i,
                                          "allowpip info",
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[3], &i,
                                          "autoboard info",
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[4], &i, "autodouble info",
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[5], &i, "automove info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[6], &i, "away info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[7], &i, "bell info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[8], &i, "crawford info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[9], &i, "double info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[10], &i, "experience info",
                                          0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[11], &i, "greedy info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[12], &i, "moreboards info",
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[13], &i, "moves info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[14], &i, "notify info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_double (tokens[15], &d, "rating info",
                                         0, G_MAXDOUBLE))
                return FALSE;
        *result = gibbon_clip_alloc_double (*result, GIBBON_CLIP_TYPE_DOUBLE,
                                            d);

        if (!gibbon_clip_extract_integer (tokens[16], &i, "ratings info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[17], &i, "ready info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (g_strcmp0 ("unlimited", tokens[18])) {
                if (!gibbon_clip_extract_integer (tokens[18], &i,
                                                  "redoubles info",
                                                  0, G_MAXINT))
                        return FALSE;
        } else {
                i = -1;
        }
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[19], &i, "report info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[20], &i, "silent info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result,
                                               GIBBON_CLIP_TYPE_BOOLEAN,
                                               i);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[21]);

        return TRUE;
}

static gboolean
gibbon_clip_parse_clip_who_info (const gchar *line, gchar **tokens,
                                 GSList **result)
{
        gint64 i;
        gdouble d;
        gchar *s;

        if (13 != g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        s = tokens[2];
        if ('-' == s[0] && !s[1])
                s = "";
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME, s);

        s = tokens[3];
        if ('-' == s[0] && !s[1])
                s = "";
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME, s);

        if (!gibbon_clip_extract_integer (tokens[4], &i, "ready who info",
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[5], &i, "away who info", 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_double (tokens[6], &d, "rating who info",
                                         0, G_MAXDOUBLE))
                return FALSE;
        *result = gibbon_clip_alloc_double (*result, GIBBON_CLIP_TYPE_DOUBLE,
                                            d);

        if (!gibbon_clip_extract_integer (tokens[7], &i, "experience who info",
                                          0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[8], &i, "idle who info",
                                          0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[9], &i, "idle who timestamp",
                                          0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_TIMESTAMP,
                                         i);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[10]);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[11]);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[12]);

        return TRUE;
}

static gboolean
gibbon_clip_parse_clip_somebody_message (const gchar *line, gchar **tokens,
                                         GSList **result)
{
        const gchar *s;
        gsize length;
        gchar *s2;

        if (3 > g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        s = gibbon_skip_ws_tokens (line, (const gchar * const *) tokens, 2);
        length = 1 + strlen (s);
        s2 = g_alloca (length);
        memcpy (s2, s, length);

        while (gibbon_clip_parse_chop (s2, '.')) {}
        while (gibbon_clip_parse_chop (s2, ' ')) {}
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            s2);

        return TRUE;
}


static gboolean
gibbon_clip_parse_clip_somebody_something (const gchar *line, gchar **tokens,
                                           GSList **result)
{
        const gchar *s;

        if (3 > g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        s = gibbon_skip_ws_tokens (line, (const gchar * const *) tokens, 2);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            s);

        return TRUE;
}

static gboolean
gibbon_clip_parse_clip_message (const gchar *line, gchar **tokens,
                                GSList **result)
{
        gint64 i;
        const gchar *s;

        if (4 >= g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        if (!gibbon_clip_extract_integer (tokens[2], &i, "message timestamp",
                                          G_MININT, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_TIMESTAMP,
                                         i);

        s = gibbon_skip_ws_tokens (line, (const gchar * const* const) tokens,
                                   3);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            s);

        return TRUE;
}

static gboolean
gibbon_clip_parse_clip_somebody (const gchar *line, gchar **tokens,
                                 GSList **result)
{
        if (2 != g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[1]);

        return TRUE;
}

static gboolean
gibbon_clip_parse_clip_something (const gchar *line, gchar **tokens,
                                  GSList **result)
{
        const gchar *s;

        if (2 > g_strv_length (tokens))
                return FALSE;

        s = gibbon_skip_ws_tokens (line, (const gchar * const *) tokens, 1);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            s);

        return TRUE;
}

static gboolean
gibbon_clip_parse_error (const gchar *line, gchar **tokens,
                         GSList **result)
{
        const gchar *s;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_ERROR);

        s = gibbon_skip_ws_tokens (line,
                                   (const gchar * const* const) tokens, 1);

        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            s);

        return TRUE;
}

static gboolean
gibbon_clip_parse_board (const gchar *line, gchar **_tokens,
                         GSList **result)
{
        gchar **tokens;
        gsize num_tokens;
        gboolean retval = FALSE;

        tokens = g_strsplit (_tokens[0] + 6, ":", 0);
        num_tokens = g_strv_length (tokens);

        if (52 != num_tokens)
                goto bail_out;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_BOARD);

        /* Player's name.  */
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[0]);
        /* Opponent's name.  */
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[1]);

        retval = TRUE;

bail_out:
        g_strfreev (tokens);

        return retval;
}

static GSList *
gibbon_clip_alloc_int (GSList *list, enum GibbonClipType type, gint64 value)
{
        struct GibbonClipTokenSet *token = g_malloc (sizeof *token);

        token->type = type;
        token->v.i64 = value;

        return g_slist_prepend (list, token);
}

static GSList *
gibbon_clip_alloc_double (GSList *list, enum GibbonClipType type, gdouble value)
{
        struct GibbonClipTokenSet *token = g_malloc (sizeof *token);

        token->type = type;
        token->v.d = value;

        return g_slist_prepend (list, token);
}

static GSList *
gibbon_clip_alloc_string (GSList *list, enum GibbonClipType type,
                          const gchar *value)
{
        struct GibbonClipTokenSet *token = g_malloc (sizeof *token);

        token->type = type;
        token->v.s = g_strdup (value);

        return g_slist_prepend (list, token);
}

void
gibbon_clip_free_result (GSList *result)
{
        GSList *iter;
        struct GibbonClipTokenSet *set;

        if (G_UNLIKELY (!result))
                return;

        iter = result;
        while (iter) {
                set = (struct GibbonClipTokenSet *) iter->data;
                switch (set->type) {
                        case GIBBON_CLIP_TYPE_NAME:
                        case GIBBON_CLIP_TYPE_STRING:
                                g_free (set->v.s);
                                break;
                        default:
                                break;
                }
                iter = iter->next;
        }

        g_slist_free (result);
}

static gboolean
gibbon_clip_extract_integer (const gchar *str, gint64 *result,
                             const gchar *what, gint64 lower, gint64 upper)
{
        char *endptr;
        gint64 r;

        errno = 0;

        r = g_ascii_strtoll (str, &endptr, 10);

        if (errno) {
                g_printerr (_("Error parsing %s: `%s': %s.\n"),
                            what, str, strerror (errno));
                return FALSE;
        }

        if (*endptr != 0) {
                g_printerr (_("Error parsing %s: `%s': %s.\n"),
                            what, str, _("Trailing garbage in integer)"));
                return FALSE;
        }

        *result = (gint) r;
        if (*result < lower || *result > upper) {
                g_printerr (_("Error parsing %s: `%s':"
                              " value %lld outside valid range"
                              " (%lld to %lld).\n"),
                            what, str, r, lower, upper);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_clip_extract_double (const gchar *str, gdouble *result,
                            const gchar *what, gdouble lower, gdouble upper)
{
        char *endptr;
        gdouble r;

        errno = 0;

        r = g_ascii_strtod (str, &endptr);

        if (errno) {
                g_printerr (_("Error parsing %s: `%s': %s.\n"),
                            what, str, strerror (errno));
                return FALSE;
        }

        if (*endptr != 0) {
                g_printerr (_("Error parsing %s: `%s': %s.\n"),
                            what, str, _("Trailing garbage in double)"));
                return FALSE;
        }

        *result = r;
        if (*result < lower || *result > upper) {
                g_printerr (_("Error parsing %s: `%s':"
                              " value %f outside valid range"
                              " (%f to %f).\n"),
                            what, str, r, lower, upper);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_clip_parse_chop (gchar *str, gchar c)
{
        gsize length = strlen (str);

        if (length && str[length - 1] == c) {
                str[length - 1] = 0;
                return TRUE;
        }

        return FALSE;
}

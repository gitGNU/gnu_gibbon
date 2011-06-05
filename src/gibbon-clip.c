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
#include "gibbon-position.h"

#ifdef GIBBON_CLIP_DEBUG_BOARD_STATE
static const gchar *keys[] = {
                "player",
                "opponent",
                "match length",
                "player's score",
                "opponents's score",
                "home/bar",
                "point 0",
                "point 1",
                "point 2",
                "point 3",
                "point 4",
                "point 5",
                "point 6",
                "point 7",
                "point 8",
                "point 9",
                "point 10",
                "point 11",
                "point 12",
                "point 13",
                "point 14",
                "point 15",
                "point 16",
                "point 17",
                "point 18",
                "point 19",
                "point 20",
                "point 21",
                "point 22",
                "point 23",
                "home/bar",
                "turn",
                "player's die 0",
                "player's die 1",
                "opponent's die 0",
                "opponent's die 1",
                "doubling cube",
                "player may double",
                "opponent may double",
                "was doubled",
                "color",
                "direction",
                "home index",
                "bar index",
                "player's checkers on home",
                "opponent's checkers on home",
                "player's checkers on bar",
                "opponent's checkers on bar",
                "can move",
                "forced move",
                "did crawford",
                "redoubles",
                NULL
};

static void gibbon_clip_dump_board (const gchar *raw,
                                    gchar **tokens);
#endif /* #ifdef GIBBON_CLIP_DEBUG_BOARD_STATE */

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

static gboolean gibbon_clip_parse_youre_watching (const gchar *line,
                                                  gchar **tokens,
                                                  GSList **result);

static gboolean gibbon_clip_parse_and (const gchar *line,
                                       gchar **tokens,
                                       GSList **result);
static gboolean gibbon_clip_parse_resuming (const gchar *line,
                                            gchar **tokens,
                                            GSList **result);
static gboolean gibbon_clip_parse_start_match (const gchar *line,
                                               gchar **tokens,
                                               GSList **result);

static gboolean gibbon_clip_parse_wins (const gchar *line,
                                        gchar **tokens,
                                        GSList **result);

static gboolean gibbon_clip_parse_rolls (const gchar *line,
                                         gchar **tokens,
                                         GSList **result);
static gboolean gibbon_clip_parse_moves (const gchar *line,
                                         gchar **tokens,
                                         GSList **result);

static gboolean gibbon_clip_parse_movement (gchar *string, GSList **result);
static GSList *gibbon_clip_alloc_int (GSList *list, enum GibbonClipType type,
                                      gint64 value);
static GSList *gibbon_clip_alloc_double (GSList *list, enum GibbonClipType type,
                                         gdouble value);
static GSList *gibbon_clip_alloc_string (GSList *list, enum GibbonClipType type,
                                         const gchar *value);
static gboolean gibbon_clip_extract_integer (const gchar *str, gint64 *result,
                                             gint64 lower, gint64 upper);
static gboolean gibbon_clip_extract_double (const gchar *str, gdouble *result,
                                            gdouble lower, gdouble upper);
static gboolean gibbon_clip_chomp (gchar *str, gchar c);

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
        case 'Y':
                if (0 == g_strcmp0 ("You're", tokens[0])
                    && 0 == g_strcmp0 ("now", tokens[1])
                    && 0 == g_strcmp0 ("watching", tokens[2]))
                        success = gibbon_clip_parse_youre_watching (line,
                                                                    tokens,
                                                                    &result);
                break;
        }

        if (!success) {
                if (!tokens[1])
                        goto bail_out;

                switch (tokens[1][0]) {
                case 'a':
                        if (0 == g_strcmp0 ("and", tokens[1]))
                                success = gibbon_clip_parse_and (line, tokens,
                                                                 &result);
                        break;
                case 'm':
                        if (0 == g_strcmp0 ("moves", tokens[1]))
                                success = gibbon_clip_parse_moves (line, tokens,
                                                                  &result);
                        break;
                case 'r':
                        if (0 == g_strcmp0 ("rolls", tokens[1]))
                                success = gibbon_clip_parse_rolls (line, tokens,
                                                                   &result);
                        break;
                case 'w':
                        if (0 == g_strcmp0 ("wins", tokens[1]))
                                success = gibbon_clip_parse_wins (line, tokens,
                                                                  &result);
                        break;
                }
        }

bail_out:
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
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[3], &i,
                                          0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[4], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[5], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[6], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[7], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[8], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[9], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[10], &i, 0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[11], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[12], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[13], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[14], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_double (tokens[15], &d, 0, G_MAXDOUBLE))
                return FALSE;
        *result = gibbon_clip_alloc_double (*result, GIBBON_CLIP_TYPE_DOUBLE,
                                            d);

        if (!gibbon_clip_extract_integer (tokens[16], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[17], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (g_strcmp0 ("unlimited", tokens[18])) {
                if (!gibbon_clip_extract_integer (tokens[18], &i,
                                                  0, G_MAXINT))
                        return FALSE;
        } else {
                i = -1;
        }
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[19], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[20], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

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

        if (!gibbon_clip_extract_integer (tokens[4], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_integer (tokens[5], &i, 0, 1))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_BOOLEAN, i);

        if (!gibbon_clip_extract_double (tokens[6], &d, 0, G_MAXDOUBLE))
                return FALSE;
        *result = gibbon_clip_alloc_double (*result, GIBBON_CLIP_TYPE_DOUBLE,
                                            d);

        if (!gibbon_clip_extract_integer (tokens[7], &i, 0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[8], &i, 0, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i);

        if (!gibbon_clip_extract_integer (tokens[9], &i, 0, G_MAXINT))
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

        while (gibbon_clip_chomp (s2, '.')) {}
        while (gibbon_clip_chomp (s2, ' ')) {}
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

        if (!gibbon_clip_extract_integer (tokens[2], &i, G_MININT, G_MAXINT))
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
        gint64 color, direction, turn;
        gint64 i64;
        gint i;

        tokens = g_strsplit (_tokens[0] + 6, ":", 0);
        num_tokens = g_strv_length (tokens);

#ifdef GIBBON_CLIP_DEBUG_BOARD_STATE
        gibbon_clip_dump_board (line, tokens);
#endif

        if (52 != num_tokens)
                goto bail_out_board;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_BOARD);

        /* Player's name.  */
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[0]);
        /* Opponent's name.  */
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_STRING,
                                            tokens[1]);

        if (!gibbon_clip_extract_integer (tokens[2], &i64, 0, G_MAXINT))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);

        if (!gibbon_clip_extract_integer (tokens[3], &i64, 0, G_MAXINT))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);
        if (!gibbon_clip_extract_integer (tokens[4], &i64, 0, G_MAXINT))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);

        if (!gibbon_clip_extract_integer (tokens[40], &color, -1, 1))
                goto bail_out_board;

        if (!gibbon_clip_extract_integer (tokens[41], &direction, -1, 1))
                goto bail_out_board;

        if (!direction)
                goto bail_out_board;

        if (direction == GIBBON_POSITION_SIDE_BLACK) {
                for (i = 6; i < 30; ++i) {
                        if (!gibbon_clip_extract_integer (tokens[i], &i64,
                                                          -15, 15))
                                goto bail_out_board;

                        i64 *= color;

                        *result = gibbon_clip_alloc_int (*result,
                                                         GIBBON_CLIP_TYPE_INT,
                                                         i64);
                }
        } else {
                for (i = 29; i >= 0; --i) {
                        if (!gibbon_clip_extract_integer (tokens[i], &i64,
                                                          -15, 15))
                                goto bail_out_board;

                        i64 *= color;

                        *result = gibbon_clip_alloc_int (*result,
                                                         GIBBON_CLIP_TYPE_INT,
                                                         i64);
                }
        }

        if (!gibbon_clip_extract_integer (tokens[31], &turn, -1, 1))
                goto bail_out_board;

        if (turn == color) {
                if (!gibbon_clip_extract_integer (tokens[32], &i64,
                                                  0, 6))
                        goto bail_out_board;
                *result = gibbon_clip_alloc_int (*result,
                                                 GIBBON_CLIP_TYPE_INT,
                                                 i64);
                if (!gibbon_clip_extract_integer (tokens[33], &i64,
                                                  0, 6))
                        goto bail_out_board;
                *result = gibbon_clip_alloc_int (*result,
                                                 GIBBON_CLIP_TYPE_INT,
                                                 i64);
        } else if (turn) {
                if (!gibbon_clip_extract_integer (tokens[34], &i64,
                                                  0, 6))
                        goto bail_out_board;
                *result = gibbon_clip_alloc_int (*result,
                                                 GIBBON_CLIP_TYPE_INT,
                                                 -i64);
                if (!gibbon_clip_extract_integer (tokens[35], &i64,
                                                  0, 6))
                        goto bail_out_board;
                *result = gibbon_clip_alloc_int (*result,
                                                 GIBBON_CLIP_TYPE_INT,
                                                 -i64);
        }

        if (!gibbon_clip_extract_integer (tokens[36], &i64,
                                          1, G_MAXINT))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result,
                                         GIBBON_CLIP_TYPE_UINT,
                                         i64);
        if (!gibbon_clip_extract_integer (tokens[37], &i64,
                                          0, 1))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result,
                                         GIBBON_CLIP_TYPE_BOOLEAN,
                                         i64);
        if (!gibbon_clip_extract_integer (tokens[38], &i64,
                                          0, 1))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result,
                                         GIBBON_CLIP_TYPE_BOOLEAN,
                                         i64);

        if (!gibbon_clip_extract_integer (tokens[46], &i64,
                                          0, 15))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result,
                                         GIBBON_CLIP_TYPE_UINT,
                                         i64);
        if (!gibbon_clip_extract_integer (tokens[47], &i64,
                                          0, 15))
                goto bail_out_board;
        *result = gibbon_clip_alloc_int (*result,
                                         GIBBON_CLIP_TYPE_UINT,
                                         i64);

        retval = TRUE;

bail_out_board:
        g_strfreev (tokens);

        return retval;
}

static gboolean
gibbon_clip_parse_youre_watching (const gchar *line, gchar **tokens,
                                  GSList **result)
{
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_YOURE_WATCHING);
        gibbon_clip_chomp (tokens[3], '.');
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[3]);

        return TRUE;
}

static gboolean
gibbon_clip_parse_and (const gchar *line, gchar **tokens, GSList **result)
{
        if (!tokens[2])
                return FALSE;
        if (!tokens[3])
                return FALSE;

        if (0 == g_strcmp0 ("are", tokens[3])) {
                if (0 == g_strcmp0 ("resuming", tokens[4])
                    && 0 == g_strcmp0 ("their", tokens[5]))
                        return gibbon_clip_parse_resuming (line, tokens,
                                                           result);
        } else if (0 == g_strcmp0 ("start", tokens[3])) {
                return gibbon_clip_parse_start_match (line, tokens, result);
        }

        return TRUE;
}


static gboolean
gibbon_clip_parse_resuming (const gchar *line, gchar **tokens,
                            GSList **result)
{
        gint64 length;
        gchar *ptr;

        if (8 != g_strv_length (tokens))
                return FALSE;

        if (g_strcmp0 ("match.", tokens[7]))
                return FALSE;

        if (0 == g_strcmp0 ("unlimited", tokens[6])) {
                length = 0;
        } else {
                ptr = index (tokens[6], '-');
                if (!ptr)
                        return FALSE;
                if (g_strcmp0 ("-point", ptr))
                        return FALSE;
                *ptr = 0;
                if (!gibbon_clip_extract_integer (tokens[6], &length,
                                                  1, G_MAXINT))
                        return FALSE;
        }

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_RESUME_MATCH);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[0]);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[2]);
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         length);

        return TRUE;
}

static gboolean
gibbon_clip_parse_start_match (const gchar *line, gchar **tokens,
                               GSList **result)
{
        gint64 length = -1;

        if ((0 == g_strcmp0 ("a", tokens[4])
             || 0 == g_strcmp0 ("an", tokens[4]))) {
                if (0 == g_strcmp0 ("unlimited", tokens[5])
                    && 0 == g_strcmp0 ("match.", tokens[6])
                    && !tokens[7]) {
                        length = 0;
                } else if (gibbon_clip_extract_integer (tokens[5], &length,
                                                        1, G_MAXINT)
                           && 0 == g_strcmp0 ("point", tokens[6])
                           && 0 == g_strcmp0 ("match", tokens[7])
                           && !tokens[8]) {
                        /* Side-effect from extracting integer.  */
                }
        }

        if  (length == -1)
                return FALSE;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_START_MATCH);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[0]);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[2]);
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         length);

        return TRUE;
}

static gboolean
gibbon_clip_parse_wins (const gchar *line, gchar **tokens, GSList **result)
{
        gint64 i64;
        gint64 s0, s1;
        gchar *ptr;

        if (10 != g_strv_length (tokens))
                return FALSE;

        if (g_strcmp0 ("a", tokens[2]))
                return FALSE;
        if (g_strcmp0 ("point", tokens[4]))
                return FALSE;
        if (g_strcmp0 ("match", tokens[5]))
                return FALSE;
        if (g_strcmp0 ("against", tokens[6]))
                return FALSE;
        if (g_strcmp0 (".", tokens[9]))
                return FALSE;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_WIN_MATCH);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[0]);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[7]);

        if (!gibbon_clip_extract_integer (tokens[3], &i64, 1, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);

        ptr = index (tokens[8], '-');
        if (!ptr)
                return FALSE;

        *ptr++ = 0;
        if (!gibbon_clip_extract_integer (tokens[8], &s0, 1, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, s0);
        if (!gibbon_clip_extract_integer (ptr, &s1, 1, G_MAXINT))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, s1);

        if (s0 < i64)
                return FALSE;
        if (s0 < s1)
                return FALSE;
        if (s1 >= i64)
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_clip_parse_rolls (const gchar *line, gchar **tokens, GSList **result)
{
        gint64 i64;

        if (5 != g_strv_length (tokens))
                return FALSE;

        if (g_strcmp0 ("and", tokens[3]))
                return FALSE;

        gibbon_clip_chomp (tokens[4], '.');

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_ROLLS);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[0]);

        if (!gibbon_clip_extract_integer (tokens[2], &i64, 1, 6))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);
        if (!gibbon_clip_extract_integer (tokens[4], &i64, 1, 6))
                return FALSE;
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, i64);

        return TRUE;
}

static gboolean
gibbon_clip_parse_moves (const gchar *line, gchar **tokens, GSList **result)
{
        gint64 num_movements = g_strv_length (tokens) - 3;
        gint i;

        if (num_movements < 1 || num_movements > 4)
                return FALSE;

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         GIBBON_CLIP_CODE_MOVES);
        *result = gibbon_clip_alloc_string (*result, GIBBON_CLIP_TYPE_NAME,
                                            tokens[0]);
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                         num_movements);
        for (i = 0; i < num_movements; ++i)
                if (!gibbon_clip_parse_movement (tokens[i + 2], result))
                        return FALSE;

        return TRUE;
}

static gboolean
gibbon_clip_parse_movement (gchar *str, GSList **result)
{
        gint64 from, to;
        gchar *ptr;

        ptr = index (str, '-');
        if (!ptr)
                return FALSE;

        *ptr++ = 0;

        if (0 == g_strcmp0 ("bar", str))
                from = 0;
        else if (!gibbon_clip_extract_integer (str, &from, 1, 24))
                return FALSE;

        if (0 == g_strcmp0 ("off", ptr))
                to = 0;
        else if (!gibbon_clip_extract_integer (ptr, &to, 1, 24))
                return FALSE;

        if (!(from || to)) {
                return FALSE;
        } else if (!from) {
                if (to >= 19)
                        from = 25;
                else if (to > 6)
                        return FALSE;
        } else if (!to) {
                if (from >= 19)
                        to = 25;
                else if (from > 6)
                        return FALSE;
        }

        if (from == to) {
                return FALSE;
        } else if (from < to) {
                if (to - from > 6)
                        return FALSE;
        } else {
                if (from - to > 6)
                        return FALSE;
        }

        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, from);
        *result = gibbon_clip_alloc_int (*result, GIBBON_CLIP_TYPE_UINT, to);

        return TRUE;
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
                             gint64 lower, gint64 upper)
{
        char *endptr;
        gint64 r;

        if (!str)
                return FALSE;

        errno = 0;

        r = g_ascii_strtoll (str, &endptr, 10);

        if (errno)
                return FALSE;

        if (*endptr != 0)
                return FALSE;

        *result = (gint) r;
        if (*result < lower || *result > upper)
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_clip_extract_double (const gchar *str, gdouble *result,
                            gdouble lower, gdouble upper)
{
        char *endptr;
        gdouble r;

        if (!str)
                return FALSE;

        errno = 0;

        r = g_ascii_strtod (str, &endptr);

        if (errno)
                return FALSE;

        if (*endptr != 0)
                return FALSE;

        *result = r;
        if (*result < lower || *result > upper)
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_clip_chomp (gchar *str, gchar c)
{
        gsize length = strlen (str);

        if (length && str[length - 1] == c) {
                str[length - 1] = 0;
                return TRUE;
        }

        return FALSE;
}


#ifdef GIBBON_CLIP_DEBUG_BOARD_STATE
static void
gibbon_clip_dump_board (const gchar *raw,
                        gchar **tokens)
{
        int i = 0;

        g_printerr ("=== Board ===\n");
        g_printerr ("board:%s\n", raw);
        for (i = 0; keys[i]; ++i)
                g_printerr ("%s (%s)\n", keys[i], tokens[i]);
}
#endif

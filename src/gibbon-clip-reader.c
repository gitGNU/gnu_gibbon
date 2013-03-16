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

/**
 * SECTION:gibbon-clip-reader
 * @short_description: Parse FIBS server output.
 *
 * Since: 0.2.0
 *
 * This class pre-processes the output from FIBS and translated it into
 * simple syntax trees.
 */

#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip-reader.h"
#include "gibbon-clip-reader-priv.h"
#include "gibbon-util.h"
#include "gibbon-position.h"

typedef struct _GibbonCLIPReaderPrivate GibbonCLIPReaderPrivate;
struct _GibbonCLIPReaderPrivate {
        void *yyscanner;
        GSList *values;
};

#define GIBBON_CLIP_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderPrivate))

G_DEFINE_TYPE (GibbonCLIPReader, gibbon_clip_reader, G_TYPE_OBJECT)

static gboolean gibbon_clip_reader_alloc_value (GibbonCLIPReader *self,
                                                gchar *token,
                                                enum GibbonCLIPLexerTokenType t);

static void 
gibbon_clip_reader_init (GibbonCLIPReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderPrivate);

        self->priv->yyscanner = NULL;
        self->priv->values = NULL;
}

static void
gibbon_clip_reader_finalize (GObject *object)
{
        GibbonCLIPReader *self = GIBBON_CLIP_READER (object);

        if (self->priv->yyscanner)
                gibbon_clip_lexer_lex_destroy (self->priv->yyscanner);

        G_OBJECT_CLASS (gibbon_clip_reader_parent_class)->finalize(object);
}

static void
gibbon_clip_reader_class_init (GibbonCLIPReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonCLIPReaderPrivate));

        g_value_register_transform_func (
                GIBBON_TYPE_POSITION, G_TYPE_STRING,
                gibbon_position_transform_to_string_value);

        object_class->finalize = gibbon_clip_reader_finalize;
}

/**
 * gibbon_clip_reader_new:
 *
 * Creates a new #GibbonCLIPReader.
 *
 * Returns: The newly created #GibbonCLIPReader or %NULL in case of failure.
 */
GibbonCLIPReader *
gibbon_clip_reader_new ()
{
        GibbonCLIPReader *self = g_object_new (GIBBON_TYPE_CLIP_READER, NULL);

        if (gibbon_clip_lexer_lex_init_extra (self, &self->priv->yyscanner)) {
                g_error (_("Error creating tokenizer: %s!"),
                         strerror (errno));
                /* NOTREACHED */
                return NULL;
        }

        return self;
}

GSList *
gibbon_clip_reader_parse (GibbonCLIPReader *self, const gchar *line)
{
        GSList *retval;
        GValue *value;
        GValue init = G_VALUE_INIT;
        const gchar *ptr;
        gint status;
        gboolean error = FALSE;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), NULL);
        g_return_val_if_fail (line != NULL, NULL);

        gibbon_clip_lexer_current_buffer (self->priv->yyscanner, line);

        while (0 != (status = gibbon_clip_lexer_lex (self->priv->yyscanner))) {
                if (status < 0) {
                        error = TRUE;
                        break;
                } else if (status == 2) {
                        /*
                         * Restart the scanner.  This happens, when we are
                         * leaving one of the multi-line states.
                         */
                        gibbon_clip_lexer_current_buffer (self->priv->yyscanner,
                                                          line);
                }
        }

        if (error) {
                gibbon_clip_reader_free_result (self, self->priv->values);
                self->priv->values = NULL;
                gibbon_clip_lexer_reset_condition_stack (self->priv->yyscanner);

                /*
                 * Was this an error message?
                 */
                if ('*' == line[0] && '*' == line[1]
                    && (' ' == line[2] || '\t' == line[2])) {
                        ptr = line + 3;
                        while (*ptr == ' ' || *ptr == '\t')
                                ++ptr;

                        value = g_malloc (sizeof *value);
                        *value = init;
                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_STRING);
                        g_value_set_string (value, ptr);

                        value = g_malloc (sizeof *value);
                        *value = init;
                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_INT64);
                        g_value_set_int64 (value, GIBBON_CLIP_ERROR_UNKNOWN);

                        value = g_malloc (sizeof *value);
                        *value = init;
                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_INT64);
                        g_value_set_int64 (value, GIBBON_CLIP_ERROR);
                } else {
                        return NULL;
                }
        }

        retval = self->priv->values;
        self->priv->values = NULL;

        return retval;
}

static gboolean
gibbon_clip_reader_alloc_value (GibbonCLIPReader *self,
                                gchar *token,
                                enum GibbonCLIPLexerTokenType type)
{
        GValue *value;
        GValue init = G_VALUE_INIT;
        gint64 i;
        gdouble d;
        size_t length;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (token != NULL, FALSE);

        value = g_malloc (sizeof *value);
        *value = init;

        self->priv->values = g_slist_prepend (self->priv->values, value);

        switch (type) {
        case GIBBON_TT_END:
                /*
                 * We have to initialize the value.  Otherwise, the test
                 * suite will fail.
                 */
                g_value_init (value, G_TYPE_INT64);
                g_return_val_if_fail (type != GIBBON_TT_END, FALSE);
                break;
        case GIBBON_TT_USER:
                g_value_init (value, G_TYPE_STRING);
                if (!g_strcmp0 (token, "You"))
                        return FALSE;
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_MAYBE_YOU:
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_MAYBE_USER:
                g_value_init (value, G_TYPE_STRING);
                if (!g_strcmp0 (token, "-"))
                        token = "";
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_TIMESTAMP:
                g_value_init (value, G_TYPE_INT64);
                i = g_ascii_strtoll (token, NULL, 10);
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_WORD:
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_BOOLEAN:
                g_value_init (value, G_TYPE_BOOLEAN);
                if (token[0] == '0')
                        i = FALSE;
                else if (token[0] == '1')
                        i = TRUE;
                else
                        return FALSE;
                if (token[1])
                        return FALSE;
                g_value_set_boolean (value, i);
                break;
        case GIBBON_TT_N0:
                g_value_init (value, G_TYPE_INT64);
                i = g_ascii_strtoll (token, NULL, 10);
                if (i < 0)
                        return FALSE;
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_POSITIVE:
                g_value_init (value, G_TYPE_INT64);
                i = g_ascii_strtoll (token, NULL, 10);
                if (i < 1)
                        return FALSE;
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_DOUBLE:
                g_value_init (value, G_TYPE_DOUBLE);
                d = g_ascii_strtod (token, NULL);
                g_value_set_double (value, d);
                break;
        case GIBBON_TT_REDOUBLES:
                g_value_init (value, G_TYPE_INT64);
                if (!g_strcmp0 (token, "unlimited"))
                        i = -1;
                else if (!g_strcmp0 (token, "none"))
                        i = 0;
                else
                        i = g_ascii_strtoull (token, NULL, 10);
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_MESSAGE:
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_HOSTNAME:
                g_value_init (value, G_TYPE_STRING);
                length = strlen (token);
                if ('*' == token[length - 1])
                        token[length - 1] = 0;
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_DIE:
                g_value_init (value, G_TYPE_INT64);
                i = token[0] - '0';
                if (token[1] || i < 1 || i > 6)
                        return FALSE;
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_POINT:
                g_value_init (value, G_TYPE_INT64);
                /*
                 * At this point we cannot decide whether off and bar
                 * correspond to 0 or 25.  We will fix that up in a later step
                 * by looking at the adjacent point.
                 */
                if (!g_strcmp0 (token, "off"))
                        i = 0;
                else if (!g_strcmp0 (token, "bar"))
                        i = 0;
                else
                        i = g_ascii_strtoull (token, NULL, 10);
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_CUBE:
                g_value_init (value, G_TYPE_INT64);
                i = g_ascii_strtoll (token, NULL, 10);
                if (i <= 0 || (i & (~i + 1)) != i)
                        return FALSE;
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_MATCH_LENGTH:
                g_value_init (value, G_TYPE_INT64);
                if (!g_strcmp0 ("unlimited", token)) {
                        i = 0;
                } else if (!g_strcmp0 ("resume", token)) {
                        i = -1;
                } else {
                        i = g_ascii_strtoll (token, NULL, 10);
                        if (i < 1)
                                return FALSE;
                }
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_YESNO:
                g_value_init (value, G_TYPE_BOOLEAN);
                if (!g_strcmp0 (token, "YES"))
                        i = TRUE;
                else if (!g_strcmp0 (token, "NO"))
                        i = FALSE;
                else
                        return FALSE;
                g_value_set_boolean (value, i);
                break;
        }

        return TRUE;
}

void
gibbon_clip_reader_free_result (GibbonCLIPReader *self, GSList *values)
{
        if (values) {
                g_slist_foreach (values, (GFunc) g_value_unset, NULL);
                g_slist_foreach (values, (GFunc) g_free, NULL);
                g_slist_free (values);
        }
}

gboolean
gibbon_clip_reader_set_board (GibbonCLIPReader *self, gchar **tokens)
{
        GValue *value;
        GValue init = G_VALUE_INIT;
        GibbonPosition *pos;
        GibbonPositionSide color, turn;
        gboolean direction;
        gboolean post_crawford, no_crawford;
        gint64 numbers[50];
        gint i;
        gchar *end = NULL;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);

        if (52 != g_strv_length (tokens))
                return FALSE;

        errno = 0;
        for (i = 0; i < G_N_ELEMENTS (numbers); ++i) {
                numbers[i] = g_ascii_strtoll (tokens[i + 2], &end, 10);
                if (errno)
                        return FALSE;
                if (!end || *end)
                        return FALSE;
        }

        pos = gibbon_position_new ();

        /*
         * Clear all points.
         */
        memset (pos->points, 0, sizeof pos->points);

        /*
         * No iterate over the list and fill our board structure.
         *
         * The documentation for the data structure is available at
         * http://www.fibs.com/fibs_interface.html#board_state
         */

        pos->players[0] = g_strdup (tokens[0]);
        pos->players[1] = g_strdup (tokens[1]);

        if (numbers[0] < 1)
                goto bail_out_board;
        if (numbers[0] >= 9999)
                numbers[0] = 0;
        pos->match_length = numbers[0];

        if (numbers[1] < 0)
                goto bail_out_board;
        pos->scores[0] = numbers[1];

        if (numbers[2] < 0)
                goto bail_out_board;
        pos->scores[1] = numbers[2];

        /* Color.  */
        if (numbers[38] != -1 && numbers[38] != 1)
                goto bail_out_board;
        color = numbers[38];

        /* Direction.  */
        if (numbers[39] != -1 && numbers[39] != 1)
                goto bail_out_board;
        direction = numbers[39];

        /* Regular points.  */
        if (direction == GIBBON_POSITION_SIDE_BLACK) {
                for (i = 0; i < 24; ++i) {
                        numbers[i + 4] *= color;
                        if (numbers[i + 4] < -15 || numbers[i + 4] > +15)
                                goto bail_out_board;
                        pos->points[i] = numbers[i + 4];
                }
        } else {
                for (i = 0; i < 24; ++i) {
                        numbers[i + 4] *= color;
                        if (numbers[i + 4] < -15 || numbers[i + 4] > +15)
                                goto bail_out_board;
                        pos->points[23 - i] = numbers[i + 4];
                }
        }

        if (numbers[29] < -1 || numbers[29] > 1)
                goto bail_out_board;
        turn = numbers[29];

        if (numbers[30] < 0 || numbers[30] > 6)
                goto bail_out_board;
        if (numbers[31] < 0 || numbers[31] > 6)
                goto bail_out_board;
        if (numbers[32] < 0 || numbers[32] > 6)
                goto bail_out_board;
        if (numbers[33] < 0 || numbers[33] > 6)
                goto bail_out_board;

        /*
         * Translate FIBS' notion to who is on turn to our internal one.
         */
        if (turn == color) {
                pos->turn = GIBBON_POSITION_SIDE_WHITE;
                pos->dice[0] = numbers[30];
                pos->dice[1] = numbers[31];
        } else if (turn) {
                pos->turn = GIBBON_POSITION_SIDE_BLACK;
                pos->dice[0] = numbers[32];
                pos->dice[1] = numbers[33];
        } else {
                pos->turn = GIBBON_POSITION_SIDE_NONE;
        }

        if (numbers[34] < 0
            || (numbers[34] & (~numbers[34] + 1)) != numbers[34])
                goto bail_out_board;
        pos->cube = numbers[34];

        /*
         * May double flags.  These may have to be corrected later.
         */
        if (numbers[35] != 0 && numbers[35] != 1)
                goto bail_out_board;
        pos->may_double[0] = numbers[35];
        if (numbers[36] != 0 && numbers[36] != 1)
                goto bail_out_board;
        pos->may_double[1] = numbers[36];

        /*
         * Cube currently turned? Documented as "Was Doubled".
         */
        if (numbers[37] != 0 && numbers[37] != 1)
                goto bail_out_board;
        pos->cube_turned = numbers[37];

        /*
         * Number of checkers on the bar.
         */
        if (numbers[44] < 0 || numbers[44] > 15)
                goto bail_out_board;
        pos->bar[0] = numbers[44];
        if (numbers[45] < 0 || numbers[45] > 15)
                goto bail_out_board;
        pos->bar[1] = numbers[45];

        /*
         * FIXME: Number of redoubles is ignored.
         */

        /*
         * The next two flags are described incorrectly at the URI mentioned
         * above.
         *
         * The flag called "forced move" there is really a flag
         * indicating that the Crawford rule is active.  Unfortunately,
         * the flag is only set for the Crawford game, or for post-Crawford
         * games.  Before that it is always turned off.  The other oddity
         * which is not limited to 0 and 1 but to 0 and an arbitrary
         * integer.
         *
         * The best strategy for Crawford detection is therefore to always
         * assume that the Crawford rule applies.  And when one opponent
         * is 1-away, check this flag.
         *
         * The flag described as "Did Crawford" is really the post-Crawford
         * flag.  If one opponent is 1-away, and the flag is set, we know
         * that the Crawford rule applies, and that this is a post-Crawford
         * game.
         */
        if (numbers[47] < 0)
                goto bail_out_board;
        no_crawford = (gboolean) numbers[47];
        if (numbers[48] != 0 && numbers[48] != 1)
                goto bail_out_board;
        post_crawford = numbers[48];

        if (!no_crawford && pos->match_length
            && (pos->scores[0] == pos->match_length - 1
                || pos->scores[1] == pos->match_length - 1)) {
                if (post_crawford) {
                        pos->game_info = g_strdup (_("Post-Crawford game"));
                } else {
                        pos->game_info = g_strdup (_("Crawford game"));
                        pos->may_double[0] = pos->may_double[1] = FALSE;
                }
        }

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (NULL, value);
        g_value_init (value, G_TYPE_BOOLEAN);
        g_value_set_boolean (value, direction == -1);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (self->priv->values, value);
        g_value_init (value, GIBBON_TYPE_POSITION);
        g_value_set_boxed (value, pos);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (self->priv->values, value);
        g_value_init (value, G_TYPE_INT64);
        g_value_set_int64 (value, GIBBON_CLIP_BOARD);

        return TRUE;

bail_out_board:
        gibbon_position_free (pos);

        return FALSE;
}

gboolean
gibbon_clip_reader_set_result (GibbonCLIPReader *self, const gchar *yytext,
                               gint max_tokens, const gchar *delimiter,
                               gint clip_code, ...)
{
        va_list args;
        enum GibbonCLIPLexerTokenType type;
        int position;
        gboolean retval = TRUE;

        gchar **tokens = NULL;
        GValue *value;
        GValue init = G_VALUE_INIT;
        gint vector_length = 0;

        g_return_val_if_fail (yytext != NULL, FALSE);
        g_return_val_if_fail (max_tokens >= 0, FALSE);

        if (max_tokens && delimiter) {
                tokens = gibbon_strsplit_set (yytext, delimiter, max_tokens);
                vector_length = g_strv_length (tokens);
        }

        va_start (args, clip_code);

        while ((type = va_arg (args, enum GibbonCLIPLexerTokenType))
               != GIBBON_TT_END) {
                position = va_arg (args, guint);
                if (position < 0)
                        position = vector_length + position;
                if (position > vector_length || position < 0) {
                        retval = FALSE;
                        break;
                }
                if (!gibbon_clip_reader_alloc_value (self, tokens[position],
                                                     type)) {
                        retval = FALSE;
                        break;
                }
        }

        va_end (args);

        if (tokens)
                g_strfreev (tokens);

        if (retval && clip_code) {
                value = g_malloc (sizeof *value);
                *value = init;

                g_value_init (value, G_TYPE_INT64);
                g_value_set_int64 (value, clip_code);
                self->priv->values = g_slist_prepend (self->priv->values,
                                                      value);
        }

        return retval;
}

gboolean
gibbon_clip_reader_append_message (GibbonCLIPReader *self, const gchar *yytext)
{
        GValue *value;
        GValue init = G_VALUE_INIT;

        g_return_val_if_fail (yytext != NULL, FALSE);

        value = g_malloc (sizeof *value);
        *value = init;

        self->priv->values = g_slist_append (self->priv->values, value);

        g_value_init (value, G_TYPE_STRING);
        g_value_set_string (value, yytext);

        return TRUE;
}

void
gibbon_clip_reader_set_error (GibbonCLIPReader *self,
                              enum GibbonCLIPErrorCode code,
                              const gchar *format, ...)
{
        va_list args;
        gchar *msg;
        GValue *value;
        GValue init = G_VALUE_INIT;

        va_start (args, format);
        msg = g_strdup_vprintf (format, args);
        va_end (args);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (self->priv->values, value);
        g_value_init (value, G_TYPE_STRING);
        g_value_set_string (value, msg);
        g_free (msg);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (self->priv->values, value);
        g_value_init (value, G_TYPE_INT64);
        g_value_set_int64 (value, (gint64) code);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (self->priv->values, value);
        g_value_init (value, G_TYPE_INT64);
        g_value_set_int64 (value, (gint64) GIBBON_CLIP_ERROR);
}

gboolean
gibbon_clip_reader_fixup_moves (GibbonCLIPReader *self)
{
        GSList *iter;
        gint i;
        gint64 from, to;
        GValue *vfrom;
        GValue *vto;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);

        iter = self->priv->values;
        g_return_val_if_fail (iter != NULL, FALSE);

        /*
         * Skip GIBBON_CLIP_MOVE and the player name.
         */
        iter = iter->next;
        g_return_val_if_fail (iter != NULL, FALSE);
        iter = iter->next;
        g_return_val_if_fail (iter != NULL, FALSE);

        for (i = 0; iter != NULL && i < 4; ++i, iter = iter->next) {
                g_return_val_if_fail (iter != NULL, FALSE);
                vfrom = (GValue *) iter->data;
                from = g_value_get_int64 (vfrom);

                iter = iter->next;
                g_return_val_if_fail (iter != NULL, FALSE);
                vto = (GValue *) iter->data;
                to = g_value_get_int64 (vto);

                if (from < 0 || to < 0)
                        return FALSE;

                if (from == 0 && to > 18) {
                        from = 25;
                        g_value_set_int64 (vfrom, 25);
                }
                if (to == 0 && from > 18) {
                        to = 25;
                        g_value_set_int64 (vto, 25);
                }
                if (from == to)
                        return FALSE;
                if (from > to && from - to > 6)
                        return FALSE;
                if (to > from && to - from > 6)
                        return FALSE;
        }

        return TRUE;
}

gboolean
gibbon_clip_reader_get_string (const GibbonCLIPReader *self,
                               GSList **iter, const gchar **string)
{
        GValue *value;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (iter != NULL, FALSE);
        g_return_val_if_fail (*iter != NULL, FALSE);
        g_return_val_if_fail (string != NULL, FALSE);

        value = (*iter)->data;
        g_return_val_if_fail (G_IS_VALUE (value), FALSE);
        g_return_val_if_fail (G_VALUE_HOLDS_STRING (value), FALSE);

        *string = g_value_get_string (value);

        *iter = (*iter)->next;

        return TRUE;
}

gboolean
gibbon_clip_reader_get_int (const GibbonCLIPReader *self,
                            GSList **iter, gint *i)
{
        GValue *value;
        gint64 i64;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (iter != NULL, FALSE);
        g_return_val_if_fail (*iter != NULL, FALSE);
        g_return_val_if_fail (i != NULL, FALSE);

        value = (*iter)->data;
        g_return_val_if_fail (G_IS_VALUE (value), FALSE);
        g_return_val_if_fail (G_VALUE_HOLDS_INT64 (value), FALSE);

        i64 = g_value_get_int64 (value);
        g_return_val_if_fail (i64 <= G_MAXINT, FALSE);
        g_return_val_if_fail (i64 >= G_MININT, FALSE);

        *i = (gint) i64;

        *iter = (*iter)->next;

        return TRUE;
}

gboolean
gibbon_clip_reader_get_boolean (const GibbonCLIPReader *self,
                            GSList **iter, gboolean *b)
{
        GValue *value;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (iter != NULL, FALSE);
        g_return_val_if_fail (*iter != NULL, FALSE);
        g_return_val_if_fail (b != NULL, FALSE);

        value = (*iter)->data;
        g_return_val_if_fail (G_IS_VALUE (value), FALSE);
        g_return_val_if_fail (G_VALUE_HOLDS_BOOLEAN (value), FALSE);

        *b = g_value_get_boolean (value);

        *iter = (*iter)->next;

        return TRUE;
}

gboolean
gibbon_clip_reader_get_int64 (const GibbonCLIPReader *self,
                              GSList **iter, gint64 *i64)
{
        GValue *value;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (iter != NULL, FALSE);
        g_return_val_if_fail (*iter != NULL, FALSE);
        g_return_val_if_fail (i64 != NULL, FALSE);

        value = (*iter)->data;
        g_return_val_if_fail (G_IS_VALUE (value), FALSE);
        g_return_val_if_fail (G_VALUE_HOLDS_INT64 (value), FALSE);

        *i64 = g_value_get_int64 (value);

        *iter = (*iter)->next;

        return TRUE;
}

gboolean
gibbon_clip_reader_get_double (const GibbonCLIPReader *self,
                               GSList **iter, gdouble *d)
{
        GValue *value;

        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        g_return_val_if_fail (iter != NULL, FALSE);
        g_return_val_if_fail (*iter != NULL, FALSE);
        g_return_val_if_fail (d != NULL, FALSE);

        value = (*iter)->data;
        g_return_val_if_fail (G_IS_VALUE (value), FALSE);
        g_return_val_if_fail (G_VALUE_HOLDS_DOUBLE (value), FALSE);

        *d = g_value_get_double (value);

        *iter = (*iter)->next;

        return TRUE;
}

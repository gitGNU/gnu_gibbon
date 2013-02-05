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

#include "gibbon-clip.h"
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
                        value = g_malloc (sizeof *value);
                        *value = init;

                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_UINT);
                        g_value_set_uint (value, GIBBON_CLIP_ERROR);

                        value = g_malloc (sizeof *value);
                        *value = init;
                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_UINT);
                        g_value_set_uint (value, GIBBON_CLIP_ERROR_UNKNOWN);

                        ptr = line + 3;
                        while (*ptr == ' ' || *ptr == '\t')
                                ++ptr;
                        value = g_malloc (sizeof *value);
                        *value = init;
                        self->priv->values = g_slist_prepend (
                                        self->priv->values, value);
                        g_value_init (value, G_TYPE_STRING);
                        g_value_set_string (value, ptr);
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
                g_value_init (value, G_TYPE_INT);
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
        case GIBBON_TT_NATURAL:
                g_value_init (value, G_TYPE_UINT64);
                i = g_ascii_strtoull (token, NULL, 10);
                g_value_set_uint64 (value, i);
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
                else
                        i = g_ascii_strtoull (token, NULL, 10);
                g_value_set_int64 (value, i);
                break;
        case GIBBON_TT_MESSAGE:
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, token);
                break;
        case GIBBON_TT_HOSTNAME:
                length = strlen (token);
                if ('*' == token[length - 1])
                        token[length - 1] = 0;
                g_value_init (value, G_TYPE_STRING);
                g_value_set_string (value, token);
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
gibbon_clip_reader_fixup_board (GibbonCLIPReader *self)
{
        GValue *value;
        GValue init = G_VALUE_INIT;
        GibbonPosition *pos;
        GSList *iter = self->priv->values;
        GibbonPositionSide color, turn;
        gboolean direction, cube_turned;
        gboolean post_crawford, no_crawford;
        gint dice[4];
        g_return_val_if_fail (GIBBON_IS_CLIP_READER (self), FALSE);
        gint i, checkers;

        pos = gibbon_position_new ();

        /*
         * Clear all points.
         */
        memset (pos->points, 0, sizeof pos->points);

        /*
         * No iterate over the list and fill our board structure.  Remember
         * that we are walking over the list from the tail.
         *
         * The documentation for the data structure is available at
         * http://www.fibs.com/fibs_interface.html#board_state
         */

        /*
         * Number of redoubles is ignored.
         */

        /*
         * The next to flags are described incorrectly at the URI mentioned
         * above.
         *
         * The flag called "forced move" there is really a flag
         * indicating that the Crawford rule is active.  Unfortunately,
         * the flag is only set for the Crawford game, or for post-Crawford
         * games.  Before that it is always turned off.
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
        iter = iter->next;
        post_crawford = g_value_get_boolean (iter->data);
        iter = iter->next;
        no_crawford = g_value_get_boolean (iter->data);

        /* Skip "Can Move.  */
        iter = iter->next;

        /*
         * Number of checkers on the bar.
         */
        iter = iter->next;
        pos->bar[1] = g_value_get_uint (iter->data);
        iter = iter->next;
        pos->bar[0] = g_value_get_uint (iter->data);

        /*
         * The next four fields are "on home" and "home and bar".  We all
         * ignore them as they are redundant.
         */
        iter = iter->next;
        iter = iter->next;
        iter = iter->next;
        iter = iter->next;

        iter = iter->next;
        direction = g_value_get_int (iter->data);
        iter = iter->next;
        color = g_value_get_int (iter->data);
        iter = iter->next;
        cube_turned = g_value_get_boolean (iter->data);

        /*
         * May double flags.  These may have to be corrected later.
         */
        iter = iter->next;
        pos->may_double[1] = g_value_get_boolean (iter->data);
        iter = iter->next;
        pos->may_double[0] = g_value_get_boolean (iter->data);

        iter = iter->next;
        pos->cube = g_value_get_uint64 (iter->data);

        iter = iter->next;
        dice[3] = g_value_get_int (iter->data);
        iter = iter->next;
        dice[2] = g_value_get_int (iter->data);
        iter = iter->next;
        dice[1] = g_value_get_int (iter->data);
        iter = iter->next;
        dice[0] = g_value_get_int (iter->data);
        if (dice[3] && dice[2]) {
                pos->dice[1] = -dice[3];
                pos->dice[0] = -dice[2];
        } else {
                pos->dice[1] = dice[1];
                pos->dice[0] = dice[0];
        }

        iter = iter->next;
        turn = g_value_get_int (iter->data);

        /* FIBS encodes the bar and home again.  Skip it.  */
        iter = iter->next;

        if (direction == GIBBON_POSITION_SIDE_BLACK) {
                for (i = 23; i >= 0; --i) {
                        iter = iter->next;
                        checkers = g_value_get_int (iter->data);
                        pos->points[i] = color * checkers;
                }
        } else {
                for (i = 0; i < 24; ++i) {
                        iter = iter->next;
                        checkers = g_value_get_int (iter->data);
                        pos->points[i] = color * checkers;
                }
        }

        /* FIBS encodes the bar and home again.  Skip it.  */
        iter = iter->next;

        iter = iter->next;
        pos->scores[1] = g_value_get_uint (iter->data);
        iter = iter->next;
        pos->scores[0] = g_value_get_uint (iter->data);
        iter = iter->next;
        pos->match_length = g_value_get_uint (iter->data);

        iter = iter->next;
        pos->players[1] = g_strdup (g_value_get_string (iter->data));
        iter = iter->next;
        pos->players[0] = g_strdup (g_value_get_string (iter->data));

        /*
         * Translate FIBS' notion to who is on turn to our internal one.
         */
        if (turn == color) {
                pos->turn = GIBBON_POSITION_SIDE_WHITE;
        } else if (turn) {
                pos->turn = GIBBON_POSITION_SIDE_BLACK;
        } else {
                pos->turn = GIBBON_POSITION_SIDE_NONE;
        }

        /*
         * Crawford detection.
         */
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

        /*
         * The old list is now no longer needed.
         */
        gibbon_clip_reader_free_result (self, self->priv->values);

        value = g_malloc (sizeof *value);
        *value = init;
        self->priv->values = g_slist_prepend (NULL, value);
        g_value_init (value, GIBBON_TYPE_POSITION);
        g_value_set_boxed (value, pos);

        return TRUE;
}

gboolean
gibbon_clip_reader_set_result (GibbonCLIPReader *self, const gchar *line,
                               gint max_tokens, gboolean allow_dot,
                               guint clip_code, ...)
{
        va_list args;
        enum GibbonCLIPLexerTokenType type;
        int position;
        gboolean retval = TRUE;
        const gchar *delimiter;

        gchar **tokens = NULL;
        GValue *value;
        GValue init = G_VALUE_INIT;
        gint vector_length = 0;

        g_return_val_if_fail (line != NULL, FALSE);
        g_return_val_if_fail (max_tokens >= 0, FALSE);

        if (max_tokens) {
                if (allow_dot)
                        delimiter = " \t";
                else
                        delimiter = " \t.";
                tokens = g_strsplit_set (line, delimiter, max_tokens);
                vector_length = g_strv_length (tokens);
        }

        va_start (args, clip_code);

        while ((type = va_arg (args, enum GibbonCLIPLexerTokenType))
               != GIBBON_TT_END) {
                position = va_arg (args, guint);
                if (position > vector_length) {
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

        if (retval) {
                value = g_malloc (sizeof *value);
                *value = init;

                g_value_init (value, G_TYPE_UINT);
                g_value_set_uint (value, clip_code);
                self->priv->values = g_slist_prepend (self->priv->values,
                                                      value);
        }

        return retval;
}

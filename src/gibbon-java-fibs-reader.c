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
 * SECTION:gibbon-java-fibs-reader
 * @short_description: Read JavaFIBS internal format.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchReader for reading match files in the JavaFIBS internal
 * format.!
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "gibbon-java-fibs-reader-priv.h"

#include "gibbon-game.h"
#include "gibbon-game-action.h"
#include "gibbon-game-actions.h"

typedef struct _GibbonJavaFIBSReaderPrivate GibbonJavaFIBSReaderPrivate;
struct _GibbonJavaFIBSReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        const gchar *filename;
        void *yyscanner;
        GibbonMatch *match;

        GSList *names;

        gchar *white;
};

#define GIBBON_JAVA_FIBS_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSReader, gibbon_java_fibs_reader, GIBBON_TYPE_MATCH_READER)

static GibbonMatch *gibbon_java_fibs_reader_parse (GibbonMatchReader *match_reader,
                                                   const gchar *filename);
static gboolean gibbon_java_fibs_reader_add_action (GibbonJavaFIBSReader *self,
                                                    const gchar *name,
                                                    GibbonGameAction *action);
static void gibbon_java_fibs_reader_error (const GibbonJavaFIBSReader *self,
                                           const gchar *msg);

static void 
gibbon_java_fibs_reader_init (GibbonJavaFIBSReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_READER, GibbonJavaFIBSReaderPrivate);

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;

        /* Per parser-instance data.  */
        self->priv->filename = NULL;
        self->priv->match = NULL;
        self->priv->names = NULL;
        self->priv->white = NULL;
}

static void
gibbon_java_fibs_reader_finalize (GObject *object)
{
        GibbonJavaFIBSReader *self = GIBBON_JAVA_FIBS_READER (object);

        gibbon_java_fibs_reader_free_names (self);

        G_OBJECT_CLASS (gibbon_java_fibs_reader_parent_class)->finalize(object);
}

static void
gibbon_java_fibs_reader_class_init (GibbonJavaFIBSReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_java_fibs_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonJavaFIBSReaderPrivate));

        object_class->finalize = gibbon_java_fibs_reader_finalize;
}

/**
 * gibbon_java_fibs_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonJavaFIBSReader.
 *
 * Returns: The newly created #GibbonJavaFIBSReader.
 */
GibbonJavaFIBSReader *
gibbon_java_fibs_reader_new (GibbonMatchReaderErrorFunc yyerror,
                             gpointer user_data)
{
        GibbonJavaFIBSReader *self = g_object_new (GIBBON_TYPE_JAVA_FIBS_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;

        return self;
}

static GibbonMatch *
gibbon_java_fibs_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonJavaFIBSReader *self;
        FILE *in;
        int parse_status;
        void *yyscanner;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (_self), NULL);
        self = GIBBON_JAVA_FIBS_READER (_self);

        if (gibbon_java_fibs_lexer_lex_init_extra (self, &yyscanner)) {
                g_error (_("Error creating tokenizer: %s!"),
                         strerror (errno));
                /* NOTREACHED */
                return NULL;
        }

        self->priv->filename = filename;
        self->priv->yyscanner = yyscanner;
        if (self->priv->match)
                g_object_unref (self->priv->match);
        self->priv->match = gibbon_match_new (NULL, NULL, 0, FALSE);
        gibbon_java_fibs_reader_free_names (self);
        g_free (self->priv->white);
        self->priv->white = NULL;

        if (filename)
                in = fopen (filename, "rb");
        else
                in = stdin;
        if (in) {
                gibbon_java_fibs_lexer_set_in (in, yyscanner);

                parse_status = gibbon_java_fibs_parser_parse (yyscanner);

                if (filename)
                        fclose (in);
                if (parse_status) {
                        g_object_unref (self->priv->match);
                        self->priv->match = NULL;
                        g_free (self->priv->white);
                        self->priv->white = NULL;
                }
        } else {
                g_object_unref (self->priv->match);
                self->priv->match = NULL;
                g_free (self->priv->white);
                self->priv->white = NULL;
                gibbon_java_fibs_reader_error (self, strerror (errno));
        }

        self->priv->filename = NULL;
        self->priv->yyscanner = NULL;

        gibbon_java_fibs_lexer_lex_destroy (yyscanner);

        if (self->priv->white && self->priv->match)
                gibbon_match_set_white (self->priv->match, self->priv->white);
        g_free (self->priv->white);
        self->priv->white = NULL;

        return self->priv->match;
}

static void
gibbon_java_fibs_reader_error (const GibbonJavaFIBSReader *self,
                               const gchar *msg)
{
        gchar *full_msg;
        const gchar *filename;
        int lineno;

        if (self->priv->filename)
                filename = self->priv->filename;
        else
                filename = _("[standard input]");

        lineno = gibbon_java_fibs_lexer_get_lineno (self->priv->yyscanner);

        if (lineno)
                full_msg = g_strdup_printf ("%s:%d: %s", filename, lineno, msg);
        else
                full_msg = g_strdup_printf ("%s: %s", filename, msg);

        if (self->priv->yyerror)
                self->priv->yyerror (self->priv->user_data, full_msg);
        else
                g_printerr ("%s\n", full_msg);

        g_free (full_msg);
}

void
gibbon_java_fibs_reader_yyerror (void *scanner, const gchar *msg)
{
        const GibbonJavaFIBSReader *reader;

        reader = (GibbonJavaFIBSReader *) gibbon_java_fibs_lexer_get_extra (scanner);

        if (reader)
                gibbon_java_fibs_reader_error (reader, msg);
}

void
_gibbon_java_fibs_reader_set_white (GibbonJavaFIBSReader *self,
                                    const gchar *white)
{
        g_return_if_fail (GIBBON_IS_JAVA_FIBS_READER (self));
        g_return_if_fail (self->priv->match);

        if (g_strcmp0 (gibbon_match_get_white (self->priv->match), white))
                gibbon_match_set_white (self->priv->match, white);
        if (!self->priv->white)
                self->priv->white = g_strdup (white);
}


void
_gibbon_java_fibs_reader_set_black (GibbonJavaFIBSReader *self,
                                    const gchar *black)
{
        g_return_if_fail (GIBBON_IS_JAVA_FIBS_READER (self));
        g_return_if_fail (self->priv->match);

        if (g_strcmp0 (gibbon_match_get_black (self->priv->match), black))
                gibbon_match_set_black (self->priv->match, black);
}

void
_gibbon_java_fibs_reader_set_match_length (GibbonJavaFIBSReader *self,
                                           gsize length)
{
        g_return_if_fail (GIBBON_IS_JAVA_FIBS_READER (self));
        g_return_if_fail (self->priv->match);

        gibbon_match_set_length (self->priv->match, length);
}

gboolean
_gibbon_java_fibs_reader_add_game (GibbonJavaFIBSReader *self)
{
        GError *error = NULL;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        if (!gibbon_match_add_game (self->priv->match, &error)) {
                gibbon_java_fibs_reader_error (self, error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

gboolean
_gibbon_java_fibs_reader_roll (GibbonJavaFIBSReader *self,
                               const gchar *name,
                               guint die1, guint die2)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (die1, die2));

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

gboolean _gibbon_java_fibs_reader_move (GibbonJavaFIBSReader *self,
                                        const gchar *name,
                                        guint64 encoded)
{
        GibbonMove *move;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        if (encoded & 0xffff000000000000ULL) {
                move = gibbon_move_newv (0, 0,
                                         (encoded & 0xff00000000000000ULL) >> 56,
                                         (encoded & 0x00ff000000000000ULL) >> 48,
                                         (encoded & 0x0000ff0000000000ULL) >> 40,
                                         (encoded & 0x000000ff00000000ULL) >> 32,
                                         (encoded & 0x00000000ff000000ULL) >> 24,
                                         (encoded & 0x0000000000ff0000ULL) >> 16,
                                         (encoded & 0x000000000000ff00ULL) >> 8,
                                         (encoded & 0x00000000000000ffULL),
                                         -1);
        } else if (encoded & 0xffff00000000ULL) {
                move = gibbon_move_newv (0, 0,
                                         (encoded & 0x0000ff0000000000ULL) >> 40,
                                         (encoded & 0x000000ff00000000ULL) >> 32,
                                         (encoded & 0x00000000ff000000ULL) >> 24,
                                         (encoded & 0x0000000000ff0000ULL) >> 16,
                                         (encoded & 0x000000000000ff00ULL) >> 8,
                                         (encoded & 0x00000000000000ffULL),
                                         -1);
        } else if (encoded & 0xffff0000ULL) {
                move = gibbon_move_newv (0, 0,
                                         (encoded & 0x00000000ff000000ULL) >> 24,
                                         (encoded & 0x0000000000ff0000ULL) >> 16,
                                         (encoded & 0x000000000000ff00ULL) >> 8,
                                         (encoded & 0x00000000000000ffULL),
                                         -1);
        } else if (encoded & 0xffffULL) {
                move = gibbon_move_newv (0, 0,
                                         (encoded & 0x000000000000ff00ULL) >> 8,
                                         (encoded & 0x00000000000000ffULL),
                                         -1);
        } else {
                move = gibbon_move_newv (0, 0, -1);
        }

        return gibbon_java_fibs_reader_add_action (self, name,
                                                   GIBBON_GAME_ACTION (move));
}

gboolean
_gibbon_java_fibs_reader_double (GibbonJavaFIBSReader *self,
                                 const gchar *name)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

gboolean
_gibbon_java_fibs_reader_drop (GibbonJavaFIBSReader *self,
                               const gchar *name)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

gboolean
_gibbon_java_fibs_reader_take (GibbonJavaFIBSReader *self,
                               const gchar *name)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

/*
 * We only have to consider this item, after a resignation offer.  Otherwise,
 * we know better, when the game is over.
 */
gboolean
_gibbon_java_fibs_reader_win_game (GibbonJavaFIBSReader *self,
                                   const gchar *name, guint points)
{
        GibbonGameAction *action;
        const GibbonGameAction *last_action;
        GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_java_fibs_reader_error (self, _("Syntax error!"));
                return FALSE;
        }

        last_action = gibbon_game_get_nth_action (game, -1, NULL);
        if (GIBBON_IS_RESIGN (last_action)) {
                action = GIBBON_GAME_ACTION (gibbon_accept_new ());
                return gibbon_java_fibs_reader_add_action (self, name, action);
        }

        /* Otherwise, simply ignore this item.  */
        return TRUE;
}

gboolean
_gibbon_java_fibs_reader_score (GibbonJavaFIBSReader *self,
                                const gchar *winner, guint points_winner,
                                const gchar *loser, guint points_loser)
{
        const gchar *black;
        GibbonMatch *match;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        match = self->priv->match;

        if (self->priv->white && g_strcmp0 ("You", self->priv->white))
                return TRUE;

        black = gibbon_match_get_black (match);
        if (0 == g_strcmp0 (winner, black)) {
                g_free (self->priv->white);
                self->priv->white = g_strdup (loser);
        } else if (0 == g_strcmp0 (loser, black)) {
                g_free (self->priv->white);
                self->priv->white = g_strdup (winner);
        }

        return TRUE;
}

gboolean
_gibbon_java_fibs_reader_resign (GibbonJavaFIBSReader *self,
                                 const gchar *name, guint points)
{
        GibbonGameAction *action;
        const GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);
        g_return_val_if_fail (points != 0, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_java_fibs_reader_error (self, _("Syntax error!"));
                return TRUE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (points));

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

gboolean
_gibbon_java_fibs_reader_reject_resign (GibbonJavaFIBSReader *self,
                                        const gchar *name)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_reject_new ());

        return gibbon_java_fibs_reader_add_action (self, name, action);
}

static gboolean
gibbon_java_fibs_reader_add_action (GibbonJavaFIBSReader *self,
                                    const gchar *name,
                                    GibbonGameAction *action)
{
        GibbonGame *game;
        const GibbonPosition *position;
        GibbonPositionSide side;
        GError *error = NULL;
        GibbonMove *move;
        gsize i;
        const GibbonGameAction *last_action;
        GibbonGameAction *reject;
        gint last_side;

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_java_fibs_reader_error (self, _("No game in progress!"));
                g_object_unref (action);
                return FALSE;
        }

        last_action = gibbon_game_get_nth_action (game, -1, &last_side);
        if (last_action && GIBBON_IS_RESIGN (last_action)
            && !GIBBON_IS_REJECT (action)
            && !GIBBON_IS_ACCEPT (action)) {
                /*
                 * We try to implicitely add the missing rejection.  JavaFIBS
                 * seems to omit it quite often.
                 */
                reject = GIBBON_GAME_ACTION (gibbon_reject_new ());
                if (!gibbon_game_add_action (game, -last_side, reject,
                                             G_MININT64, &error)) {
                        gibbon_java_fibs_reader_error (self, error->message);
                        g_object_unref (reject);
                        g_error_free (error);
                        return FALSE;
                }
        }

        position = gibbon_game_get_position (game);
        if (g_strcmp0 (position->players[0], name))
                side = GIBBON_POSITION_SIDE_BLACK;
        else
                side = GIBBON_POSITION_SIDE_WHITE;

        if (GIBBON_IS_MOVE (action)
            && side == GIBBON_POSITION_SIDE_BLACK) {
                /*
                 * The parser has already translated the move to a move
                 * that corresponds white's direction, that is from higher
                 * points to 0.  If our move is actually for black, we have
                 * to translate it once more.
                 */
                move = GIBBON_MOVE (action);
                for (i = 0; i < move->number; ++i) {
                        move->movements[i].from = -move->movements[i].from + 25;
                        move->movements[i].to = -move->movements[i].to + 25;
                }
        }

        if (!gibbon_game_add_action (game, side, action, G_MININT64, &error)) {
                gibbon_java_fibs_reader_error (self, error->message);
                g_error_free (error);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}

gchar *
gibbon_java_fibs_reader_alloc_name (GibbonJavaFIBSReader *self,
                                     const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_JAVA_FIBS_READER (self), NULL);

        self->priv->names = g_slist_prepend (self->priv->names,
                                             g_strdup (name));

        return self->priv->names->data;
}

void
gibbon_java_fibs_reader_free_names (GibbonJavaFIBSReader *self)
{
        g_return_if_fail (GIBBON_IS_JAVA_FIBS_READER (self));

        g_slist_foreach (self->priv->names, (GFunc) g_free, NULL);
        g_slist_free (self->priv->names);
        self->priv->names = NULL;
}

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
 * SECTION:gibbon-jelly-fish-reader
 * @short_description: Read JellyFish internal format.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchReader for reading match files in the JellyFish internal
 * format.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "gibbon-jelly-fish-reader-priv.h"

#include "gibbon-game.h"
#include "gibbon-game-actions.h"

typedef struct _GibbonJellyFishReaderPrivate GibbonJellyFishReaderPrivate;
struct _GibbonJellyFishReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        const gchar *filename;
        void *yyscanner;
        GibbonMatch *match;

        GSList *names;

        GibbonPositionSide side;
};

GibbonJellyFishReader *_gibbon_jelly_fish_reader_instance = NULL;

#define GIBBON_JELLY_FISH_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JELLY_FISH_READER, GibbonJellyFishReaderPrivate))

G_DEFINE_TYPE (GibbonJellyFishReader, gibbon_jelly_fish_reader, GIBBON_TYPE_MATCH_READER)

static GibbonMatch *gibbon_jelly_fish_reader_parse (GibbonMatchReader *match_reader,
                                                   const gchar *filename);
static gboolean gibbon_jelly_fish_reader_add_action (GibbonJellyFishReader *self,
                                                    GibbonGameAction *action);
static void gibbon_jelly_fish_reader_error (const GibbonJellyFishReader *self,
                                            const gchar *msg);

static void 
gibbon_jelly_fish_reader_init (GibbonJellyFishReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JELLY_FISH_READER, GibbonJellyFishReaderPrivate);

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;

        /* Per parser-instance data.  */
        self->priv->filename = NULL;
        self->priv->match = NULL;
        self->priv->names = NULL;
        self->priv->side = GIBBON_POSITION_SIDE_NONE;
}

static void
gibbon_jelly_fish_reader_finalize (GObject *object)
{
        GibbonJellyFishReader *self = GIBBON_JELLY_FISH_READER (object);

        gibbon_jelly_fish_reader_free_names (self);

        G_OBJECT_CLASS (gibbon_jelly_fish_reader_parent_class)->finalize(object);
}

static void
gibbon_jelly_fish_reader_class_init (GibbonJellyFishReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_jelly_fish_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonJellyFishReaderPrivate));

        object_class->finalize = gibbon_jelly_fish_reader_finalize;
}

/**
 * gibbon_jelly_fish_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonJellyFishReader.
 *
 * Returns: The newly created #GibbonJellyFishReader.
 */
GibbonJellyFishReader *
gibbon_jelly_fish_reader_new (GibbonMatchReaderErrorFunc yyerror,
                              gpointer user_data)
{
        GibbonJellyFishReader *self = g_object_new (GIBBON_TYPE_JELLY_FISH_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;

        return self;
}

static GibbonMatch *
gibbon_jelly_fish_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonJellyFishReader *self;
        FILE *in;
        int parse_status;
        void *yyscanner;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (_self), NULL);
        self = GIBBON_JELLY_FISH_READER (_self);

        if (gibbon_jelly_fish_lexer_lex_init_extra (self, &yyscanner)) {
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
        gibbon_jelly_fish_reader_free_names (self);
        self->priv->side = GIBBON_POSITION_SIDE_NONE;

        if (filename)
                in = fopen (filename, "rb");
        else
                in = stdin;
        if (in) {
                gibbon_jelly_fish_lexer_set_in (in, yyscanner);

                parse_status = gibbon_jelly_fish_parser_parse (yyscanner);
                if (filename)
                        fclose (in);
                if (parse_status) {
                        g_object_unref (self->priv->match);
                        self->priv->match = NULL;
                        self->priv->side = GIBBON_POSITION_SIDE_NONE;
                }
        } else {
                g_object_unref (self->priv->match);
                self->priv->match = NULL;
                self->priv->side = GIBBON_POSITION_SIDE_NONE;
                gibbon_jelly_fish_reader_error (self, strerror (errno));
        }

        self->priv->filename = NULL;
        self->priv->side = GIBBON_POSITION_SIDE_NONE;

        gibbon_jelly_fish_lexer_lex_destroy (yyscanner);

        return self->priv->match;
}

static void
gibbon_jelly_fish_reader_error (const GibbonJellyFishReader *self,
                                const gchar *msg)
{
        gchar *full_msg;
        const gchar *filename;
        int lineno;

        if (self->priv->filename)
                filename = self->priv->filename;
        else
                filename = _("[standard input]");

        lineno = gibbon_jelly_fish_lexer_get_lineno (self->priv->yyscanner);

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
gibbon_jelly_fish_reader_yyerror (void *scanner, const gchar *msg)
{
        const GibbonJellyFishReader *reader;

        reader = (GibbonJellyFishReader *) gibbon_jelly_fish_lexer_get_extra (scanner);

        if (reader)
                gibbon_jelly_fish_reader_error (reader, msg);
}

void
_gibbon_jelly_fish_reader_set_white (GibbonJellyFishReader *self,
                                    const gchar *white)
{
        g_return_if_fail (GIBBON_IS_JELLY_FISH_READER (self));
        g_return_if_fail (self->priv->match);

        if (g_strcmp0 (gibbon_match_get_white (self->priv->match), white))
                gibbon_match_set_white (self->priv->match, white);
}


void
_gibbon_jelly_fish_reader_set_black (GibbonJellyFishReader *self,
                                    const gchar *black)
{
        g_return_if_fail (GIBBON_IS_JELLY_FISH_READER (self));
        g_return_if_fail (self->priv->match);

        if (g_strcmp0 (gibbon_match_get_black (self->priv->match), black))
                gibbon_match_set_black (self->priv->match, black);
}

void
_gibbon_jelly_fish_reader_set_match_length (GibbonJellyFishReader *self,
                                            gsize length)
{
        g_return_if_fail (GIBBON_IS_JELLY_FISH_READER (self));
        g_return_if_fail (self->priv->match);

        /* Unlimited matches are encoded with a zero match length.  */
        if (!length) length = -1;

        gibbon_match_set_length (self->priv->match, length);
}

gboolean
_gibbon_jelly_fish_reader_add_game (GibbonJellyFishReader *self)
{
        GError *error = NULL;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        if (!gibbon_match_add_game (self->priv->match, &error)) {
                gibbon_jelly_fish_reader_error (self, error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

void
_gibbon_jelly_fish_reader_set_side (GibbonJellyFishReader *self,
                                    GibbonPositionSide side)
{
        g_return_if_fail (GIBBON_IS_JELLY_FISH_READER (self));
        g_return_if_fail (self->priv->match);

        self->priv->side = side;
}

gboolean
_gibbon_jelly_fish_reader_move (GibbonJellyFishReader *self,
                                guint64 dice, guint64 encoded)
{
        GibbonMove *move;
        GibbonRoll *roll;
        gsize i;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);
        g_return_val_if_fail (self->priv->side, FALSE);

        roll = gibbon_roll_new (dice / 10, dice % 10);

        if (!gibbon_jelly_fish_reader_add_action (self,
                                                  GIBBON_GAME_ACTION (roll)))
                return FALSE;

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

        if (self->priv->side == GIBBON_POSITION_SIDE_BLACK) {
                /* We have to reverse the move.  */
                for (i = 0; i < move->number; ++i) {
                        move->movements[i].from = -move->movements[i].from + 25;
                        move->movements[i].to = -move->movements[i].to + 25;
                }
        }

        return gibbon_jelly_fish_reader_add_action (self,
                                                    GIBBON_GAME_ACTION (move));
}

gboolean
_gibbon_jelly_fish_reader_double (GibbonJellyFishReader *self)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());

        return gibbon_jelly_fish_reader_add_action (self, action);
}

gboolean
_gibbon_jelly_fish_reader_drop (GibbonJellyFishReader *self)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());

        return gibbon_jelly_fish_reader_add_action (self, action);
}

gboolean
_gibbon_jelly_fish_reader_take (GibbonJellyFishReader *self)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());

        return gibbon_jelly_fish_reader_add_action (self, action);
}

gboolean
_gibbon_jelly_fish_reader_win_game (GibbonJellyFishReader *self,
                                    guint points)
{
        GibbonGame *game;
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_jelly_fish_reader_yyerror (self, _("Syntax error!"));
                return TRUE;
        }

        /*
         * Normally, we know from the previous actions that the game is
         * already over.  We only have to consider resignations which
         * are encoded implicitely.
         */
        if (gibbon_game_over (game))
                return TRUE;

        self->priv->side = -self->priv->side;
        action = GIBBON_GAME_ACTION (gibbon_resign_new (points));
        if (!gibbon_jelly_fish_reader_add_action (self, action))
                return FALSE;
        self->priv->side = -self->priv->side;
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_jelly_fish_reader_add_action (self, action))
                return FALSE;

        return TRUE;
}

gboolean
_gibbon_jelly_fish_reader_resign (GibbonJellyFishReader *self,
                                 const gchar *name, guint points)
{
        GibbonGameAction *action;
        const GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);
        g_return_val_if_fail (points != 0, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_jelly_fish_reader_error (self, _("Syntax error!"));
                return TRUE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (points));

        return gibbon_jelly_fish_reader_add_action (self, action);
}

gboolean
_gibbon_jelly_fish_reader_reject_resign (GibbonJellyFishReader *self,
                                        const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        /* FIXME: Why always return FALSE here? */

        return FALSE;
}

static gboolean
gibbon_jelly_fish_reader_add_action (GibbonJellyFishReader *self,
                                     GibbonGameAction *action)
{
        GibbonGame *game;
        GibbonPositionSide side;
        GError *error = NULL;

        g_return_val_if_fail (self->priv->side, FALSE);

        side = self->priv->side;

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_jelly_fish_reader_error (self,
                                                _("No game in progress!"));
                g_object_unref (action);
                return FALSE;
        }

        if (!gibbon_game_add_action (game, side, action, G_MININT64, &error)) {
                gibbon_jelly_fish_reader_error (self, error->message);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}

gchar *
gibbon_jelly_fish_reader_alloc_name (GibbonJellyFishReader *self,
                                     const gchar *name)
{
        g_return_val_if_fail (GIBBON_IS_JELLY_FISH_READER (self), NULL);

        self->priv->names = g_slist_prepend (self->priv->names,
                                             g_strdup (name));

        return self->priv->names->data;
}

void
gibbon_jelly_fish_reader_free_names (GibbonJellyFishReader *self)
{
        g_return_if_fail (GIBBON_IS_JELLY_FISH_READER (self));

        g_slist_foreach (self->priv->names, (GFunc) g_free, NULL);
        g_slist_free (self->priv->names);
        self->priv->names = NULL;
}

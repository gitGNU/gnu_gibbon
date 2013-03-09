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
 * SECTION:gibbon-game
 * @short_description: Representation of a single game of backgammon in Gibbon!
 *
 * Since: 0.1.1
 *
 * A #GibbonGame represents a single game of backgammon in Gibbon!
 **/

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-game-actions.h"
#include "gibbon-util.h"

typedef struct _GibbonGamePrivate GibbonGamePrivate;
struct _GibbonGamePrivate {
        GibbonPosition *initial_position;
        gboolean edited;

        gsize num_snapshots;
        GibbonGameSnapshot *snapshots;

        gint64 score;
        gboolean resigned;

        gboolean is_crawford;
        gboolean rolled;
};

#define GIBBON_GAME_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME, GibbonGamePrivate))

G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT)

static gboolean gibbon_game_add_roll (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonRoll *roll, gint64 timestamp,
                                      GError **error);
static gboolean gibbon_game_add_move (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonMove *move, gint64 timestamp,
                                      GError **error);
static gboolean gibbon_game_add_double (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDouble *dbl, gint64 timestamp,
                                      GError **error);
static gboolean gibbon_game_add_drop (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDrop *drop, gint64 timestamp,
                                      GError **error);
static gboolean gibbon_game_add_take (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonTake *take, gint64 timestamp,
                                      GError **error);
static gboolean gibbon_game_add_resign (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonResign *resign, gint64 timestamp,
                                        GError **error);
static gboolean gibbon_game_add_reject (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonReject *reject, gint64 timestamp,
                                        GError **error);
static gboolean gibbon_game_add_accept (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonAccept *accept, gint64 timestamp,
                                        GError **error);
static void gibbon_game_add_snapshot (GibbonGame *self,
                                      GibbonGameAction *action,
                                      GibbonPositionSide side,
                                      GibbonPosition *position,
                                      gint64 timestamp);
static const GibbonGameSnapshot *gibbon_game_get_snapshot (const GibbonGame
                                                           *self);
static const GibbonGameSnapshot *gibbon_game_get_nth_snapshot (const GibbonGame
                                                               *self, gint n);

static void 
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME, GibbonGamePrivate);

        self->priv->initial_position = NULL;
        self->priv->edited = FALSE;

        self->priv->snapshots = NULL;
        self->priv->num_snapshots = 0;

        self->priv->score = 0;
        self->priv->resigned = FALSE;

        self->priv->is_crawford = FALSE;
        self->priv->rolled = FALSE;
}

static void
gibbon_game_finalize (GObject *object)
{
        GibbonGame *self = GIBBON_GAME (object);
        gsize i;
        GibbonGameSnapshot *snapshot;

        if (self->priv->initial_position)
                gibbon_position_free (self->priv->initial_position);

        for (i = 0; i < self->priv->num_snapshots; ++i) {
                snapshot = &self->priv->snapshots[i];
                g_object_unref (snapshot->action);
                if (snapshot->analysis)
                        g_object_unref (snapshot->analysis);
                gibbon_position_free (snapshot->resulting_position);
        }
        g_free (self->priv->snapshots);

        G_OBJECT_CLASS (gibbon_game_parent_class)->finalize(object);
}

static void
gibbon_game_class_init (GibbonGameClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGamePrivate));

        object_class->finalize = gibbon_game_finalize;
}

GibbonGame *
gibbon_game_new (GibbonMatch *match, const GibbonPosition *pos,
                 gboolean crawford, gboolean is_crawford)
{
        GibbonGame *self = g_object_new (GIBBON_TYPE_GAME, NULL);

        self->priv->initial_position = gibbon_position_copy (pos);
        self->priv->is_crawford = is_crawford;

        self->priv->initial_position->may_double[0] = FALSE;
        self->priv->initial_position->may_double[1] = FALSE;

        return self;
}

gboolean
gibbon_game_add_action (GibbonGame *self, GibbonPositionSide side,
                        GibbonGameAction *action, gint64 timestamp,
                        GError **error)
{
        const GibbonPosition *current;

        gibbon_return_val_if_fail (GIBBON_IS_GAME (self), FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE,
                                         error);
        gibbon_return_val_if_fail (side == GIBBON_POSITION_SIDE_WHITE
                                         || GIBBON_POSITION_SIDE_BLACK,
                                         FALSE, error);

        current = gibbon_game_get_position (self);
        if (current->resigned) {
                if (!GIBBON_IS_ACCEPT (action) && !GIBBON_IS_REJECT (action)) {
                        g_set_error (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_UNRESPONDED_RESIGNATION,
                                     _("A reply to the resignation is"
                                       " required!"));
                        return FALSE;

                }
        }
        if (current->cube_turned) {
                if (!GIBBON_IS_TAKE (action) && !GIBBON_IS_DROP (action)) {
                        g_set_error (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_UNRESPONDED_DOUBLE,
                                     _("A reply to the double is required!"));
                        return FALSE;

                }
        }

        if (GIBBON_IS_ROLL (action)) {
                return gibbon_game_add_roll (self, side, GIBBON_ROLL (action),
                                             timestamp, error);
        } else if (GIBBON_IS_MOVE (action)) {
                return gibbon_game_add_move (self, side, GIBBON_MOVE (action),
                                             timestamp, error);
        } else if (GIBBON_IS_DOUBLE (action)) {
                return gibbon_game_add_double (self, side,
                                               GIBBON_DOUBLE (action),
                                               timestamp, error);
        } else if (GIBBON_IS_DROP (action)) {
                return gibbon_game_add_drop (self, side,
                                             GIBBON_DROP (action),
                                             timestamp, error);
        } else if (GIBBON_IS_TAKE (action)) {
                return gibbon_game_add_take (self, side,
                                             GIBBON_TAKE (action),
                                             timestamp, error);
        } else if (GIBBON_IS_RESIGN (action)) {
                return gibbon_game_add_resign (self, side,
                                               GIBBON_RESIGN (action),
                                               timestamp, error);
        } else if (GIBBON_IS_REJECT (action)) {
                return gibbon_game_add_reject (self, side,
                                               GIBBON_REJECT (action),
                                               timestamp, error);
        } else if (GIBBON_IS_ACCEPT (action)) {
                return gibbon_game_add_accept (self, side,
                                               GIBBON_ACCEPT (action),
                                               timestamp, error);
        } else {
                /*
                 * We do not bother to translate this message.  It can only
                 * occur if the software itself is broken.
                 */
                g_set_error (error, GIBBON_ERROR,
                             GIBBON_MATCH_ERROR_UNSUPPORTED_ACTION,
                            "gibbon_game_add_action: unsupported action type"
                            " %s!", G_OBJECT_TYPE_NAME (action));
                return FALSE;
        }

        return TRUE;
}

gboolean
gibbon_game_add_action_with_analysis (GibbonGame *self, GibbonPositionSide side,
                                      GibbonGameAction *action,
                                      GibbonAnalysis *analysis,
                                      gint64 timestamp,
                                      GError **error)
{
        GibbonGameSnapshot *snapshot;

        gibbon_return_val_if_fail (GIBBON_IS_GAME (self), FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE,
                                         error);
        gibbon_return_val_if_fail (side == GIBBON_POSITION_SIDE_WHITE
                                         || GIBBON_POSITION_SIDE_BLACK,
                                         FALSE, error);
        if (analysis)
                gibbon_return_val_if_fail (GIBBON_IS_ANALYSIS (analysis),
                                                 FALSE, error);

        if (!gibbon_game_add_action (self, side, action, timestamp, error))
                return FALSE;

        if (!analysis)
                return TRUE;

        snapshot = self->priv->snapshots + self->priv->num_snapshots - 1;
        snapshot->analysis = analysis;

        return TRUE;
}

static gboolean
gibbon_game_add_roll (GibbonGame *self, GibbonPositionSide side,
                      GibbonRoll *roll, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        snapshot = gibbon_game_get_snapshot (self);

        if (snapshot) {
                pos = gibbon_position_copy (snapshot->resulting_position);
        } else {
                pos = gibbon_position_copy (self->priv->initial_position);
        }

        if (side && pos->turn && side != pos->turn) {
                gibbon_position_free (pos);
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_NOT_ON_TURN,
                                     _("This player is not on turn!"));
                return FALSE;
        }

        if (pos->turn) {
                if (pos->dice[0] || pos->dice[1]) {
                        gibbon_position_free (pos);
                        g_set_error_literal (error, GIBBON_ERROR,
                                             GIBBON_MATCH_ERROR_ALREADY_ROLLED,
                                             _("The dice have already been"
                                               " rolled!"));
                        return FALSE;
                }
        } else if (!side) {
                if (roll->die1 > roll->die2)
                        side = GIBBON_POSITION_SIDE_WHITE;
                else if (roll->die1 < roll->die2)
                        side = GIBBON_POSITION_SIDE_BLACK;
        }

        pos->turn = side;
        pos->dice[0] = roll->die1;
        pos->dice[1] = roll->die2;

        if (!self->priv->rolled) {
                self->priv->rolled = TRUE;
                if (!self->priv->is_crawford) {
                        pos->may_double[0] = TRUE;
                        pos->may_double[1] = TRUE;
                }
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (roll), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_move (GibbonGame *self, GibbonPositionSide side,
                      GibbonMove *move, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;
        gchar *pretty_move;

        gibbon_return_val_if_fail (move->number <= 4, FALSE, error);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (!side || side != pos->turn) {
                gibbon_position_free (pos);
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_NOT_ON_TURN,
                                     _("This player is not on turn!"));
                return FALSE;
        }

        if (!pos->dice[0] || !pos->dice[1]) {
                gibbon_position_free (pos);
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_NO_ROLL,
                                     _("Move without a roll!"));
                return FALSE;
        }

        move->die1 = abs (pos->dice[0]);
        move->die2 = abs (pos->dice[1]);

        if (!gibbon_position_apply_move (pos, move, side, FALSE)) {
                gibbon_position_free (pos);
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_NO_ROLL,
                                     _("Invalid move!"));
                return FALSE;
        }

        g_free (pos->status);
        if (move->movements) {
                pretty_move = gibbon_position_format_move (pos, move, side,
                                                           FALSE);
                pos->status = g_strdup_printf (_("%d%d: %s has moved %s."),
                                               move->die1, move->die2,
                                               side
                                               == GIBBON_POSITION_SIDE_BLACK
                                               ? pos->players[1]
                                               : pos->players[0],
                                               pretty_move);
                g_free (pretty_move);
        } else {
                pos->status = g_strdup_printf (_("%d%d: %s cannot move!"),
                                               move->die1, move->die2,
                                               side
                                               == GIBBON_POSITION_SIDE_BLACK
                                               ? pos->players[1]
                                               : pos->players[0]);
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (move), side, pos,
                                  timestamp);

        self->priv->score = gibbon_position_game_over (pos);

        return TRUE;
}

static gboolean
gibbon_game_add_double (GibbonGame *self, GibbonPositionSide side,
                        GibbonDouble *dbl, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot;

        gibbon_return_val_if_fail (self->priv->score == 0, FALSE, error);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (pos->dice[0] || pos->dice[1]) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_DOUBLE_AFTER_ROLL,
                                     _("Double after dice have been rolled!"));
                return FALSE;
        }

        if (!side || side != pos->turn) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_NOT_ON_TURN,
                                     _("This player is not on turn!"));
                return FALSE;
        }

        if (self->priv->is_crawford) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_DOUBLE_CRAWFORD,
                                     _("Double during Crawford game!"));
                return FALSE;
        }

        if ((side > 0 && !pos->may_double[0])
            || (side < 0 && !pos->may_double[1])) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_DOUBLE_NOT_CUBE_OWNER,
                                     _("Double not allowed!"));
                return FALSE;
        }

        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                pos->cube_turned = GIBBON_POSITION_SIDE_WHITE;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[0]);
        } else {
                pos->cube_turned = GIBBON_POSITION_SIDE_BLACK;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[1]);
        }

        /* Beaver? */
        snapshot = gibbon_game_get_snapshot (self);
        if (snapshot && GIBBON_IS_DOUBLE (snapshot->action))
                pos->cube <<= 1;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (dbl), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_drop (GibbonGame *self, GibbonPositionSide side,
                      GibbonDrop *drop, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));
        if (!side || !pos->cube_turned || pos->cube_turned != -side) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_DROP_WITHOUT_DOUBLE,
                                     _("Opponent did not double!"));
                return FALSE;
        }

        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                self->priv->score = -pos->cube;
                pos->scores[1] += pos->cube;
                pos->score = pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[0]);

        } else {
                self->priv->score = pos->cube;
                pos->scores[0] += pos->cube;
                pos->score = -pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[1]);
        }

        pos->cube_turned = GIBBON_POSITION_SIDE_NONE;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (drop), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_take (GibbonGame *self, GibbonPositionSide side,
                      GibbonTake *take, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));
        if (!side || !pos->cube_turned || pos->cube_turned != -side) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_TAKE_WITHOUT_DOUBLE,
                                     _("Opponent did not double!"));
                return FALSE;
        }

        if (side < 0) {
                pos->may_double[0] = FALSE;
                pos->may_double[1] = TRUE;
        } else {
                pos->may_double[0] = TRUE;
                pos->may_double[1] = FALSE;
        }

        pos->cube <<= 1;
        pos->cube_turned = GIBBON_POSITION_SIDE_NONE;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (take), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_resign (GibbonGame *self, GibbonPositionSide side,
                        GibbonResign *resign, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;

        gibbon_return_val_if_fail (side, FALSE, error);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        if (!resign->value) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_EMPTY_RESIGNATION,
                                     _("Resignation without a value!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));
        pos->resigned = side == GIBBON_POSITION_SIDE_BLACK ?
                        -(guint64) resign->value : (guint64) resign->value;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (resign), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_reject (GibbonGame *self, GibbonPositionSide side,
                        GibbonReject *reject, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;
        gchar *player;
        GibbonPositionSide other;
        const GibbonGameSnapshot *snapshot = NULL;

        gibbon_return_val_if_fail (side, FALSE, error);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        other = side == GIBBON_POSITION_SIDE_BLACK ?
                        GIBBON_POSITION_SIDE_WHITE : GIBBON_POSITION_SIDE_BLACK;
        snapshot = gibbon_game_get_snapshot (self);
        if (!snapshot || !GIBBON_IS_RESIGN (snapshot->action)
            || other != snapshot->side) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Reject without resignation!"));
                return FALSE;
        }

        pos->resigned = 0;

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                player = pos->players[1];
        } else {
                player = pos->players[0];
        }

        g_free (pos->status);
        pos->status = g_strdup_printf (_("%s rejects."), player);

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (reject), side, pos,
                                  timestamp);

        return TRUE;
}

static gboolean
gibbon_game_add_accept (GibbonGame *self, GibbonPositionSide side,
                        GibbonAccept *accept, gint64 timestamp, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot = NULL;
        GibbonPositionSide other;
        GibbonResign* resign;

        gibbon_return_val_if_fail (side, FALSE, error);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
                return FALSE;
        }

        other = side == GIBBON_POSITION_SIDE_BLACK ?
                        GIBBON_POSITION_SIDE_WHITE : GIBBON_POSITION_SIDE_BLACK;

        snapshot = gibbon_game_get_snapshot (self);
        if (!snapshot || !GIBBON_IS_RESIGN (snapshot->action)
            || other != snapshot->side) {
                g_set_error_literal (error, GIBBON_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Accept without resignation!"));
                return FALSE;
        }
        resign = GIBBON_RESIGN (snapshot->action);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        g_free (pos->status);
        pos->status = g_strdup_printf (
                        g_dngettext (GETTEXT_PACKAGE,
                                     "%s resigns and gives up one point.",
                                     "%s resigns and gives up %llu points.",
                                     resign->value),
                      other == GIBBON_POSITION_SIDE_BLACK ?
                                      pos->players[0] : pos->players[1],
                      (unsigned long long) pos->cube);

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                pos->scores[1] += resign->value;
                self->priv->score = -resign->value;
                pos->score = resign->value;
        } else {
                pos->scores[0] += resign->value;
                self->priv->score = resign->value;
                pos->score = resign->value;
        }

        self->priv->resigned = TRUE;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (accept), side, pos,
                                  timestamp);

        return TRUE;
}

gint
gibbon_game_over (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self),
                              GIBBON_POSITION_SIDE_NONE);

        return self->priv->score;
}

const GibbonPosition *
gibbon_game_get_position (const GibbonGame *self)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        snapshot = gibbon_game_get_snapshot (self);
        if (snapshot)
                return snapshot->resulting_position;
        else
                return self->priv->initial_position;
}

static void
gibbon_game_add_snapshot (GibbonGame *self, GibbonGameAction *action,
                          GibbonPositionSide side, GibbonPosition *position,
                          gint64 timestamp)
{
        GibbonGameSnapshot *snapshot;

        self->priv->snapshots = g_realloc (self->priv->snapshots,
                                           ++self->priv->num_snapshots
                                           * sizeof *self->priv->snapshots);

        snapshot = self->priv->snapshots + self->priv->num_snapshots - 1;

        snapshot->action = action;
        snapshot->side = side;
        snapshot->resulting_position = position;
        snapshot->analysis = NULL;
        snapshot->timestamp = timestamp;
}

static const GibbonGameSnapshot *
gibbon_game_get_snapshot (const GibbonGame *self)
{
        if (!self->priv->snapshots)
                return NULL;

        return self->priv->snapshots + self->priv->num_snapshots - 1;
}

gboolean
gibbon_game_is_crawford (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), FALSE);

        return self->priv->is_crawford;
}

void gibbon_game_set_is_crawford (GibbonGame *self, gboolean crawford)
{
        g_return_if_fail (GIBBON_IS_GAME (self));

        self->priv->is_crawford = crawford;
}

const GibbonPosition *
gibbon_game_get_initial_position (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        return self->priv->initial_position;
}

GibbonPosition *
gibbon_game_get_initial_position_editable (GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        self->priv->edited = TRUE;

        return self->priv->initial_position;
}

gboolean
gibbon_game_get_edited (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), FALSE);

        return self->priv->edited;
}

const GibbonPosition *
gibbon_game_get_nth_position (const GibbonGame *self, gint n)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        snapshot = gibbon_game_get_nth_snapshot (self, n);

        if (snapshot)
                return snapshot->resulting_position;
        else
                return NULL;
}

GibbonAnalysis *
gibbon_game_get_nth_analysis (const GibbonGame *self, gint n)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        snapshot = gibbon_game_get_nth_snapshot (self, n);

        if (snapshot)
                return snapshot->analysis;
        else
                return NULL;
}

guint64
gibbon_game_get_nth_timestamp (const GibbonGame *self, gint n)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), G_MININT64);

        snapshot = gibbon_game_get_nth_snapshot (self, n);

        if (snapshot)
                return snapshot->timestamp;
        else
                return G_MININT64;
}

void
gibbon_game_set_start_time (GibbonGame *self, gint64 timestamp)
{
        if (self->priv->snapshots)
                self->priv->snapshots->timestamp = timestamp;
}

static const GibbonGameSnapshot *
gibbon_game_get_nth_snapshot (const GibbonGame *self, gint n)
{
        gsize i;

        if (n < 0) {
                i = self->priv->num_snapshots + n;
        } else {
                i = n;
        }

        if (i >= self->priv->num_snapshots)
                return NULL;

        return self->priv->snapshots + i;
}

void
gibbon_game_set_white (GibbonGame *self, const gchar *white)
{
        GibbonPosition *position;
        GibbonGameSnapshot *snapshot;
        gsize i;

        g_return_if_fail (GIBBON_IS_GAME (self));

        position = self->priv->initial_position;
        g_free (position->players[0]);
        position->players[0] = g_strdup (white);

        for (i = 0; i < self->priv->num_snapshots; ++i) {
                snapshot = self->priv->snapshots + i;
                position = snapshot->resulting_position;
                g_free (position->players[0]);
                position->players[0] = g_strdup (white);
        }

        return;
}

void
gibbon_game_set_black (GibbonGame *self, const gchar *black)
{
        GibbonPosition *position;
        GibbonGameSnapshot *snapshot;
        gsize i;

        g_return_if_fail (GIBBON_IS_GAME (self));

        position = self->priv->initial_position;
        g_free (position->players[1]);
        position->players[1] = g_strdup (black);

        for (i = 0; i < self->priv->num_snapshots; ++i) {
                snapshot = self->priv->snapshots + i;
                position = snapshot->resulting_position;
                g_free (position->players[1]);
                position->players[1] = g_strdup (black);
        }

        return;
}

void
gibbon_game_set_match_length (GibbonGame *self, gsize length)
{
        GibbonPosition *position;
        GibbonGameSnapshot *snapshot;
        gsize i;

        g_return_if_fail (GIBBON_IS_GAME (self));

        position = self->priv->initial_position;
        position->match_length = length;

        for (i = 0; i < self->priv->num_snapshots; ++i) {
                snapshot = self->priv->snapshots + i;
                position = snapshot->resulting_position;
                position->match_length = length;
        }

        return;
}

const GibbonGameAction *
gibbon_game_get_nth_action (const GibbonGame *self, gint n,
                            GibbonPositionSide *side)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        snapshot = gibbon_game_get_nth_snapshot (self, n);
        if (!snapshot)
                return NULL;

        if (side)
                *side = snapshot->side;

        return snapshot->action;
}

gboolean
gibbon_game_resignation (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), FALSE);

        return self->priv->resigned;
}

gsize
gibbon_game_get_num_actions (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), 1);

        return self->priv->num_snapshots;
}

void
gibbon_game_set_initial_position (GibbonGame *self,
                                  const GibbonPosition *position)
{
        g_return_if_fail (GIBBON_IS_GAME (self));
        g_return_if_fail (position != NULL);

        if (self->priv->initial_position)
                gibbon_position_free (self->priv->initial_position);
        self->priv->initial_position = gibbon_position_copy (position);
}

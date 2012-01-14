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
 * A #GibbonGame represents a single game of backgammon in Gibbon!  It is
 * always associated with a #GSGFGameTree that is used as a backend.
 **/

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"
#include "gibbon-resign.h"
#include "gibbon-accept.h"
#include "gibbon-reject.h"
#include "gibbon-setup.h"

typedef struct _GibbonGameSnapshot GibbonGameSnapshot;
struct _GibbonGameSnapshot {
        GibbonGameAction *action;
        GibbonPositionSide side;
        GibbonPosition *resulting_position;
};

typedef struct _GibbonGamePrivate GibbonGamePrivate;
struct _GibbonGamePrivate {
        GibbonMatch *match;

        GibbonPosition *initial_position;
        gsize num_snapshots;
        GibbonGameSnapshot *snapshots;

        gint score;

        gboolean is_crawford;
};

#define GIBBON_GAME_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME, GibbonGamePrivate))

G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT)

static gboolean gibbon_game_add_roll (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonRoll *roll, GError **error);
static gboolean gibbon_game_add_move (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonMove *move, GError **error);
static gboolean gibbon_game_add_double (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDouble *dbl, GError **error);
static gboolean gibbon_game_add_drop (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDrop *drop, GError **error);
static gboolean gibbon_game_add_take (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonTake *take, GError **error);
static gboolean gibbon_game_add_resign (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonResign *resign, GError **error);
static gboolean gibbon_game_add_reject (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonReject *reject, GError **error);
static gboolean gibbon_game_add_accept (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonAccept *accept, GError **error);
static gboolean gibbon_game_add_setup (GibbonGame *self,
                                       GibbonPositionSide side,
                                       GibbonSetup *setup, GError **error);
static void gibbon_game_add_snapshot (GibbonGame *self,
                                      GibbonGameAction *action,
                                      GibbonPositionSide side,
                                      GibbonPosition *position);
static const GibbonGameSnapshot *gibbon_game_get_snapshot (const GibbonGame *
                                                           self);
static const GibbonGameSnapshot *gibbon_game_get_nth_snapshot (const GibbonGame
                                                               * self, gint n);

static void 
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME, GibbonGamePrivate);

        self->priv->match = NULL;
        self->priv->initial_position = NULL;
        self->priv->snapshots = NULL;
        self->priv->num_snapshots = 0;

        self->priv->score = 0;

        self->priv->is_crawford = FALSE;
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

        self->priv->match = match;
        self->priv->initial_position = gibbon_position_copy (pos);
        self->priv->is_crawford = is_crawford;

        return self;
}

gboolean
gibbon_game_add_action (GibbonGame *self, GibbonPositionSide side,
                        GibbonGameAction *action, GError **error)
{
        gibbon_match_return_val_if_fail (GIBBON_IS_GAME (self), FALSE, error);
        gibbon_match_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE,
                                         error);
        gibbon_match_return_val_if_fail (side == GIBBON_POSITION_SIDE_WHITE
                                         || GIBBON_POSITION_SIDE_BLACK,
                                         FALSE, error);

        if (GIBBON_IS_ROLL (action)) {
                return gibbon_game_add_roll (self, side, GIBBON_ROLL (action),
                                             error);
        } else if (GIBBON_IS_MOVE (action)) {
                return gibbon_game_add_move (self, side, GIBBON_MOVE (action),
                                             error);
        } else if (GIBBON_IS_DOUBLE (action)) {
                return gibbon_game_add_double (self, side,
                                               GIBBON_DOUBLE (action), error);
        } else if (GIBBON_IS_DROP (action)) {
                return gibbon_game_add_drop (self, side,
                                             GIBBON_DROP (action), error);
        } else if (GIBBON_IS_TAKE (action)) {
                return gibbon_game_add_take (self, side,
                                             GIBBON_TAKE (action), error);
        } else if (GIBBON_IS_RESIGN (action)) {
                return gibbon_game_add_resign (self, side,
                                               GIBBON_RESIGN (action), error);
        } else if (GIBBON_IS_REJECT (action)) {
                return gibbon_game_add_reject (self, side,
                                               GIBBON_REJECT (action), error);
        } else if (GIBBON_IS_ACCEPT (action)) {
                return gibbon_game_add_accept (self, side,
                                               GIBBON_ACCEPT (action), error);
        } else if (GIBBON_IS_SETUP (action)) {
                return gibbon_game_add_setup (self, side,
                                              GIBBON_SETUP (action), error);
        } else {
                /*
                 * We do not bother to translate this message.  It can only
                 * occur if the software itself is broken.
                 */
                g_set_error (error, GIBBON_MATCH_ERROR,
                             GIBBON_MATCH_ERROR_UNSUPPORTED_ACTION,
                            "gibbon_game_add_action: unsupported action type"
                            " %s!", G_OBJECT_TYPE_NAME (action));
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_roll (GibbonGame *self, GibbonPositionSide side,
                      GibbonRoll *roll, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        snapshot = gibbon_game_get_snapshot (self);

        if (snapshot) {
                pos = gibbon_position_copy (snapshot->resulting_position);
        } else {
                pos = gibbon_position_copy (self->priv->initial_position);
        }

        if (side && pos->turn && side != pos->turn) {
                gibbon_position_free (pos);
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_NOT_ON_TURN,
                                     _("This player is not on turn!"));
                return FALSE;
        }
        pos->turn = side;
        pos->dice[0] = roll->die1;
        pos->dice[1] = roll->die2;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (roll), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_move (GibbonGame *self, GibbonPositionSide side,
                      GibbonMove *move, GError **error)
{
        GibbonPosition *pos;
        gchar *pretty_move;

        g_return_val_if_fail (move->number <= 4, FALSE);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (!gibbon_position_apply_move (pos, move, side, FALSE)) {
                gibbon_position_free (pos);
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

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (move), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_double (GibbonGame *self, GibbonPositionSide side,
                        GibbonDouble *dbl, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                pos->cube_turned = GIBBON_POSITION_SIDE_BLACK;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[0]);
        } else {
                pos->cube_turned = GIBBON_POSITION_SIDE_WHITE;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[1]);
        }

        snapshot = gibbon_game_get_snapshot (self);
        if (snapshot && GIBBON_IS_DOUBLE (snapshot->action))
                pos->cube <<= 1;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (dbl), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_drop (GibbonGame *self, GibbonPositionSide side,
                      GibbonDrop *drop, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                self->priv->score = -pos->cube;
                pos->scores[1] += pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[0]);

        } else {
                self->priv->score = pos->cube;
                pos->scores[0] += pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[0]);
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (drop), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_take (GibbonGame *self, GibbonPositionSide side,
                      GibbonTake *take, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));
        pos->cube <<= 1;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (take), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_resign (GibbonGame *self, GibbonPositionSide side,
                        GibbonResign *resign, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (resign), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_reject (GibbonGame *self, GibbonPositionSide side,
                        GibbonReject *reject, GError **error)
{
        GibbonPosition *pos;
        gchar *player;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                player = pos->players[1];
        } else {
                player = pos->players[0];
        }

        g_free (pos->status);
        pos->status = g_strdup_printf (_("%s rejects."), player);

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (reject), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_accept (GibbonGame *self, GibbonPositionSide side,
                        GibbonAccept *accept, GError **error)
{
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot = NULL;
        GibbonPositionSide other;
        GibbonResign* resign;
        gint value;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        other = side == GIBBON_POSITION_SIDE_BLACK ?
                        GIBBON_POSITION_SIDE_WHITE : GIBBON_POSITION_SIDE_BLACK;

        snapshot = gibbon_game_get_snapshot (self);
        if (!snapshot || !GIBBON_IS_RESIGN (snapshot->action)
            || other != snapshot->side) {
                g_warning (_("Accept without resignation!"));
                return FALSE;
        }
        resign = GIBBON_RESIGN (snapshot->action);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        value = resign->value * pos->cube;

        g_free (pos->status);
        pos->status = g_strdup_printf (
                        g_dngettext (GETTEXT_PACKAGE,
                                     "%s resigns and gives up one point.",
                                     "%s resigns and gives up %d points.",
                                     pos->cube),
                      other == GIBBON_POSITION_SIDE_BLACK ?
                                      pos->players[0] : pos->players[1],
                      pos->cube);

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                pos->scores[1] += value;
                self->priv->score = -value;
        } else {
                pos->scores[0] += value;
                self->priv->score = value;
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (accept), side, pos);

        return TRUE;
}

static gboolean
gibbon_game_add_setup (GibbonGame *self, GibbonPositionSide side,
                       GibbonSetup *setup, GError **error)
{
        GibbonPosition *pos;

        if (gibbon_game_over (self)) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_END_OF_GAME,
                                     _("Game is already over!"));
        }

        pos = gibbon_position_copy (setup->position);

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (setup), side, pos);

        /*
         * This is just a safe guess.  If the rest of the code detects that
         * this is actually the Crawford game, we simply set the flag
         * correctly.
         */
        self->priv->is_crawford = FALSE;

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
                          GibbonPositionSide side, GibbonPosition *position)
{
        GibbonGameSnapshot *snapshot;

        self->priv->snapshots = g_realloc (self->priv->snapshots,
                                           ++self->priv->num_snapshots
                                           * sizeof *self->priv->snapshots);

        snapshot = self->priv->snapshots + self->priv->num_snapshots - 1;

        snapshot->action = action;
        snapshot->side = side;
        snapshot->resulting_position = position;
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

const GibbonPosition *
gibbon_game_get_initial_position (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        return self->priv->initial_position;
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

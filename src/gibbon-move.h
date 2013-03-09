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

#ifndef _GIBBON_MOVE_H
# define _GIBBON_MOVE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"
#include "gibbon-movement.h"
#include "gibbon-position.h"

#define GIBBON_TYPE_MOVE \
        (gibbon_move_get_type ())
#define GIBBON_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MOVE, \
                GibbonMove))
#define GIBBON_MOVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MOVE, GibbonMoveClass))
#define GIBBON_IS_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MOVE))
#define GIBBON_IS_MOVE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MOVE))
#define GIBBON_MOVE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MOVE, GibbonMoveClass))

/**
 * GibbonMoveError:
 * @GIBBON_MOVE_LEGAL: legal move
 * @GIBBON_MOVE_ILLEGAL: illegal move
 * @GIBBON_MOVE_TOO_MANY_MOVES: more checkers moved than dice rolled
 * @GIBBON_MOVE_BLOCKED: one of the intermediate landing points was occupied,
 *                       never used here
 * @GIBBON_MOVE_USE_ALL: at least one more checker can be moved
 * @GIBBON_MOVE_USE_HIGHER: in doubt, you must use the higher value
 * @GIBBON_MOVE_TRY_SWAP: two checkers can be moved by swapping the dice order
 * @GIBOBN_MOVE_PREMATURE_BEAR_OFF: checker borne off with checkers outhside
 *                                  home board
 * @GIBBON_MOVE_ILLEGAL_WASTE: move higher before bearing off with waste
 * @GIBBON_MOVE_DANCING: Must come in from the bar first
 *
 * Symbolic constants for different kinds of move errors.
 */
typedef enum {
        GIBBON_MOVE_LEGAL = 0,
        GIBBON_MOVE_ILLEGAL = 1,
        GIBBON_MOVE_TOO_MANY_MOVES = 2,
        GIBBON_MOVE_BLOCKED = 3,
        GIBBON_MOVE_USE_ALL = 4,
        GIBBON_MOVE_USE_HIGHER = 5,
        GIBBON_MOVE_TRY_SWAP = 6,
        GIBBON_MOVE_PREMATURE_BEAR_OFF = 7,
        GIBBON_MOVE_ILLEGAL_WASTE = 8,
        GIBBON_MOVE_DANCING = 9
} GibbonMoveError;

/**
 * GibbonMove:
 *
 * One instance of a #GibbonMove.
 *
 * The properties are public because this used to be a simple structure.  It
 * is now a GObject so that it can inherit from GibbonGameAction.
 *
 * Attention! Do not g_free individual movements.  They are allocated as one
 * single block.
 */
typedef struct _GibbonMove GibbonMove;
struct _GibbonMove
{
        struct _GibbonGameAction parent_instance;

        /*< public >*/
        guint die1;
        guint die2;
        gsize number;
        GibbonMoveError status;
        GibbonMovement *movements;
};

/**
 * GibbonMoveClass:
 *
 * A complete move in Gibbon.
 */
typedef struct _GibbonMoveClass GibbonMoveClass;
struct _GibbonMoveClass
{
        /* <private >*/
        struct _GibbonGameActionClass parent_class;
};

GType gibbon_move_get_type (void) G_GNUC_CONST;

GibbonMove *gibbon_move_new (guint die1, guint die2,
                             gsize num_movements);
GibbonMove *gibbon_move_newv (guint die1, guint die2, ...);

GibbonMove *gibbon_move_copy (const GibbonMove *self);

/*
 * Bring the movements in a consistent order.  The sorting order (ascending
 * vs. descending) depends on the move direction:  Movements with the
 * rearmost checkers are always sorted first.  If two movements use the same
 * starting point, the short mover is sorted before the longer one.
 *
 * the The direction of the move is deduced from the first checker movement.
 */
void gibbon_move_sort (GibbonMove *self);

#endif

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

#ifndef _GIBBON_MATCH_H
# define _GIBBON_MATCH_H

#include <glib.h>
#include <glib-object.h>

#include "gibbon-position.h"

#define GIBBON_TYPE_MATCH \
        (gibbon_match_get_type ())
#define GIBBON_MATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH, \
                GibbonMatch))
#define GIBBON_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH, GibbonMatchClass))
#define GIBBON_IS_MATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH))
#define GIBBON_IS_MATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH))
#define GIBBON_MATCH_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH, GibbonMatchClass))

/**
 * GibbonMatch:
 *
 * One instance of a #GibbonMatch.  All properties are private.
 **/
typedef struct _GibbonMatch GibbonMatch;
struct _GibbonMatch
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchPrivate *priv;
};

/**
 * GibbonMatchClass:
 *
 * Class representing a complete match of backgammon!
 **/
typedef struct _GibbonMatchClass GibbonMatchClass;
struct _GibbonMatchClass
{
        /* <private >*/
        GObjectClass parent_class;
};

/**
 * GibbonMatchError:
 * @GIBBON_MATCH_ERROR_NONE: No error.
 * @GIBBON_MATCH_ERROR_GENERIC: Generic error.
 * @GIBBON_MATCH_ERROR_END_OF_MATCH: Attempt to add match data after end of
 *                                   match.
 * @GIBBON_MATCH_ERROR_END_OF_GAME: Attempt to add game data after end of game.
 * @GIBBON_MATCH_ERROR_UNSUPPORTED_ACTION: An unsupported #GibbonGameAction was
 *                                         encountered.
 * @GIBBON_MATCH_ERROR_NOT_ON_TURN: This player is not on turn.
 * @GIBBON_MATCH_ERROR_NO_ROLL: Move without prior roll.
 *
 * Error codes for the domain #GIBBON_MATCH_ERROR.
 */
typedef enum {
        GIBBON_MATCH_ERROR_NONE = 0,
        GIBBON_MATCH_ERROR_GENERIC,
        GIBBON_MATCH_ERROR_END_OF_MATCH,
        GIBBON_MATCH_ERROR_END_OF_GAME,
        GIBBON_MATCH_ERROR_UNSUPPORTED_ACTION,
        GIBBON_MATCH_ERROR_NOT_ON_TURN,
        GIBBON_MATCH_ERROR_NO_ROLL
} GibbonMatchError;

GType gibbon_match_get_type (void) G_GNUC_CONST;

#define GIBBON_MATCH_ERROR gibbon_match_error_quark ()

GQuark gibbon_match_error_quark (void);

#define gibbon_match_return_val_if_fail(expr, val, error) G_STMT_START{      \
     if G_LIKELY(expr) { } else                                              \
       {                                                                     \
         g_set_error (error, GIBBON_MATCH_ERROR, GIBBON_MATCH_ERROR_GENERIC, \
                      _("In function `%s': assertion `%s' failed."),         \
                      __PRETTY_FUNCTION__, #expr);                           \
         g_return_if_fail_warning (G_LOG_DOMAIN,                             \
                                   __PRETTY_FUNCTION__,                      \
                                   #expr);                                   \
         return (val);                                                       \
       };                               }G_STMT_END


GibbonMatch *gibbon_match_new (const gchar *white, const gchar *black,
                               guint length, gboolean crawford);

gboolean gibbon_match_get_crawford (const GibbonMatch *self);

struct _GibbonGame *gibbon_match_get_current_game (const GibbonMatch
                                                   *self);
gsize gibbon_match_get_number_of_games (const GibbonMatch *self);
struct _GibbonGame *gibbon_match_get_nth_game (const GibbonMatch *self,
                                               gsize i);
const GibbonPosition *gibbon_match_get_current_position (const GibbonMatch *
                                                         self);
struct _GibbonGame *gibbon_match_add_game (GibbonMatch *self, GError **error);

void gibbon_match_set_white (GibbonMatch *self, const gchar *white);
const gchar *gibbon_match_get_white (const GibbonMatch *self);
void gibbon_match_set_black (GibbonMatch *self, const gchar *black);
const gchar *gibbon_match_get_black (const GibbonMatch *self);
void gibbon_match_set_length (GibbonMatch *self, gsize length);
gsize gibbon_match_get_length (const GibbonMatch *self);

#endif

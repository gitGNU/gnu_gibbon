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

#ifndef _GIBBON_GAME_H
# define _GIBBON_GAME_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match.h"
#include "gibbon-game-action.h"
#include "gibbon-position.h"
#include "gibbon-analysis.h"

#define GIBBON_TYPE_GAME \
        (gibbon_game_get_type ())
#define GIBBON_GAME(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GAME, \
                GibbonGame))
#define GIBBON_GAME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GAME, GibbonGameClass))
#define GIBBON_IS_GAME(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GAME))
#define GIBBON_IS_GAME_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GAME))
#define GIBBON_GAME_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GAME, GibbonGameClass))

typedef struct _GibbonGameSnapshot GibbonGameSnapshot;
struct _GibbonGameSnapshot {
        GibbonGameAction *action;
        GibbonPositionSide side;
        GibbonPosition *resulting_position;
        GibbonAnalysis *analysis;
        gint64 timestamp;
};

/**
 * GibbonGame:
 *
 * One instance of a #GibbonGame.  All properties are private.
 **/
typedef struct _GibbonGame GibbonGame;
struct _GibbonGame
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonGamePrivate *priv;
};

/**
 * GibbonGameClass:
 *
 * Class representing one game in a backgammon match!
 **/
typedef struct _GibbonGameClass GibbonGameClass;
struct _GibbonGameClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_game_get_type (void) G_GNUC_CONST;

GibbonGame *gibbon_game_new (GibbonMatch *match,
                             const GibbonPosition *initial_position,
                             gboolean crawford, gboolean is_crawford);
gboolean gibbon_game_add_action (GibbonGame *self, GibbonPositionSide side,
                                 GibbonGameAction *action, gint64 timestamp,
                                 GError **error);
gboolean gibbon_game_add_action_with_analysis (GibbonGame *self,
                                               GibbonPositionSide side,
                                               GibbonGameAction *action,
                                               GibbonAnalysis *analysis,
                                               gint64 timestamp,
                                               GError **error);
gint gibbon_game_over (const GibbonGame *self);
gboolean gibbon_game_resignation (const GibbonGame *self);

const GibbonPosition *gibbon_game_get_position (const GibbonGame *self);

gboolean gibbon_game_is_crawford (const GibbonGame *self);
void gibbon_game_set_is_crawford (GibbonGame *self, gboolean crawford);

const GibbonPosition *gibbon_game_get_initial_position (const GibbonGame *self);

/*
 * Calling this method has the side effect of setting the "edited" flat to
 * TRUE.
 */
GibbonPosition *gibbon_game_get_initial_position_editable (GibbonGame *self);
gboolean gibbon_game_get_edited (const GibbonGame *self);
void gibbon_game_set_initial_position (GibbonGame *self,
                                       const GibbonPosition *position);

/* Yes! N can be negative, think Perl! */
const GibbonPosition *gibbon_game_get_nth_position (const GibbonGame *self,
                                                    gint n);
const GibbonGameAction *gibbon_game_get_nth_action (const GibbonGame *self,
                                                    gint n,
                                                    GibbonPositionSide *side);
GibbonAnalysis *gibbon_game_get_nth_analysis (const GibbonGame *self, gint n);
guint64 gibbon_game_get_nth_timestamp (const GibbonGame *self, gint n);
gsize gibbon_game_get_num_actions (const GibbonGame *self);

void gibbon_game_set_white (GibbonGame *self, const gchar *white);
void gibbon_game_set_black (GibbonGame *self, const gchar *black);
void gibbon_game_set_match_length (GibbonGame *self, gsize length);
void gibbon_game_set_start_time (GibbonGame *self, gint64 timestamp);

#endif

/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBGSGF_RESULT_H
# define _LIBGSGF_RESULT_H

#include <glib.h>

#define GSGF_TYPE_RESULT \
        (gsgf_result_get_type ())
#define GSGF_RESULT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_RESULT, \
                GSGFResult))
#define GSGF_RESULT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_RESULT, GSGFResultClass))
#define GSGF_IS_RESULT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_RESULT))
#define GSGF_IS_RESULT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_RESULT))
#define GSGF_RESULT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_RESULT, GSGFResultClass))

/**
 * GSGFResult:
 *
 * One instance of a #GSGFResult.  All properties are private.
 **/
typedef struct _GSGFResult GSGFResult;
struct _GSGFResult
{
        GSGFSimpleText parent_instance;

        /*< private >*/
        struct _GSGFResultPrivate *priv;
};

/**
 * GSGFResultClass:
 *
 * Representation of a game result!
 **/
typedef struct _GSGFResultClass GSGFResultClass;
struct _GSGFResultClass
{
        /* <private >*/
        GSGFSimpleTextClass parent_class;
};

GType gsgf_result_get_type (void) G_GNUC_CONST;

/**
 * GSGFResultWinner:
 * @GSGF_RESULT_BLACK: Black.
 * @GSGF_RESULT_WHITE: White.
 * @GSGF_RESULT_DRAW: A draw.
 * @GSGF_RESULT_VOID: No result of suspended play.
 * @GSGF_RESULT_UNKNOWN: An unknown result.
 *
 * Constants for the winner of a game.  Note that #GSGFColorEnum is a
 * compatible subset of this enumeration and it is safe to cast from a
 * #GSGFColorEnum to a #GSGFResultWinner but not the other way round.
 */
typedef enum {
        GSGF_RESULT_BLACK = 0,
        GSGF_RESULT_WHITE = 1,
        GSGF_RESULT_DRAW = 2,
        GSGF_RESULT_VOID = 3,
        GSGF_RESULT_UNKNOWN = 32767
} GSGFResultWinner;

/**
 * GSGFResultCause:
 * @GSGF_RESULT_NORMAL: Normal end.
 * @GSGF_RESULT_RESIGNATION: Resignation.
 * @GSGF_RESULT_TIME: Win on time.
 * @GSGF_RESULT_FORFEIT: Forfeit.
 * @GSGF_RESULT_OTHER: Other.
 *
 * Cause for the end of a game.
 */
typedef enum {
        GSGF_RESULT_NORMAL = 0,
        GSGF_RESULT_RESIGNATION = 1,
        GSGF_RESULT_TIME = 2,
        GSGF_RESULT_FORFEIT = 3,
        GSGF_RESULT_OTHER = 32767
} GSGFResultCause;

GSGFResult *gsgf_result_new (GSGFResultWinner winner, gdouble score,
                             GSGFResultCause cause);
GSGFCookedValue *gsgf_result_new_from_raw (const GSGFRaw* raw,
                                           const GSGFFlavor *flavor,
                                           const struct _GSGFProperty *property,
                                           GError **error);
GSGFResultWinner gsgf_result_get_winner (const GSGFResult *self);
GSGFResultCause gsgf_result_get_cause (const GSGFResult *self);
gdouble gsgf_result_get_score (const GSGFResult *self);

#endif

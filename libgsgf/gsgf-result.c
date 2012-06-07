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

/**
 * SECTION:gsgf-result
 * @short_description: A result of a game in SGF!
 * Since: 0.1.0
 *
 * The SGF specification stipulates a certain format for expressing the
 * result of the game.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

#include <errno.h>
#include <math.h>

typedef struct _GSGFResultPrivate GSGFResultPrivate;
struct _GSGFResultPrivate {
        GSGFResultWinner winner;
        gdouble score;
        GSGFResultCause cause;
};

#define GSGF_RESULT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_RESULT, GSGFResultPrivate))

G_DEFINE_TYPE (GSGFResult, gsgf_result, GSGF_TYPE_SIMPLE_TEXT)

static gboolean gsgf_result_set_value (GSGFText *self, const gchar *value,
                                       gboolean copy, GError **error);
static void gsgf_result_sync_text (GSGFResult *self);

static void 
gsgf_result_init (GSGFResult *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_RESULT, GSGFResultPrivate);

        self->priv->winner = GSGF_RESULT_UNKNOWN;
        self->priv->score = 0;
        self->priv->cause = GSGF_RESULT_OTHER;
}

static void
gsgf_result_finalize (GObject *object)
{
        G_OBJECT_CLASS (gsgf_result_parent_class)->finalize(object);
}

static void
gsgf_result_class_init (GSGFResultClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFTextClass *gsgf_text_class = GSGF_TEXT_CLASS (klass);

        gsgf_text_class->set_value = gsgf_result_set_value;
        
        g_type_class_add_private (klass, sizeof (GSGFResultPrivate));

        object_class->finalize = gsgf_result_finalize;
}

/**
 * gsgf_result_new:
 * @winner: The winner of the game.
 * @score: Score or 0 for no score.
 * @cause: Cause for the end of the game.
 *
 * Create a new #GSGFResult.
 *
 * The SGF specification for backgammon defines an extension to the
 * regular result format.  For backgammon you can specifiy a score along
 * with a resignation.
 *
 * This library considers this "extension" as compatible with the regular
 * definition of a result in SGF.  It further extends the definition
 * and also allows scores for wins on time or forfeits.
 *
 * If you want to ensure full compatibility with the SGF specification
 * always give a score of 0 for all irregular ends of a game.
 *
 * In extension to the literal wording of the SGF specification, this library
 * parses results case-insensitive and accepts "T" and "F" as shortcuts
 * for wins on time or forfeit.  When writing back results, they are always
 * correctly spelt.
 *
 * Returns: The new #GSGFResult, the function cannot fail.
 */
GSGFResult *
gsgf_result_new (GSGFResultWinner winner, gdouble score, GSGFResultCause cause)
{
        GSGFResult *self = g_object_new (GSGF_TYPE_RESULT, NULL);

        self->priv->winner = winner;
        self->priv->score = score;
        self->priv->cause = cause;

        /* FIXME! Set the value of GSGFText.  */
        switch (cause) {
                case GSGF_RESULT_NORMAL:
                        break;
                case GSGF_RESULT_RESIGNATION:
                        break;
                case GSGF_RESULT_TIME:
                        break;
                case GSGF_RESULT_FORFEIT:
                        break;
                default:
                        break;
        };

        gsgf_result_sync_text (self);

        return self;
}

static gboolean
gsgf_result_set_value (GSGFText *_self, const gchar *value,
                       gboolean copy, GError **error)
{
        GSGFResult *self = GSGF_RESULT (_self);
        gchar *endptr;
        gboolean stop = FALSE;

        if  (error)
                *error = NULL;

        self->priv->winner = GSGF_RESULT_UNKNOWN;
        self->priv->cause = GSGF_RESULT_OTHER;
        self->priv->score = 0;

        if ('0' == value[0]) {
                self->priv->winner = GSGF_RESULT_DRAW;
                stop = TRUE;
        } else if ('B' == value[0]) {
                self->priv->winner = GSGF_RESULT_BLACK;
                if ('+' == value[1])
                        value += 2;
                else
                        stop = TRUE;
        } else if ('W' == value[0]) {
                self->priv->winner = GSGF_RESULT_WHITE;
                if ('+' == value[1])
                        value += 2;
                else
                        stop = TRUE;
        } else if (!g_ascii_strcasecmp ("Void", value)) {
                self->priv->winner = GSGF_RESULT_VOID;
                stop = TRUE;
        } else {
                self->priv->winner = GSGF_RESULT_UNKNOWN;
                stop = TRUE;
        }

        if (!stop && '0' <= value[0] && '9' >= value[0]) {
                self->priv->score = g_ascii_strtod (value, &endptr);
                if (errno) {
                        self->priv->score = 0;
                        stop = TRUE;
                }
                value = endptr;
        }

        if (stop) {
                /* Do nothing.  */
        } else if (!g_ascii_strcasecmp ("R", value)
            || !g_ascii_strcasecmp ("Resign", value)) {
                self->priv->cause = GSGF_RESULT_RESIGNATION;
        } else if (!g_ascii_strcasecmp ("T", value)
                   || !g_ascii_strcasecmp ("Time", value)) {
                self->priv->cause = GSGF_RESULT_TIME;
        } else if (!g_ascii_strcasecmp ("F", value)
                   || !g_ascii_strcasecmp ("Forfeit", value)) {
                self->priv->cause = GSGF_RESULT_FORFEIT;
        }

        gsgf_result_sync_text (self);

        return TRUE;
}

static void
gsgf_result_sync_text (GSGFResult *self)
{
        const gchar *winner;
        const gchar *cause;
        gchar *score_text;
        gchar *text;
        GSGFTextClass *text_class;
        gchar sign[2];

        switch (self->priv->winner) {
                case GSGF_RESULT_WHITE:
                        winner = "W";
                        break;
                case GSGF_RESULT_BLACK:
                        winner = "B";
                        break;
                case GSGF_RESULT_VOID:
                        winner = "Void";
                        break;
                default:
                        winner = "?";
                        break;
        }

        switch (self->priv->cause) {
                case GSGF_RESULT_RESIGNATION:
                        cause = "Resign";
                        break;
                case GSGF_RESULT_TIME:
                        cause = "Time";
                        break;
                case GSGF_RESULT_FORFEIT:
                        cause = "Forfeit";
                        break;
                case GSGF_RESULT_NORMAL:
                default:
                        cause = "";
                        break;
        }

        if (self->priv->score) {
                sign[0] = self->priv->score > 0 ? '+' : 0;
                sign[1] = 0;
                score_text = gsgf_ascii_dtostring (self->priv->score, -1, -1,
                                                   FALSE, TRUE);
                text = g_strdup_printf ("%s%s%s%s",
                                        winner, sign, score_text, cause);
                g_free (score_text);
        } else
                text = g_strdup_printf ("%s", winner);

        text_class = g_type_class_peek_parent (GSGF_RESULT_GET_CLASS (self));
        text_class->set_value (GSGF_TEXT (self), text, TRUE, NULL);

        g_free (text);
}

/**
 * gsgf_result_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Creates a new #GSGFResult from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFResult or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_result_new_from_raw (const GSGFRaw *raw, const GSGFFlavor *flavor,
                          const GSGFProperty *property, GError **error)
{
        const gchar *string;
        GSGFResult *self;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                             _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value(raw, 0);

        self = g_object_new (GSGF_TYPE_RESULT, NULL);
        if (!gsgf_text_set_value (GSGF_TEXT (self), string, TRUE, error)) {
                g_object_unref (self);
                return NULL;
        }

        return GSGF_COOKED_VALUE (self);
}

/**
 * gsgf_result_get_winner:
 * @self: The #GSGFResult.
 *
 * The winner encoded into @self.
 *
 * Returns: the encoded winner.
 */
GSGFResultWinner
gsgf_result_get_winner (const GSGFResult *self)
{
        g_return_val_if_fail (GSGF_IS_RESULT (self), GSGF_RESULT_UNKNOWN);

        return self->priv->winner;
}

/**
 * gsgf_result_get_cause:
 * @self: The #GSGFResult.
 *
 * The cause of the end-of-game encoded into @self.
 *
 * Returns: the encoded cause.
 */
GSGFResultCause
gsgf_result_get_cause (const GSGFResult *self)
{
        g_return_val_if_fail (GSGF_IS_RESULT (self), GSGF_RESULT_OTHER);

        return self->priv->cause;
}


/**
 * gsgf_result_get_score:
 * @self: The #GSGFResult.
 *
 * The score encoded into @self.
 *
 * Returns: the encoded score.
 */
gdouble
gsgf_result_get_score (const GSGFResult *self)
{
#ifdef NAN
        g_return_val_if_fail (GSGF_IS_RESULT (self), NAN);
#else
        g_return_val_if_fail (GSGF_IS_RESULT (self), 0.0);
#endif

        return self->priv->score;
}

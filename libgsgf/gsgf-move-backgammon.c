/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-move-backgammon
 * @short_description: Definitions for a move in Backgammon
 *
 * Representation of one single move in Backgammon
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFMoveBackgammonPrivate GSGFMoveBackgammonPrivate;

struct _GSGFMoveBackgammonPrivate {
        gint num_moves;
        gint dice[2];
        gint moves[4][2];
};

#define GSGF_MOVE_BACKGAMMON_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                         GSGF_TYPE_MOVE_BACKGAMMON,        \
                                         GSGFMoveBackgammonPrivate))

G_DEFINE_TYPE(GSGFMoveBackgammon, gsgf_move_backgammon, GSGF_TYPE_MOVE)

static gboolean gsgf_move_backgammon_write_stream (const GSGFValue *self,
                                                   GOutputStream *out,
                                                   gsize *bytes_written,
                                                   GCancellable *cancellable,
                                                   GError **error);

static GSGFMoveBackgammon *gsgf_move_backgammon_new_regular(const gchar *string,
                                                           GError **error);
static GSGFMoveBackgammon *gsgf_move_backgammon_new_double();
static GSGFMoveBackgammon *gsgf_move_backgammon_new_take();
static GSGFMoveBackgammon *gsgf_move_backgammon_new_drop();

static void
gsgf_move_backgammon_init(GSGFMoveBackgammon *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GSGF_TYPE_MOVE_BACKGAMMON,
                                                  GSGFMoveBackgammonPrivate);

        (void) memset(self->priv, 0, sizeof (GSGFMoveBackgammonPrivate));

        self->priv->num_moves = -1;
}

static void
gsgf_move_backgammon_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_move_backgammon_parent_class)->finalize(object);
}

static void
gsgf_move_backgammon_class_init(GSGFMoveBackgammonClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *value_class = GSGF_VALUE_CLASS (klass);

        value_class->write_stream = gsgf_move_backgammon_write_stream;

        g_type_class_add_private(klass, sizeof(GSGFMoveBackgammonPrivate));

        object_class->finalize = gsgf_move_backgammon_finalize;
}

/**
 * gsgf_move_backgammon_new:
 *
 * Creates a new #GSGFMoveBackgammon.
 *
 * Returns: The new #GSGFMoveBackgammon.
 */
GSGFMoveBackgammon *
gsgf_move_backgammon_new (void)
{
        GSGFMoveBackgammon *self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        return self;
}

/**
 * gsgf_move_backgammon_new_from_raw:
 * @raw: The #GSGFRaw to parse.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFMoveBackgammon from a #GSGFRaw.
 *
 * Returns: The new #GSGFMoveBackgammon.
 */
GSGFMoveBackgammon *
gsgf_move_backgammon_new_from_raw (const GSGFRaw *raw, GError **error)
{
        const gchar* string;

        g_return_val_if_fail(GSGF_IS_RAW(raw), NULL);

        string = gsgf_raw_get_value(raw, 0);
        if (!string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_MOVE,
                            _("Empty move"));
                return NULL;
        }

        if (string[0] >= '1' && string[1] <= '6') {
                return gsgf_move_backgammon_new_regular(string, error);
        } else if (!strcmp(string, "double")) {
                return gsgf_move_backgammon_new_double();
        } else if (!strcmp(string, "take")) {
                return gsgf_move_backgammon_new_take();
        } else if (!strcmp(string, "drop")) {
                return gsgf_move_backgammon_new_drop();
        }

        g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_MOVE,
                    _("Invalid move syntax '%s'"), string);

        return NULL;
}

static GSGFMoveBackgammon *
gsgf_move_backgammon_new_regular (const gchar *string, GError **error)
{
        GSGFMoveBackgammon *self;
        GSGFMoveBackgammonPrivate *priv;
        gint i;

        self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        priv = self->priv;

        priv->dice[0] = 1 + (gint) string[0] - '1';
        if (string[1] < '1' || string[1] > '6') {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_MOVE,
                                _("Invalid move syntax '%s'"), string);

                return NULL;
        }
        priv->dice[1] = 1 + (gint) string[1] - '1';

        for (i = 2; i < 10; i += 2) {
                if (string[i] < 'a' || string[i] > 'z')
                        break;
                if (string[i + 1] < 'a' || string[i + 1] > 'z')
                        break;
                priv->moves[(i - 2) >> 1][0] = (gint) string[i] - 'a';
                priv->moves[(i - 2) >> 1][1] = (gint) string[i + 1] - 'a';
        }

        if (string[i]) {
                /* Either trailing garbage or illegal value.  */
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_MOVE,
                                _("Invalid move syntax"));
                g_object_unref(self);
                return NULL;
        }

        priv->num_moves = (i - 2) >> 1;

        return self;
}

static GSGFMoveBackgammon *
gsgf_move_backgammon_new_double ()
{
        GSGFMoveBackgammon *self;

        self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        self->priv->dice[0] = 1;

        return self;
}

static GSGFMoveBackgammon *
gsgf_move_backgammon_new_take ()
{
        GSGFMoveBackgammon *self;

        self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        self->priv->dice[0] = 2;

        return self;
}

static GSGFMoveBackgammon *
gsgf_move_backgammon_new_drop ()
{
        GSGFMoveBackgammon *self;

        self = g_object_new(GSGF_TYPE_MOVE_BACKGAMMON, NULL);

        self->priv->dice[0] = 3;

        return self;
}

/**
 * gsgf_move_backgammon_is_regular:
 * @self: The #GSGFMoveBackgammon to check.
 *
 * Checks whether @self is a regular backgammon move.  Other options are
 * a double, a take, or a drop (see gsgf_move_backgammon_is_double(),
 * gsgf_move_backgammon_is_take(), or gsgf_move_backgammon_is_drop()
 * respectively).
 *
 * Returns: #TRUE if @self is a regular backgammon move, #FALSE otherwise.
 */
gboolean
gsgf_move_backgammon_is_regular(const GSGFMoveBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), FALSE);

        return self->priv->num_moves >= 0;
}

/**
 * gsgf_move_backgammon_is_double:
 * @self: The #GSGFMoveBackgammon to check.
 *
 * Checks whether @self is a double in backgammon.  Other options are
 * a regular move, a take, or a drop (see gsgf_move_backgammon_is_regular(),
 * gsgf_move_backgammon_is_take(), or gsgf_move_backgammon_is_drop()
 * respectively).
 *
 * Returns: #TRUE if @self is a regular backgammon move, #FALSE otherwise.
 */
gboolean
gsgf_move_backgammon_is_double(const GSGFMoveBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), FALSE);

        return self->priv->num_moves == -1 && self->priv->dice[0] == 1;
}

/**
 * gsgf_move_backgammon_is_take:
 * @self: The #GSGFMoveBackgammon to check.
 *
 * Checks whether @self is a backgammon take (the affirmative reply to
 * a double.  Other options are a regular move, a take, or a drop (see
 * gsgf_move_backgammon_is_regular(), gsgf_move_backgammon_is_double(), or
 * gsgf_move_backgammon_is_drop() respectively).
 *
 * Returns: #TRUE if @self is a backgammon take, #FALSE otherwise.
 */
gboolean
gsgf_move_backgammon_is_take(const GSGFMoveBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), FALSE);

        return self->priv->num_moves == -1 && self->priv->dice[0] == 2;
}

/**
 * gsgf_move_backgammon_is_drop:
 * @self: The #GSGFMoveBackgammon to check.
 *
 * Checks whether @self is a backgammon drop (the negative reply to
 * a double.  Other options are a regular move, a double, or a take (see
 * gsgf_move_backgammon_is_regular(), gsgf_move_backgammon_is_double(), or
 * gsgf_move_backgammon_is_take() respectively).
 *
 * Returns: #TRUE if @self is a backgammon drop, #FALSE otherwise.
 */
gboolean
gsgf_move_backgammon_is_drop(const GSGFMoveBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), FALSE);

        return self->priv->num_moves == -1 && self->priv->dice[0] == 3;
}

/**
 * gsgf_move_backgammon_get_die:
 * @self: The #GSGFMoveBackgammon to query.
 * @i: The dice number @i.
 *
 * Get the number shown on the first or second die.  That means that @i must
 * be either 0 or 1.
 *
 * Returns: The number on die @i.
 */
guint
gsgf_move_backgammon_get_die(const GSGFMoveBackgammon *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), 0);
        g_return_val_if_fail(self->priv->num_moves >= 0, 0);
        g_return_val_if_fail(i == 0 || i == 1, 0);

        return (gsize) self->priv->dice[i];
}

/**
 * gsgf_move_backgammon_get_num_moves:
 * @self: The #GSGFMoveBackgammon to query.
 *
 * Get the number of (sub) moves in a #GSGFMoveBackgammon.  This value
 * can range from 0 (cannot move for example because of the opponent's
 * closed home board) to 4 (after a double).
 *
 * TODO: Rename to gsgf_move_backgammon_get_sub_move?
 * TODO: Compress moves, i. e. turn 65: 24/18/13 into 65: 24/13.
 *
 * Returns: The number of sub moves executed after a dice roll.
 */
gsize
gsgf_move_backgammon_get_num_moves(const GSGFMoveBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), FALSE);

        return (gsize) self->priv->num_moves;
}

/**
 * gsgf_move_backgammon_get_from:
 * @self: The #GSGFMoveBackgammon to query.
 * @i: The sub move @i.
 *
 * Get the starting point of a move.
 *
 * TODO: Compress moves, i. e. turn 65: 24/18/13 into 65: 24/13.
 *
 * Points are counted with base 0, or 0 is the ace point for white (and the 24
 * point for black), through to 23 which is the 24 point for white (and the ace
 * point for black). 24 is the bar, and 25 is the bearoff tray.
 *
 * Returns: The number associated to the starting point.
 */
guint
gsgf_move_backgammon_get_from(const GSGFMoveBackgammon *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), 0);
        g_return_val_if_fail(self->priv->num_moves > i, 0);
        g_assert(self->priv->num_moves <= 4);

        return (gsize) self->priv->moves[i][0];
}

/**
 * gsgf_move_backgammon_get_to:
 * @self: The #GSGFMoveBackgammon to query.
 * @i: The sub move @i.
 *
 * Get the end point of a move.
 *
 * TODO: Compress moves, i. e. turn 65: 24/18/13 into 65: 24/13.
 *
 * Points are counted with base 0, or 0 is the ace point for white (and the 24
 * point for black), through to 23 which is the 24 point for white (and the ace
 * point for black). 24 is the bar, and 25 is the bearoff tray.
 *
 * Returns: The number associated to the end point.
 */
guint
gsgf_move_backgammon_get_to(const GSGFMoveBackgammon *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_MOVE_BACKGAMMON(self), 0);
        g_return_val_if_fail(self->priv->num_moves > i, 0);
        g_assert(self->priv->num_moves <= 4);

        return (gsize) self->priv->moves[i][1];
}

static gboolean
gsgf_move_backgammon_write_stream (const GSGFValue *_self,
                                   GOutputStream *out, gsize *bytes_written,
                                   GCancellable *cancellable, GError **error)
{
        GSGFMoveBackgammon *self = GSGF_MOVE_BACKGAMMON (_self);
        gchar buffer[2];
        gsize written_here;
        gint i;

        *bytes_written = 0;

        if (gsgf_move_backgammon_is_regular (self)) {
                buffer[0] = '0' + self->priv->dice[0];
                buffer[1] = '0' + self->priv->dice[1];
                if (!g_output_stream_write_all(out, buffer, 2,
                                               &written_here,
                                               cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                for (i = 0; i < self->priv->num_moves; ++i) {
                        buffer[0] = self->priv->moves[i][0] + 'a';
                        buffer[1] = self->priv->moves[i][1] + 'a';
                        if (!g_output_stream_write_all(out, buffer, 2,
                                                       &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                return FALSE;
                        }
                }
        } else if (gsgf_move_backgammon_is_double (self)) {
                if (!g_output_stream_write_all(out, "double", 6,
                                               bytes_written,
                                               cancellable, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_take (self)) {
                if (!g_output_stream_write_all(out, "take", 4,
                                               bytes_written,
                                               cancellable, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_drop (self)) {
                if (!g_output_stream_write_all(out, "drop", 4,
                                               bytes_written,
                                               cancellable, error))
                        return FALSE;
        }

        return TRUE;
}

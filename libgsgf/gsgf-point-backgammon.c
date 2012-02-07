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
 * SECTION:gsgf-point-backgammon
 * @short_description: Definitions for a point in Backgammon
 *
 * Representation of one single point in Backgammon.
 *
 * The class is actually internal.  You will never have to use it yourself.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFPointBackgammonPrivate GSGFPointBackgammonPrivate;
struct _GSGFPointBackgammonPrivate {
        gint point;
};

#define GSGF_POINT_BACKGAMMON_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                         GSGF_TYPE_POINT_BACKGAMMON,        \
                                         GSGFPointBackgammonPrivate))

G_DEFINE_TYPE(GSGFPointBackgammon, gsgf_point_backgammon, GSGF_TYPE_POINT)

static gint _gsgf_point_backgammon_get_point(const GSGFPoint* point);

static gboolean gsgf_point_backgammon_write_stream (const GSGFValue *self,
                                                    GOutputStream *out,
                                                    gsize *bytes_written,
                                                    GCancellable *cancellable,
                                                    GError **error);

static void
gsgf_point_backgammon_init(GSGFPointBackgammon *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                                  GSGF_TYPE_POINT_BACKGAMMON,
                                                  GSGFPointBackgammonPrivate);

        self->priv->point = 0;
}

static void
gsgf_point_backgammon_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_point_backgammon_parent_class)->finalize(object);
}

static void
gsgf_point_backgammon_class_init(GSGFPointBackgammonClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFValueClass *value_class = GSGF_VALUE_CLASS(klass);
        GSGFPointClass *point_class = GSGF_POINT_CLASS (klass);

        value_class->write_stream = gsgf_point_backgammon_write_stream;
        point_class->get_normalized_value = _gsgf_point_backgammon_get_point;

        g_type_class_add_private(klass, sizeof(GSGFPointBackgammonPrivate));

        object_class->finalize = gsgf_point_backgammon_finalize;
}

/**
 * gsgf_point_backgammon_new:
 * @point: The point as a number between 0 and 25.
 *
 * Creates a new #GSGFPointBackgammon.
 *
 * Points are counted with base 0, or 0 is the ace point for white (and the 24
 * point for black), through to 23 which is the 24 point for white (and the ace
 * point for black). 24 is the bar, and 25 is the bearoff tray.
 *
 * Returns: The new #GSGFPointBackgammon.
 */
GSGFPointBackgammon *
gsgf_point_backgammon_new (gint point)
{
        GSGFPointBackgammon *self;

        g_return_val_if_fail(point >= 0 && point <= 25, NULL);

        self = g_object_new(GSGF_TYPE_POINT_BACKGAMMON, NULL);
        self->priv->point = point;

        return self;
}

/**
 * gsgf_point_backgammon_append_to_list_of:
 * @list_of: The #GSGFListOf to append to.
 * @raw: The #GSGFRaw to parse.
 * @i: Parse the ith value in @raw.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Creates a new #GSGFPointBackgammon from a #GSGFRaw.
 *
 * The weird interface to this function is owed to internals of the library.
 * Normally, you will not have to move this function.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_point_backgammon_append_to_list_of (GSGFListOf *list_of, const GSGFRaw *raw,
                                         gsize i, GError **error)
{
        const gchar* string;
        gint from;
        gint to;
        gint tmp;
        gint p;
        GSGFPointBackgammon *point;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);

        string = gsgf_raw_get_value(raw, i);
        if (!string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_POINT,
                            _("Empty point"));
                return FALSE;
        }

        if (string[0] < 'a' || string[0] > 'z') {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_POINT,
                                _("Invalid point syntax"));
                return FALSE;
        }

        from = (gint) (string[0] - 'a');
        if (string[1]) {
                if (':' != string[1]
                    || string[2] < 'a' || string[2] > 'z') {
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_POINT,
                                    _("Invalid point syntax '%s'"), string);
                        return FALSE;
                }

                to = (gint) (string[2] - 'a');

                if (to < from) {
                        tmp = from;
                        from = to;
                        to = tmp;
                }
        } else {
                to = from;
        }

        for (p = from; p <= to; ++p) {
                point = gsgf_point_backgammon_new(p);
                if (!gsgf_list_of_append(list_of, GSGF_COOKED_VALUE(point),
                                         error)) {
                        g_object_unref(point);
                        return FALSE;
                }
        }

        return TRUE;
}

/**
 * gsgf_point_backgammon_new_from_raw:
 * @raw: The #GSGFRaw to parse.
 * @i: The index into @raw.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFPointBackgammon from a #GSGFRaw.
 *
 * Returns: The new #GSGFPointBackgammon.
 */
GSGFPointBackgammon *
gsgf_point_backgammon_new_from_raw (const GSGFRaw *raw, gsize i, GError **error)
{
        const gchar* string;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        string = gsgf_raw_get_value(raw, i);
        if (!string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_STONE,
                            _("Empty stone"));
                return NULL;
        }

        if (string[0] < 'a' || string[0] > 'z' || string[1]) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_STONE,
                                _("Invalid stone syntax"));
                return NULL;
        }

        return gsgf_point_backgammon_new((gint) string[0] - 'a');
}

/**
 * gsgf_point_backgammon_get_point:
 * @self: The #GSGFPointBackgammon to query.
 *
 * Get the number (0-25) associated with a #GSGFPointBackgammon.
 *
 * Returns: The number (0-25) associated with @self.
 */
gint
gsgf_point_backgammon_get_point(const GSGFPointBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_POINT_BACKGAMMON(self), 0);

        return (guint) self->priv->point;
}

gint
_gsgf_point_backgammon_get_point(const GSGFPoint *self)
{
        g_return_val_if_fail(GSGF_IS_POINT_BACKGAMMON(self), 0);

        return (guint) GSGF_POINT_BACKGAMMON(self)->priv->point;
}

static gboolean
gsgf_point_backgammon_write_stream (const GSGFValue *_self,
                                    GOutputStream *out, gsize *bytes_written,
                                    GCancellable *cancellable, GError **error)
{
        GSGFPointBackgammon *self = GSGF_POINT_BACKGAMMON (_self);
        gchar buf;

        *bytes_written = 0;

        buf = self->priv->point + 'a';
        if (!g_output_stream_write_all(out, &buf, 1,
                                       bytes_written,
                                       cancellable, error))
                return FALSE;

        return TRUE;
}

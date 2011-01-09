/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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
 * Representation of one single point in Backgammon
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFPointBackgammonPrivate {
        gint point;
};

#define GSGF_POINT_BACKGAMMON_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                         GSGF_TYPE_POINT_BACKGAMMON,        \
                                         GSGFPointBackgammonPrivate))

G_DEFINE_TYPE(GSGFPointBackgammon, gsgf_point_backgammon, GSGF_TYPE_POINT)

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
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFPointBackgammonPrivate));

        /* FIXME: write_stream() must be implemented! */

        object_class->finalize = gsgf_point_backgammon_finalize;
}

/**
 * gsgf_point_backgammon_new:
 *
 * Creates a new #GSGFPointBackgammon.
 *
 * Returns: The new #GSGFPointBackgammon.
 */
GSGFPointBackgammon *
gsgf_point_backgammon_new (gint point)
{
        GSGFPointBackgammon *self = g_object_new(GSGF_TYPE_POINT_BACKGAMMON, NULL);

        self->priv->point = point;

        return self;
}

/**
 * gsgf_point_backgammon_new_from_raw:
 * @raw: The #GSGFRaw to parse.
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

        g_return_val_if_fail(GSGF_IS_RAW(raw), NULL);

        string = gsgf_raw_get_value(raw, i);
        if (!string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_POINT,
                            _("Empty point"));
                return NULL;
        }

        if (string[0] < 'a' || string[0] > 'z' || string[1]) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_POINT,
                                _("Invalid point syntax"));
                return NULL;
        }

        return gsgf_point_backgammon_new((gint) string[0] - 'a');
}

gint
gsgf_point_backgammon_get_point(const GSGFPointBackgammon *self)
{
        g_return_val_if_fail(GSGF_IS_POINT_BACKGAMMON(self), 0);

        return (guint) self->priv->point;
}

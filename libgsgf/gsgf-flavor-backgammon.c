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
 * SECTION:gsgf-flavor-backgammon
 * @short_description: Definitions for the Backgammon flavor of SGF
 *
 * The Backgammon flavor of SGF as defined for GNU Backgammon.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

G_DEFINE_TYPE(GSGFFlavorBackgammon, gsgf_flavor_backgammon, GSGF_TYPE_FLAVOR)

static GSGFMove *gsgf_flavor_backgammon_create_move (const GSGFFlavor *flavor,
                                                     const struct _GSGFRaw *raw,
                                                     GError **error);
static GSGFStone *gsgf_flavor_backgammon_create_stone (const GSGFFlavor *flavor,
                                                       const struct _GSGFRaw *raw,
                                                       gsize i,
                                                       GError **error);
static GSGFPoint *gsgf_flavor_backgammon_create_point (const GSGFFlavor *flavor,
                                                       const struct _GSGFRaw *raw,
                                                       gsize i,
                                                       GError **error);
static gboolean gsgf_flavor_backgammon_append_points (const GSGFFlavor *flavor,
                                                      GSGFListOf *list_of,
                                                      const struct _GSGFRaw *raw,
                                                      gsize i,
                                                      GError **error);
static gboolean gsgf_flavor_backgammon_write_compressed_list (const GSGFFlavor
                                                              *flavor,
                                   const struct _GSGFListOf *list_of,
                                   GOutputStream *out,
                                   gsize *bytes_written,
                                   GCancellable *cancellable,
                                   GError **error);

static void
gsgf_flavor_backgammon_init(GSGFFlavorBackgammon *self)
{
}

static void
gsgf_flavor_backgammon_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_flavor_backgammon_parent_class)->finalize(object);
}

static void
gsgf_flavor_backgammon_class_init(GSGFFlavorBackgammonClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFFlavorClass *flavor_class = GSGF_FLAVOR_CLASS(klass);

        flavor_class->create_move = gsgf_flavor_backgammon_create_move;
        flavor_class->stone_type = GSGF_TYPE_STONE_BACKGAMMON;
        flavor_class->create_stone = gsgf_flavor_backgammon_create_stone;
        flavor_class->point_type = GSGF_TYPE_POINT_BACKGAMMON;
        flavor_class->create_point = gsgf_flavor_backgammon_create_point;
        flavor_class->append_points = gsgf_flavor_backgammon_append_points;
        flavor_class->write_compressed_list =
                        gsgf_flavor_backgammon_write_compressed_list;

        object_class->finalize = gsgf_flavor_backgammon_finalize;
}

/**
 * gsgf_flavor_backgammon_new:
 *
 * Creates a new #GSGFFlavorBackgammon.
 *
 * Returns: The new #GSGFFlavorBackgammon as a #GSGFFlavor.
 */
GSGFFlavor *
gsgf_flavor_backgammon_new (void)
{
        GSGFFlavorBackgammon *self = g_object_new(GSGF_TYPE_FLAVOR_BACKGAMMON, NULL);

        return GSGF_FLAVOR(self);
}

static GSGFMove *
gsgf_flavor_backgammon_create_move (const GSGFFlavor *flavor,
                                    const GSGFRaw *raw,
                                    GError **error)
{
        GSGFMoveBackgammon *result = gsgf_move_backgammon_new_from_raw(raw, error);

        if (!result)
                return NULL;

        return GSGF_MOVE(result);
}

static GSGFStone *
gsgf_flavor_backgammon_create_stone (const GSGFFlavor *flavor,
                                     const GSGFRaw *raw,
                                     gsize i,
                                     GError **error)
{
        GSGFStoneBackgammon *result = gsgf_stone_backgammon_new_from_raw(raw, i, error);

        if (!result)
                return NULL;

        return GSGF_STONE(result);
}

static GSGFPoint *
gsgf_flavor_backgammon_create_point (const GSGFFlavor *flavor,
                                     const GSGFRaw *raw,
                                     gsize i,
                                     GError **error)
{
        GSGFPointBackgammon *result = gsgf_point_backgammon_new_from_raw(raw, i, error);

        if (!result)
                return NULL;

        return GSGF_POINT(result);
}

static gboolean
gsgf_flavor_backgammon_append_points (const GSGFFlavor *flavor,
                                      GSGFListOf *list_of,
                                      const GSGFRaw *raw,
                                      gsize i,
                                      GError **error)
{
        return gsgf_point_backgammon_append_to_list_of(list_of, raw, i, error);
}

static gboolean
gsgf_flavor_backgammon_write_compressed_list (const GSGFFlavor *flavor,
                                   const struct _GSGFListOf *list_of,
                                   GOutputStream *out,
                                   gsize *bytes_written,
                                   GCancellable *cancellable,
                                   GError **error)
{
        gsize written_here;
        gsize i, num_items;
        gint last_point = -1;
        gint this_point = -1;
        GSGFPointBackgammon *point;
        gchar buffer[5];
        gsize pending = 0;
        *bytes_written = 0;

        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (flavor), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_LIST_OF (list_of), FALSE, error);
        gsgf_return_val_if_fail (GSGF_TYPE_POINT_BACKGAMMON ==
                        gsgf_list_of_get_item_type (list_of), FALSE, error);

        num_items = gsgf_list_of_get_number_of_items (list_of);
        if (!num_items)
                return TRUE;

        for (i = 0; i < num_items; ++i) {
                point = GSGF_POINT_BACKGAMMON (gsgf_list_of_get_nth_item (
                                list_of, i));
                this_point = gsgf_point_backgammon_get_point (point);
                if (last_point < 0) {
                        *buffer = 'a' + this_point;
                        if (!g_output_stream_write_all(out, buffer, 1,
                                                       &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                return FALSE;
                        }
                        pending = 0;
                } else if (this_point != last_point + 1) {
                        buffer[0] = ':';
                        buffer[1] = 'a' + last_point;
                        buffer[2] = ']';
                        buffer[3] = '[';
                        buffer[4] = 'a' + this_point;
                        if (!g_output_stream_write_all(out, buffer, 5,
                                                       &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                return FALSE;
                        }
                        pending = 0;
                } else {
                        ++pending;
                }
                last_point = this_point;
        }

        if (pending) {
                buffer[0] = ':';
                buffer[1] = 'a' + last_point;
                if (!g_output_stream_write_all(out, buffer, 2,
                                               &written_here,
                                               cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
        }

        return TRUE;
}

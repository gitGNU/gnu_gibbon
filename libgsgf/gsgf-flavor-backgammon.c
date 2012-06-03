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
static guint gsgf_flavor_backgammon_get_game_id (const GSGFFlavor *self);
static gboolean gsgf_flavor_backgammon_get_cooked_value (const GSGFFlavor *self,
                                                         const GSGFProperty
                                                             *property,
                                                         const GSGFRaw *raw,
                                                         GSGFCookedValue
                                                              **cooked,
                                                         GError **error);
static GSGFSimpleText *gsgf_flavor_backgammon_cube_position (
                const GSGFFlavorBackgammon *self,  const GSGFRaw *raw,
                GError **error);
static GSGFNumber *gsgf_flavor_backgammon_cube_value (
                const GSGFFlavorBackgammon *self,  const GSGFRaw *raw,
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

        flavor_class->get_cooked_value =
                        gsgf_flavor_backgammon_get_cooked_value;
        flavor_class->create_move = gsgf_flavor_backgammon_create_move;
        flavor_class->stone_type = GSGF_TYPE_STONE_BACKGAMMON;
        flavor_class->create_stone = gsgf_flavor_backgammon_create_stone;
        flavor_class->point_type = GSGF_TYPE_POINT_BACKGAMMON;
        flavor_class->create_point = gsgf_flavor_backgammon_create_point;
        flavor_class->append_points = gsgf_flavor_backgammon_append_points;
        flavor_class->write_compressed_list =
                        gsgf_flavor_backgammon_write_compressed_list;
        flavor_class->get_game_id = gsgf_flavor_backgammon_get_game_id;

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
        GSGFFlavorBackgammon *self = g_object_new (GSGF_TYPE_FLAVOR_BACKGAMMON,
                                                   NULL);

        return GSGF_FLAVOR(self);
}

static GSGFMove *
gsgf_flavor_backgammon_create_move (const GSGFFlavor *flavor,
                                    const GSGFRaw *raw,
                                    GError **error)
{
        GSGFMoveBackgammon *result = gsgf_move_backgammon_new_from_raw (raw,
                                                                        error);

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
        GSGFStoneBackgammon *result = gsgf_stone_backgammon_new_from_raw (
                        raw, i, error);

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
        GSGFPointBackgammon *result = gsgf_point_backgammon_new_from_raw (
                        raw, i, error);

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

static guint
gsgf_flavor_backgammon_get_game_id (const GSGFFlavor *self)
{
        return 6;
}

static gboolean
gsgf_flavor_backgammon_get_cooked_value (const GSGFFlavor *_self,
                                         const GSGFProperty *property,
                                         const GSGFRaw *raw,
                                         GSGFCookedValue **cooked,
                                         GError **error)
{
        const gchar *id;
        gpointer result;
        GSGFFlavorBackgammon *self;

        gsgf_return_val_if_fail (cooked, FALSE, error);

        self = GSGF_FLAVOR_BACKGAMMON (_self);
        *cooked = NULL;

        id = gsgf_property_get_id (property);
        if ('M' == id[0]) {
                if ('I' == id[1] && !id[2]) {
                        result = gsgf_backgammon_match_info_new_from_raw (raw,
                                       _self, property, error);
                        if (!result)
                                return FALSE;
                        *cooked = GSGF_COOKED_VALUE (result);
                        return TRUE;
                }
        } else if ('C' == id[0]) {
                if ('O' == id[1] && !id[2]) {
                        result = gsgf_flavor_backgammon_cube_position (self,
                                                                       raw,
                                                                       error);
                        if (!result)
                                return FALSE;
                        *cooked = GSGF_COOKED_VALUE (result);
                        return TRUE;
                } if ('V' == id[1] && !id[2]) {
                        result = gsgf_flavor_backgammon_cube_value (self, raw,
                                                                    error);
                        if (!result)
                                return FALSE;
                        *cooked = GSGF_COOKED_VALUE (result);
                        return TRUE;
                }
        }

        return GSGF_FLAVOR_CLASS (gsgf_flavor_backgammon_parent_class)
                        ->get_cooked_value (_self, property, raw, cooked,
                                            error);
}

static GSGFSimpleText *
gsgf_flavor_backgammon_cube_position (const GSGFFlavorBackgammon *self,
                                      const GSGFRaw *raw, GError **error)
{
        GSGFSimpleText *retval;
        const gchar *raw_value;

        if (1 < gsgf_raw_get_number_of_values (raw)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Property CO must be single-valued!"));
                return FALSE;
        }

        raw_value = gsgf_raw_get_value (raw, 0);
        if (('b' != raw_value[0] && 'w' != raw_value[0]
             && 'c' != raw_value[0] && 'n' != raw_value[0])
            || raw_value[1]) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Invalid cube position (CO) `%s'!"),
                             raw_value);
                return FALSE;
        }
        retval = gsgf_simple_text_new (raw_value);

        return retval;
}

static GSGFNumber *
gsgf_flavor_backgammon_cube_value (const GSGFFlavorBackgammon *self,
                                   const GSGFRaw *raw, GError **error)
{
        GSGFCookedValue *retval;
        guint64 cube;

        retval = gsgf_number_new_from_raw (raw, GSGF_FLAVOR (self), NULL,
                                           error);
        if (!retval)
                return FALSE;

        cube = (guint64) gsgf_number_get_value (GSGF_NUMBER (retval));

        if (!cube || (cube & (cube - 1))) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Cube value (CV[%llu]) must be a power of two"
                               " greater than one!"),
                             (unsigned long long) cube);
                g_object_unref (retval);
                return FALSE;
        }

        return GSGF_NUMBER (retval);
}

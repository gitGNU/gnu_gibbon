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

/**
 * SECTION:gsgf-value
 * @short_description: Abstract base class for the value of a #GSGFProperty!
 * Since: 0.1.0
 *
 * A #GSGFValue encapsulates the value of a property in SGF data.  It is the
 * common, abstract base class of raw, unqualified data stored in a #GSGFRaw,
 * or a #GSGFCookedValue which is the result of successfully cooking such
 * raw, unqualified data.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

G_DEFINE_TYPE (GSGFValue, gsgf_value, G_TYPE_OBJECT)

static void 
gsgf_value_init (GSGFValue *self)
{
}

static void
gsgf_value_finalize (GObject *object)
{
        G_OBJECT_CLASS (gsgf_value_parent_class)->finalize(object);
}

static void
gsgf_value_class_init (GSGFValueClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        klass->write_stream = NULL;

        object_class->finalize = gsgf_value_finalize;
}

/**
 * gsgf_value_write_stream:
 * @self: The #GSGFValue
 * @out: The #GOutputStream to write to.
 * @bytes_written: Location to store the number of bytes written or %NULL.
 * @cancellable: Optional #GCancellable object or %NULL.
 * @error: Optional #GError location or %NULL to ignore.
 *
 * Serialize a #GSGFValue into a #GOutputStream.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_value_write_stream (const GSGFValue *self,
                         GOutputStream *out, gsize *bytes_written,
                         GCancellable *cancellable, GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_VALUE (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_VALUE_GET_CLASS (self)->write_stream,
                                 FALSE, error);

        return GSGF_VALUE_GET_CLASS(self)->write_stream (self,
                                                         out, bytes_written,
                                                         cancellable, error);
}

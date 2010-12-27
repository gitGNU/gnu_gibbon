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
 * SECTION:gsgf-cooked-value
 * @short_description: Abstract base class for values stored in SGF files.
 *
 * A #GSGFCookedValue encapsulates qualified data read from an SGF file.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#define GSGF_COOKED_VALUE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COOKED_VALUE,           \
                                      GSGFCookedValuePrivate))

G_DEFINE_TYPE (GSGFCookedValue, gsgf_cooked_value, G_TYPE_OBJECT)

static void
gsgf_cooked_value_init(GSGFCookedValue *self)
{
}

static void
gsgf_cooked_value_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_cooked_value_parent_class)->finalize(object);
}

static void
gsgf_cooked_value_class_init(GSGFCookedValueClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        klass->write_stream = NULL;

        object_class->finalize = gsgf_cooked_value_finalize;
}

/**
 * gsgf_cooked_value_write_stream:
 * @self: The #GSGFCookedValue
 * @out: The #GOutputStream to write to.
 * @bytes_written: Location to store the number of bytes written or %NULL.
 * @cancellable: Optional #GCancellable object or %NULL.
 * @error: Optional #GError location or %NULL to ignore.
 *
 * Serialize a #GSGFCookedValue into a #GOutputStream.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_cooked_value_write_stream(const GSGFCookedValue *self,
                               GOutputStream *out, gsize *bytes_written,
                               GCancellable *cancellable, GError **error)
{
        if (!GSGF_IS_COOKED_VALUE(self)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Invalid cast to GSGFCookedValue"));
                /* Print standard error message on error return.  */
                g_return_val_if_fail(GSGF_IS_COOKED_VALUE(self), FALSE);
        }

        if (!GSGF_COOKED_VALUE_GET_CLASS(self)->write_stream) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Method write_stream not implemented"));
                g_return_val_if_fail(GSGF_COOKED_VALUE_GET_CLASS(self)->write_stream,
                                     FALSE);
        }

        return GSGF_COOKED_VALUE_GET_CLASS(self)->write_stream(self,
                                                               out, bytes_written,
                                                               cancellable, error);
}

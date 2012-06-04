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
 * SECTION:gsgf-component
 * @short_description: Component of an SGF document.
 * Since: 0.1.0
 *
 * An SGF document consists of a hierarchy of components, ranging from
 * #GSGFCollection over #GSGFGameTree, #GSGFNode, down to a #GSGFProperty.
 * They all implement the #GSGFComponent interface.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef GSGFComponentIface GSGFComponentInterface;
G_DEFINE_INTERFACE (GSGFComponent, gsgf_component, G_TYPE_OBJECT)

static void
gsgf_component_default_init (GSGFComponentInterface *iface)
{
}

/**
 * gsgf_component_cook:
 * @self: The #GSGFComponent to cook.
 * @culprit: Optional location for storing a #GSGFComponent that caused the
 *           operation to fail.
 * @error: Optional location to store an error.
 *
 * When parsing SGF documents, the document is only checked for well-formedness,
 * ignoring all kinds of semantic errors.  The information stored in the
 * #GSGFCollection that results from parsing is unqualified.
 *
 * Cooking is the process of turning the unqualified information into a
 * semantic tree.  Initially, all properties (#GSGFProperty) are of type
 * #GSGFRaw.  After cooking they are cooked into qualified types like
 * #GSGFReal, #GSGFSimpleText, #GSGFMove and so on.
 *
 * The process of cooking is interrupted as soon as a semantic or other error
 * is detected.  If you want to find out the #GSGFComponent that caused the
 * failure you should pass a location to save the address of the @culprit.
 * In doubt, you can try to remove the #GSGFComponent or try to repair it.
 * You can safely "re-cook" every component.  If it is already qualified,
 * nothing will be changed.
 *
 * There are three options for the return value.  A value of %NULL is returned
 * for failure.  All non-%NULL return values mean success.  Every #GSGFRaw
 * in the tree is either changed into a qualified #GSGFCookedValue, or it
 * remains unchanged if the property is unknown and cannot be qualified.
 * In the special case that you call this interface method on a #GSGFProperty,
 * the return value will differ from the passed-in @component.
 *
 * You should normally call this method on a top-level #GSGFCollection.
 * In case of failure you have two options: You can bail out, or you can try
 * to repair the collection by either modifying certain components or
 * by simply deleting the @culprit.  In the latter case you should be aware
 * that you may render the data useless.  For example, if you delete one
 * move from a game of backgammon, the resulting SGF tree will most probably
 * be ruined.  Choose your own poison!
 *
 * Returns: %TRUE for success or %FALSE for failure.
 **/
gboolean
gsgf_component_cook (GSGFComponent *self,
                     GSGFComponent **culprit, GError **error)
{
        GSGFComponentIface *iface;

        gsgf_return_val_if_fail (GSGF_IS_COMPONENT (self), FALSE, error);

        iface = GSGF_COMPONENT_GET_IFACE (self);

        return (*iface->cook) (self, culprit, error);
}

/**
 * gsgf_component_write_stream
 * @self: the #GSGFComponent.
 * @out: a #GOutputStream to write to.
 * @bytes_written: number of bytes written to the stream.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Serializes a #GSGFCollection and writes the serialized data into
 * a #GOutputStream.
 *
 * If there is an error during the operation FALSE is returned and @error
 * is set to indicate the error status, @bytes_written is updated to contain
 * the number of bytes written into the stream before the error occurred.
 *
 * Returns: %TRUE on success.  %FALSE if there was an error.
 **/
gboolean
gsgf_component_write_stream (const GSGFComponent *self,
                             GOutputStream *out,
                             gsize *bytes_written,
                             GCancellable *cancellable,
                             GError **error)
{
        GSGFComponentIface *iface;

        gsgf_return_val_if_fail (bytes_written != NULL, FALSE, error);

        *bytes_written = 0;

        gsgf_return_val_if_fail (GSGF_IS_COMPONENT (self), FALSE, error);

        iface = GSGF_COMPONENT_GET_IFACE (self);

        return (*iface->write_stream) (self, out, bytes_written,
                                       cancellable, error);
}

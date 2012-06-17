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
 * SECTION:gibbon-match-loader
 * @short_description: Load a backgammon match.
 *
 * Since: 0.2.0
 *
 * Load a backgammon match for display in Gibbon.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-loader.h"

G_DEFINE_TYPE (GibbonMatchLoader, gibbon_match_loader, G_TYPE_OBJECT)

static void 
gibbon_match_loader_init (GibbonMatchLoader *self)
{
}

static void
gibbon_match_loader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_loader_parent_class)->finalize(object);
}

static void
gibbon_match_loader_class_init (GibbonMatchLoaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        object_class->finalize = gibbon_match_loader_finalize;
}

/**
 * gibbon_match_loader_new:
 *
 * Creates a new #GibbonMatchLoader.
 *
 * Returns: The newly created #GibbonMatchLoader or %NULL in case of failure.
 */
GibbonMatchLoader *
gibbon_match_loader_new (void)
{
        GibbonMatchLoader *self = g_object_new (GIBBON_TYPE_MATCH_LOADER, NULL);

        return self;
}

GibbonMatch *
gibbon_match_loader_read_match (GibbonMatchLoader *self,
                                const gchar *filename,
                                GError **error)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LOADER (self), NULL);

        g_set_error_literal (error, GIBBON_MATCH_ERROR,
                             GIBBON_MATCH_ERROR_GENERIC,
                             _("Not yet implemented"));

        return NULL;
}

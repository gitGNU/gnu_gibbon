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
 * SECTION:gsgf-flavor-backgammon
 * @short_description: Definitions for the Backgammon flavor of SGF
 *
 * The Backgammon flavor of SGF as defined for GNU Backgammon.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

G_DEFINE_TYPE(GSGFFlavorBackgammon, gsgf_flavor_backgammon, GSGF_TYPE_FLAVOR)

static GSGFMove *gsgf_flavor_backgammon_create_move (const GSGFFlavor *flavor,
                                                     const struct _GSGFRaw *raw,
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

        object_class->finalize = gsgf_flavor_backgammon_finalize;
}

/**
 * gsgf_flavor_backgammon_new:
 *
 * Creates a new #GSGFFlavorBackgammon.
 *
 * Returns: The new #GSGFFlavorBackgammon.
 */
GSGFFlavorBackgammon *
gsgf_flavor_backgammon_new (void)
{
        GSGFFlavorBackgammon *self = g_object_new(GSGF_TYPE_FLAVOR_BACKGAMMON, NULL);

        return self;
}

static GSGFMove *
gsgf_flavor_backgammon_create_move (const GSGFFlavor *flavor,
                                    const GSGFRaw *raw,
                                    GError **error)
{
        /* FIXME! We need the GSGFProperty so that we can check that our
         * move is a single-value property.
         */
        return gsgf_move_backgammon_new_from_raw(raw, error);
}

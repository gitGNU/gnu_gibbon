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
 * SECTION:libgsgf
 * @short_description: Other functions
 * @include: libgsgf/libgsgf.h
 *
 * Initialize the library and register your own flavors.
 **/

#include <glib/gi18n.h>

#include <gsgf.h>

#include "gsgf-private.h"

GHashTable *_libgsgf_flavors = NULL;
static volatile gsize libgsgf_flavors_init = 0;

typedef struct _GSGFFlavorInfo {
        const char *id;
        const char *name;
} _GSGFFlavorInfo;

static _GSGFFlavorInfo builtin_flavors[] = {
                { "", N_("Base Definitions and Types") },
                { "BACKGAMMON", N_("Backgammon") },
};

/**
 * libgsgf_init:
 *
 * Initializes the library.  Normally you don't have to bother about
 * calling this function.  The first time you instantiate a
 * #GSGFCollection or register a new flavor, this function gets called for 
 * you automatically.
 **/
void
libgsgf_init()
{
        gsize i;
        _GSGFFlavorInfo *info;
        GSGFFlavor *flavor;

        if (g_once_init_enter(&libgsgf_flavors_init)) {
                _libgsgf_flavors = g_hash_table_new_full(g_str_hash, g_str_equal,
                                g_free,
                                (GDestroyNotify) g_hash_table_destroy);

                for (i = 0; i < sizeof builtin_flavors / sizeof builtin_flavors[0]; ++i) {
                        info = &builtin_flavors[i];
                        flavor = gsgf_flavor_new(info->id, info->name);

                        g_hash_table_insert(_libgsgf_flavors, g_strdup(info->id), flavor);
                }

                g_once_init_leave(&libgsgf_flavors_init, 1303);
        }
}

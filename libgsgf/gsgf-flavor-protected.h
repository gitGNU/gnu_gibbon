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

#ifndef _LIBGSGF_FLAVOR_PROTECTED_H
# define _LIBGSGF_FLAVOR_PROTECTED_H

#include <gsgf.h>

extern GHashTable *_libgsgf_flavors;

G_BEGIN_DECLS

typedef GSGFCookedValue * (*gsgf_cooked_constructor) (const GSGFRaw *raw, GError **error);
typedef gboolean (*gsgf_cooked_constraint) (const GSGFCookedValue *cooked, GError **error);

struct _GSGFFlavorTypeDef {
        gsgf_cooked_constructor constructor;
        gsgf_cooked_constraint constraint;
};

typedef struct _GSGFFlavorTypeDef GSGFFlavorTypeDef;

gboolean gsgf_flavor_is_positive_number(const GSGFCookedValue *cooked, GError **error);

G_END_DECLS

#endif

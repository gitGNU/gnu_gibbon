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

#ifndef _LIBGSGF_FLAVOR_PROTECTED_H
# define _LIBGSGF_FLAVOR_PROTECTED_H

#include <gsgf.h>

extern GHashTable *_libgsgf_flavors;

G_BEGIN_DECLS

typedef GSGFCookedValue * (*GSGFCookedConstructor) (const GSGFRaw *raw,
                                                    const GSGFFlavor *flavor,
                                                    const GSGFProperty *property,
                                                    GError **error);
typedef gboolean (*GSGFCookedConstraint) (const GSGFCookedValue *cooked,
                                          const GSGFRaw *raw,
                                          const GSGFProperty *property,
                                          GError **error);

struct _GSGFFlavorTypeDef {
        GSGFCookedConstructor constructor;
        GSGFCookedConstraint constraints[];
};

typedef struct _GSGFFlavorTypeDef GSGFFlavorTypeDef;

gboolean gsgf_constraint_is_positive_number(const GSGFCookedValue *cooked,
                                            const GSGFRaw *raw,
                                            const GSGFProperty *property, GError **error);
gboolean gsgf_constraint_is_root_property(const GSGFCookedValue *cooked,
                                          const GSGFRaw *raw,
                                          const GSGFProperty *property, GError **error);
gboolean gsgf_constraint_is_single_value(const GSGFCookedValue *cooked,
                                         const GSGFRaw *raw,
                                         const GSGFProperty *property, GError **error);

G_END_DECLS

#endif

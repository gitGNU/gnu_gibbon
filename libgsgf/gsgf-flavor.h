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

#ifndef _LIBGSGF_FLAVOR_H
# define _LIBGSGF_FLAVOR_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_FLAVOR             (gsgf_flavor_get_type ())
#define GSGF_FLAVOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_FLAVOR, GSGFFlavor))
#define GSGF_FLAVOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_FLAVOR, GSGFFlavorClass))
#define GSGF_IS_FLAVOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_FLAVOR))
#define GSGF_IS_FLAVOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_FLAVOR))
#define GSGF_FLAVOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_FLAVOR, GSGFFlavorClass))

/**
 * GSGFFlavor:
 *
 * One instance of a #GSGFFlavorClass.
 **/
typedef struct _GSGFFlavor        GSGFFlavor;
struct _GSGFFlavor
{
        GObject parent_instance;
};

struct _GSGFRaw;
struct _GSGFCookedValue;
struct _GSGFProperty;
struct _GSGFMove;
struct _GSGFPoint;
struct _GSGFStone;
struct _GSGFListOf;

/**
 * GSGFFlavorClass:
 *
 * Class representing a flavor of SGF.
 **/
typedef struct _GSGFFlavorClass   GSGFFlavorClass;
struct _GSGFFlavorClass
{
        /*< private >*/
        GObjectClass parent_class;

        gboolean (*get_cooked_value) (const GSGFFlavor *flavor,
                                      const struct _GSGFProperty *property,
                                      const struct _GSGFRaw *raw,
                                      struct _GSGFCookedValue **cooked,
                                      GError **error);

        struct _GSGFMove *(*create_move) (const GSGFFlavor *flavor,
                                          const struct _GSGFRaw *raw,
                                          GError **error);
        GType stone_type;
        struct _GSGFStone *(*create_stone) (const GSGFFlavor *flavor,
                                            const struct _GSGFRaw *raw,
                                            gsize i,
                                            GError **error);

        GType point_type;
        struct _GSGFPoint *(*create_point) (const GSGFFlavor *flavor,
                                            const struct _GSGFRaw *raw,
                                            gsize i,
                                            GError **error);
        gboolean (*append_points) (const GSGFFlavor *flavor,
                                   struct _GSGFListOf *list_of,
                                   const struct _GSGFRaw *raw,
                                   gsize i,
                                   GError **error);

        gboolean (*write_compressed_list) (const GSGFFlavor *flavor,
                                           const struct _GSGFListOf *list_of,
                                           GOutputStream *out,
                                           gsize *bytes_written,
                                           GCancellable *cancellable,
                                           GError **error);

        guint (*get_game_id) (const GSGFFlavor *flavor);
};

GType gsgf_flavor_get_type(void) G_GNUC_CONST;

GSGFFlavor *gsgf_flavor_new(void);
gboolean gsgf_flavor_get_cooked_value(const GSGFFlavor *self, 
                                      const struct _GSGFProperty *property,
                                      const struct _GSGFRaw *raw,
                                      struct _GSGFCookedValue **cooked,
                                      GError **error);
guint gsgf_flavor_get_game_id (const GSGFFlavor *self, GError **error);

G_END_DECLS

#endif

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

#ifndef _GIBBON_VARIANT_LIST_H
# define _GIBBON_VARIANT_LIST_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#define GIBBON_TYPE_VARIANT_LIST \
        (gibbon_variant_list_get_type ())
#define GIBBON_VARIANT_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_VARIANT_LIST, \
                GibbonVariantList))
#define GIBBON_VARIANT_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_VARIANT_LIST, GibbonVariantListClass))
#define GIBBON_IS_VARIANT_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_VARIANT_LIST))
#define GIBBON_IS_VARIANT_LIST_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_VARIANT_LIST))
#define GIBBON_VARIANT_LIST_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_VARIANT_LIST, GibbonVariantListClass))

/**
 * GibbonVariantList:
 *
 * One instance of a #GibbonVariantList.  All properties are private.
 */
typedef struct _GibbonVariantList GibbonVariantList;
struct _GibbonVariantList
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonVariantListPrivate *priv;
};

/**
 * GibbonVariantListClass:
 *
 * A list store for storing all aspects of a analysed move variant.
 */
typedef struct _GibbonVariantListClass GibbonVariantListClass;
struct _GibbonVariantListClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_variant_list_get_type (void) G_GNUC_CONST;

enum {
        GIBBON_VARIANT_LIST_COL_NUMBER = 0,
        GIBBON_VARIANT_LIST_COL_WEIGHT,
        GIBBON_VARIANT_LIST_COL_ANALYSIS_TYPE,
        GIBBON_VARIANT_LIST_COL_MOVE,
        GIBBON_VARIANT_LIST_COL_EQUITY,
        GIBBON_VARIANT_LIST_COL_MATCH_LENGTH,
        GIBBON_VARIANT_LIST_COL_CUBE,
        GIBBON_VARIANT_LIST_COL_MY_SCORE,
        GIBBON_VARIANT_LIST_COL_OPP_SCORE,
        GIBBON_VARIANT_LIST_COL_PWIN,
        GIBBON_VARIANT_LIST_COL_PWIN_G,
        GIBBON_VARIANT_LIST_COL_PWIN_BG,
        GIBBON_VARIANT_LIST_COL_PLOSE,
        GIBBON_VARIANT_LIST_COL_PLOSE_G,
        GIBBON_VARIANT_LIST_COL_PLOSE_BG,
        GIBBON_VARIANT_LIST_COL_POSITION,
        GIBBON_INVITER_LIST_N_COLUMNS
};

GibbonVariantList *gibbon_variant_list_new (void);
GtkListStore *gibbon_variant_list_get_store (GibbonVariantList *self);

#endif

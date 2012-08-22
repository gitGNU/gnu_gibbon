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

#ifndef _GIBBON_MET_H
# define _GIBBON_MET_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_MET \
        (gibbon_met_get_type ())
#define GIBBON_MET(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MET, \
                GibbonMET))
#define GIBBON_MET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MET, GibbonMETClass))
#define GIBBON_IS_MET(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MET))
#define GIBBON_IS_MET_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MET))
#define GIBBON_MET_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MET, GibbonMETClass))

#define GIBBON_MET_MAX_LENGTH 100

#define GIBBON_MET_GAMMON_RATE 0.25f

/* Free drop vigorish.  */
#define GIBBON_MET_FD2 0.015f
#define GIBBON_MET_FD4 0.004f

/**
 * GibbonMET:
 *
 * One instance of a #GibbonMET.  All properties are private.
 */
typedef struct _GibbonMET GibbonMET;
struct _GibbonMET
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMETPrivate *priv;
};

/**
 * GibbonMETClass:
 *
 * Class reqpresenting a match equity table!
 */
typedef struct _GibbonMETClass GibbonMETClass;
struct _GibbonMETClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_met_get_type (void) G_GNUC_CONST;

GibbonMET *gibbon_met_new (void);
gdouble gibbon_met_get_match_equity (const GibbonMET *self,
                                     gsize match_length, guint cube,
                                     guint my_score, guint opp_score);
gdouble gibbon_met_eq2mwc (const GibbonMET *self, gdouble equity,
                           gsize match_length, guint cube,
                           guint my_score, guint opp_score);
gdouble gibbon_met_mwc2eq (const GibbonMET *self, gdouble mwc,
                           gsize match_length, guint cube,
                           guint my_score, guint opp_score);

#endif

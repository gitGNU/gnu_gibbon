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

#ifndef _GIBBON_MATCH_TRACKER_H
# define _GIBBON_MATCH_TRACKER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-position.h"

#define GIBBON_TYPE_MATCH_TRACKER \
        (gibbon_match_tracker_get_type ())
#define GIBBON_MATCH_TRACKER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH_TRACKER, \
                GibbonMatchTracker))
#define GIBBON_MATCH_TRACKER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerClass))
#define GIBBON_IS_MATCH_TRACKER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH_TRACKER))
#define GIBBON_IS_MATCH_TRACKER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH_TRACKER))
#define GIBBON_MATCH_TRACKER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerClass))

/**
 * GibbonMatchTracker:
 *
 * One instance of a #GibbonMatchTracker.  All properties are private.
 *
 * A #GibbonMatchTracker can tracks a backgammon match, updates move list views,
 * archives the match and so on.  It is also responsible for trying to find
 * a saved state for the match in case it is a resume.
 */
typedef struct _GibbonMatchTracker GibbonMatchTracker;
struct _GibbonMatchTracker
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchTrackerPrivate *priv;
};

/**
 * GibbonMatchTrackerClass:
 *
 * Class for a #GibbonMatchTracker!
 */
typedef struct _GibbonMatchTrackerClass GibbonMatchTrackerClass;
struct _GibbonMatchTrackerClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_match_tracker_get_type (void) G_GNUC_CONST;

GibbonMatchTracker *gibbon_match_tracker_new (const gchar *player1,
                                              const gchar *player2,
                                              gsize length,
                                              gboolean resume);
void gibbon_match_tracker_store_rank (const GibbonMatchTracker *self,
                                      const gchar *rank,
                                      GibbonPositionSide side);
void gibbon_match_tracker_update (GibbonMatchTracker *self,
                                  const GibbonPosition *pos);
void gibbon_match_tracker_set_crawford (GibbonMatchTracker *self,
                                        gboolean flag);

#endif

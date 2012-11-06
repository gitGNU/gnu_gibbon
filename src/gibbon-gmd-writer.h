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

#ifndef _GIBBON_GMD_WRITER_H
# define _GIBBON_GMD_WRITER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match-writer.h"
#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-position.h"

#define GIBBON_TYPE_GMD_WRITER \
        (gibbon_gmd_writer_get_type ())
#define GIBBON_GMD_WRITER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GMD_WRITER, \
                GibbonGMDWriter))
#define GIBBON_GMD_WRITER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GMD_WRITER, GibbonGMDWriterClass))
#define GIBBON_IS_GMD_WRITER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GMD_WRITER))
#define GIBBON_IS_GMD_WRITER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GMD_WRITER))
#define GIBBON_GMD_WRITER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GMD_WRITER, GibbonGMDWriterClass))

#define GIBBON_GMD_REVISION 2

/**
 * GibbonGMDWriter:
 *
 * One instance of a #GibbonGMDWriter.  All properties are private.
 */
typedef struct _GibbonGMDWriter GibbonGMDWriter;
struct _GibbonGMDWriter
{
        GibbonMatchWriter parent_instance;
};

/**
 * GibbonGMDWriterClass:
 *
 * Writer for GMD internal format.
 */
typedef struct _GibbonGMDWriterClass GibbonGMDWriterClass;
struct _GibbonGMDWriterClass
{
        /* <private >*/
        GibbonMatchWriterClass parent_class;
};

GType gibbon_gmd_writer_get_type (void) G_GNUC_CONST;

GibbonGMDWriter *gibbon_gmd_writer_new (void);

gboolean gibbon_gmd_writer_update_rank (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        const GibbonMatch *match,
                                        GibbonPositionSide side,
                                        GError **error);
gboolean gibbon_gmd_writer_write_action (const GibbonGMDWriter *self,
                                         GOutputStream *out,
                                         const GibbonGame *game,
                                         const GibbonGameAction *action,
                                         GibbonPositionSide side,
                                         gint64 timestamp,
                                         GError **error);
gboolean gibbon_gmd_writer_add_game (const GibbonGMDWriter *self,
                                     GOutputStream *out,
                                     GError **error);

#endif

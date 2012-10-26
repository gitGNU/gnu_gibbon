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
 * SECTION:gibbon-match-tracker
 * @short_description: Track backgammon matches.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchTracker records all relevant match actions and triggers
 * appropriate actions.  It continually updates the match file in the
 * archive and it adds moves made to the move list view.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-tracker.h"
#include "gibbon-match.h"
#include "gibbon-app.h"
#include "gibbon-archive.h"
#include "gibbon-gmd-writer.h"

typedef struct _GibbonMatchTrackerPrivate GibbonMatchTrackerPrivate;
struct _GibbonMatchTrackerPrivate {
        GibbonMatch *match;
        gchar *outname;
        GibbonGMDWriter *writer;
        GOutputStream *out;
};

#define GIBBON_MATCH_TRACKER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerPrivate))

G_DEFINE_TYPE (GibbonMatchTracker, gibbon_match_tracker, G_TYPE_OBJECT)

static void 
gibbon_match_tracker_init (GibbonMatchTracker *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerPrivate);

        self->priv->match = NULL;
        self->priv->outname = NULL;
        self->priv->writer = NULL;
}

static void
gibbon_match_tracker_finalize (GObject *object)
{
        GibbonMatchTracker *self = GIBBON_MATCH_TRACKER (object);
        GError *error = NULL;

        if (self->priv->out && !g_output_stream_close (self->priv->out,
                                                       NULL, &error)) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }

        if (self->priv->match)
                g_object_unref (self->priv->match);

        g_free (self->priv->outname);

        if (self->priv->writer)
                g_object_unref (self->priv->writer);

        G_OBJECT_CLASS (gibbon_match_tracker_parent_class)->finalize(object);
}

static void
gibbon_match_tracker_class_init (GibbonMatchTrackerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchTrackerPrivate));

        object_class->finalize = gibbon_match_tracker_finalize;
}

/**
 * gibbon_match_tracker_new:
 * @player1: White.
 * @player2: Black.
 * @length:  Match length or 0 for unlimited match.
 * @resume:  Try to find the start of the match in the archive and continue
 *           it.
 *
 * Creates a new #GibbonMatchTracker.
 *
 * Returns: The newly created #GibbonMatchTracker or %NULL in case of failure.
 */
GibbonMatchTracker *
gibbon_match_tracker_new (const gchar *player1, const gchar *player2,
                          gsize length, gboolean resume)
{
        GibbonMatchTracker *self = g_object_new (GIBBON_TYPE_MATCH_TRACKER,
                                                 NULL);
        GibbonArchive *archive = gibbon_app_get_archive (app);
        GFile *file;
        GFileOutputStream *fout;
        GError *error = NULL;
        GibbonMatchWriter *writer;

        self->priv->outname = gibbon_archive_get_saved_name (archive, player1,
                                                             player2);

        file = g_file_new_for_path (self->priv->outname);
        fout = g_file_replace (file, NULL, FALSE, G_FILE_COPY_NONE,
                               NULL, &error);
        g_object_unref (file);
        if (!fout) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }
        self->priv->out = G_OUTPUT_STREAM (fout);

        /*
         * We always assume that the Crawford rule applies for fixed-length
         * matches.  We will find out whether this is correct or not later.
         */
        self->priv->match = gibbon_match_new (player1, player2, length, length);
        self->priv->writer = gibbon_gmd_writer_new ();
        writer = GIBBON_MATCH_WRITER (self->priv->writer);

        if (!gibbon_match_writer_write_stream (writer, self->priv->out,
                                               self->priv->match, &error)) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }

        return self;
}
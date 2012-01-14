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
 * SECTION:gibbon-match-writer
 * @short_description: Writ backgammon matches in various formats
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter is the abstract base class for writers of the
 * individual formats supported by Gibbon.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-writer.h"

G_DEFINE_TYPE (GibbonMatchWriter, gibbon_match_writer, G_TYPE_OBJECT)

static void 
gibbon_match_writer_init (GibbonMatchWriter *self)
{
}

static void
gibbon_match_writer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_writer_parent_class)->finalize(object);
}

static void
gibbon_match_writer_class_init (GibbonMatchWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GibbonMatchWriter_do_that; */

        object_class->finalize = gibbon_match_writer_finalize;
}

gboolean
gibbon_match_writer_write_stream (const GibbonMatchWriter *self,
                                  GOutputStream *out,
                                  const GibbonMatch *match,
                                  GError **error)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_WRITER (self), FALSE);
        g_return_val_if_fail (GIBBON_MATCH_WRITER_GET_CLASS (self)->write_stream,
                              FALSE);
        g_return_val_if_fail (G_IS_OUTPUT_STREAM (out), FALSE);
        g_return_val_if_fail (GIBBON_IS_MATCH (match), FALSE);

        return GIBBON_MATCH_WRITER_GET_CLASS(self)->write_stream (self, out,
                                                                  match, error);
}


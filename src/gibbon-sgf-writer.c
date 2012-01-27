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
 * SECTION:gibbon-sgf-writer
 * @short_description: Convert a #GibbonMatch to SGF.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for SGF.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-sgf-writer.h"

G_DEFINE_TYPE (GibbonSGFWriter, gibbon_sgf_writer, GIBBON_TYPE_MATCH_WRITER)

static void 
gibbon_sgf_writer_init (GibbonSGFWriter *self)
{
}

static void
gibbon_sgf_writer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_sgf_writer_parent_class)->finalize(object);
}

static void
gibbon_sgf_writer_class_init (GibbonSGFWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class =
                        GIBBON_MATCH_WRITER_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gibbon_match_writer_class->do_this = gibbon_sgf_writer_do_this; */
        
        object_class->finalize = gibbon_sgf_writer_finalize;
}

/**
 * gibbon_sgf_writer_new:
 *
 * Creates a new #GibbonSGFWriter.
 *
 * Returns: The newly created #GibbonSGFWriter or %NULL in case of failure.
 */
GibbonSGFWriter *
gibbon_sgf_writer_new (void)
{
        GibbonSGFWriter *self = g_object_new (GIBBON_TYPE_SGF_WRITER, NULL);

        return self;
}

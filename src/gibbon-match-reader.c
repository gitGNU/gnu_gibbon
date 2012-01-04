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
 * SECTION:gibbon-match-reader
 * @short_description: Read backgammon matches in various formats
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchReader is the abstract base class for readers of the
 * individual formats supported by Gibbon.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-reader.h"

G_DEFINE_TYPE (GibbonMatchReader, gibbon_match_reader, G_TYPE_OBJECT)

static void 
gibbon_match_reader_init (GibbonMatchReader *self)
{
}

static void
gibbon_match_reader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_reader_parent_class)->finalize(object);
}

static void
gibbon_match_reader_class_init (GibbonMatchReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GibbonMatchReader_do_that; */

        object_class->finalize = gibbon_match_reader_finalize;
}

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
 * SECTION:gibbon-jelly-fish-writer
 * @short_description: Convert GibbonMatch to JellyFish format
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for the JellyFish format.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-jelly-fish-writer.h"

G_DEFINE_TYPE (GibbonJellyFishWriter, gibbon_jelly_fish_writer,
               GIBBON_TYPE_MATCH_WRITER)

static void 
gibbon_jelly_fish_writer_init (GibbonJellyFishWriter *self)
{
}

static void
gibbon_jelly_fish_writer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_jelly_fish_writer_parent_class)->finalize(object);
}

static void
gibbon_jelly_fish_writer_class_init (GibbonJellyFishWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class = GIBBON_MATCH_WRITER_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gibbon_match_writer_class->do_this = gibbon_jelly_fish_writer_do_this; */

        object_class->finalize = gibbon_jelly_fish_writer_finalize;
}

/**
 * gibbon_jelly_fish_writer_new:
 *
 * Creates a new #GibbonJellyFishWriter.
 *
 * Returns: The newly created #GibbonJellyFishWriter or %NULL in case of failure.
 */
GibbonJellyFishWriter *
gibbon_jelly_fish_writer_new (void)
{
        GibbonJellyFishWriter *self = g_object_new (GIBBON_TYPE_JELLY_FISH_WRITER, NULL);
        return self;
}

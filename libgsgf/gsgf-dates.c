/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-dates
 * @short_description: Representation of a date in SGF.
 *
 * Since: 0.1.1
 *
 * The <link linkend="property-DT">DT</link> property contains a standardized
 * date format.  The representation for this in libgsgf is a #GSGFDates.
 * Note that dates in SGF do not contain information about the time of the day.
 * If you need information about the hour, minutes, and seconds you have to
 * store that externally.  Gibbon codes the time of the day into the filenames.
 *
 * It is <emphasis>dates</dates> (plural!) and not date because dates in SGF
 * can span over multiple days.
 *
 * Hint: Unless you are interested in analyzing the exact date you can simply
 * treat a #GSGFDates as a #GSGFSimpleText.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFDatesPrivate GSGFDatesPrivate;
struct _GSGFDatesPrivate {
        GList *dates;
};

#define GSGF_DATES_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_DATES, GSGFDatesPrivate))

G_DEFINE_TYPE (GSGFDates, gsgf_dates, GSGF_TYPE_SIMPLE_TEXT)

static void 
gsgf_dates_init (GSGFDates *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_DATES, GSGFDatesPrivate);

        self->priv->dates = NULL;
}

static void
gsgf_dates_finalize (GObject *object)
{
        GSGFDates *self = GSGF_DATES (object);

        if (self->priv->dates) {
                g_list_foreach (self->priv->dates, (GFunc) g_free, NULL);
                g_list_free (self->priv->dates);
        }
        self->priv->dates = NULL;

        G_OBJECT_CLASS (gsgf_dates_parent_class)->finalize(object);
}

static void
gsgf_dates_class_init (GSGFDatesClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFSimpleTextClass *gsgf_simple_text_class = GSGF_SIMPLE_TEXT_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gsgf_simple_text_class->do_this = gsgf_dates_do_this; */
        
        g_type_class_add_private(klass, sizeof (GSGFDatesPrivate));

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GSGFDates_do_that; */

        object_class->finalize = gsgf_dates_finalize;
}

GSGFDates *
gsgf_dates_new (gint year, gint month, gint day)
{
        GSGFDates *self = g_object_new (GSGF_TYPE_DATES, NULL);

        return self;
}

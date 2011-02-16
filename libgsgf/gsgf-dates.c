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
 * Dates in SGF do not necessarily have a resolution of a day.  They can
 * also be used to just describe a year ("2011") or a month ("2011-02").
 * If the month of the day of the month for such a date is not known the
 * library internally uses the constants #G_DATE_BAD_MONTH or
 * #G_DATE_BAD_YEAR internally.
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
                g_list_foreach (self->priv->dates, (GFunc) g_date_free, NULL);
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

/**
 * gsgf_dates_new:
 * @year: the year or #G_DATE_BAD_YEAR
 * @month: the month or #G_DATE_BAD_MONTH
 * @mday: the day of the month or #G_DATE_BAD_DAY
 *
 * Create a single date (use gsgf_dates_append() for adding more days).
 *
 * If you pass #G_DATE_BAD_YEAR (aka 0) for the year, the current year will
 * be used.
 *
 * If you pass #G_DATE_BAD_MONTH for the @month the resolution will only be
 * a year, and if you pass #G_DATE_BAD_DAY for the @mday the resolution will
 * only be a month.
 *
 * Returns: the new #GDates, the function cannot fail.
 */
GSGFDates *
gsgf_dates_new (gint year, gint month, gint mday)
{
        GSGFDates *self = g_object_new (GSGF_TYPE_DATES, NULL);
        GDate *date = g_date_new ();

        self->priv->dates = g_list_append (self->priv->dates, date);

        if (year == G_DATE_BAD_YEAR) {
                year = 1970; /* FIXME! */
        }

        if (mday != G_DATE_BAD_DAY)
                g_date_set_day (date, mday);

        if (month != G_DATE_BAD_MONTH)
                g_date_set_month (date, month);

        g_date_set_year (date, year);

        return self;
}

/**
 * gsgf_dates_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFDates from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFDates or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_dates_new_from_raw (const GSGFRaw *raw, const GSGFFlavor *flavor,
                         const GSGFProperty *property, GError **error)
{
        const gchar *string;

        g_return_val_if_fail(GSGF_IS_RAW (raw), NULL);

        if (error)
                *error = NULL;

        if (1 != gsgf_raw_get_number_of_values (raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value (raw, 0);

        return GSGF_COOKED_VALUE (gsgf_dates_new (0, 0, 0));
}

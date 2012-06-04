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
 * SECTION:gsgf-date
 * @short_description: Representation of a date (or dates!) in SGF
 * Since: 0.1.0
 *
 * A #GSGFDate represents a list of dates with at least one value.
 *
 * The maximum accuracy is one day.  If you need more you have to store this
 * externally, for example in the file name.
 *
 * The class uses #GSGFDateDMY for its internal date representation and uses
 * the values #G_DATE_BAD_MONTH or #G_DATE_BAD_DAY for month or day of the
 * month values not available.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFDatePrivate GSGFDatePrivate;
struct _GSGFDatePrivate {
        GList *dates;
};

#define GSGF_DATE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_DATE, GSGFDatePrivate))

G_DEFINE_TYPE (GSGFDate, gsgf_date, GSGF_TYPE_SIMPLE_TEXT)

static gboolean gsgf_date_set_value (GSGFText *self, const gchar *value,
                                     gboolean copy, GError **error);
static void gsgf_date_sync_text (GSGFDate *self);
static gboolean gsgf_date_valid_dmy (GSGFDateDMY *dmy);
static gint gsgf_date_consume_digits (const gchar *string, guint *number);

static void 
gsgf_date_init (GSGFDate *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GSGF_TYPE_DATE,
                                                  GSGFDatePrivate);

        self->priv->dates = NULL;
}

static void
gsgf_date_finalize (GObject *object)
{
        GSGFDate *self = GSGF_DATE (object);

        if (self->priv->dates) {
                g_list_foreach (self->priv->dates, (GFunc) g_free, NULL);
                g_list_free (self->priv->dates);
        }
        self->priv->dates = NULL;

        G_OBJECT_CLASS (gsgf_date_parent_class)->finalize(object);
}

static void
gsgf_date_class_init (GSGFDateClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFTextClass *gsgf_text_class = GSGF_TEXT_CLASS (klass);

        gsgf_text_class->set_value = gsgf_date_set_value;
        
        g_type_class_add_private(klass, sizeof (GSGFDatePrivate));

        object_class->finalize = gsgf_date_finalize;
}

/**
 * gsgf_date_new:
 * @date: An initial #GSGFDateDMY to store.
 * @error: An optional location to store an error or %NULL.
 *
 * Create a new #GSGFDate.  You can initialize a #GSGFDate only with one
 * value.  Use gsgf_date_append() for events that span over multiple days.
 *
 * The #GSGFDate you pass is copied.  You are responsible for freeing it.
 *
 * Returns: The new #GSGFDate.
 */
GSGFDate *
gsgf_date_new (GSGFDateDMY* date, GError **error)
{
        GSGFDate *self;
        GSGFDateDMY *copy;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (date, NULL, error);

        if (date) {
                if (!g_date_valid_year (date->year)) {
                        g_set_error (error, GSGF_ERROR,
                                     GSGF_ERROR_SEMANTIC_ERROR,
                                     _("Invalid year in date specification"));
                        return NULL;
                }
        }

        if (!gsgf_date_valid_dmy (date)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                             _("Invalid date specification '%04d-%02d-%02d'"
                               " or out of range"),
                               date->year, date->month, date->day);
                return NULL;
        }

        self = g_object_new (GSGF_TYPE_DATE, NULL);
        copy = g_malloc (sizeof *date);
        *copy = *date;
        self->priv->dates = g_list_append (self->priv->dates, copy);

        gsgf_date_sync_text (self);

        return self;
}

/**
 * gsgf_date_append:
 * @self: A #GSGFDateDMY to append to.
 * @date: The #GSGFDateDMY to append.
 * @error: An optional error location or #NULL.
 *
 * Append another #GSGFDateDMY.
 *
 * Returns: #TRUE for success, #FALSE for failure.
 */
gboolean
gsgf_date_append (GSGFDate *self, GSGFDateDMY* date, GError **error)
{
        GSGFDateDMY *date_copy;

        gsgf_return_val_if_fail (GSGF_IS_DATE (self), FALSE, error);
        gsgf_return_val_if_fail (date, FALSE, error);

        if (!gsgf_date_valid_dmy (date)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                             _("Invalid date specification '%04d-%02d-%02d'"
                               " or out of range"),
                               date->year, date->month, date->day);
                return FALSE;
        }

        date_copy = g_malloc (sizeof *date_copy);
        *date_copy = *date;
        self->priv->dates = g_list_append (self->priv->dates, date_copy);

        gsgf_date_sync_text (self);

        return TRUE;
}

static gboolean
gsgf_date_set_value (GSGFText *_self, const gchar *value,
                     gboolean copy, GError **error)
{
        GSGFDate *self = GSGF_DATE (_self);
        GList *dates = NULL;
        const gchar *ptr = value;
        guint last_year = 0;
        guint last_month = 0;
        guint last_day = 0;
        guint this_year;
        guint this_month;
        guint this_day;
        gint digits;
        guint number;
        gboolean has_error = FALSE;
        gint digit_pair1, digit_pair2;
        GSGFDateDMY *date;

        if (error)
                *error = NULL;

        if (self->priv->dates) {
                g_list_foreach (self->priv->dates, (GFunc) g_free, NULL);
                g_list_free (self->priv->dates);
        }

        if (!copy) {
                g_critical ("Possible memory leak: GSGFDate::set_value()"
                            " called, and copy is FALSE!");
        }

        while (*ptr) {
                this_year = G_DATE_BAD_YEAR;
                this_month = G_DATE_BAD_MONTH;
                this_day = G_DATE_BAD_DAY;

                digit_pair1 = digit_pair2 = 0;

                digits = gsgf_date_consume_digits (ptr, &number);
                if (digits == 4) {
                        if (!number || !g_date_valid_year (number)) {
                                has_error = TRUE;
                                break;
                        }
                        this_year = number;
                } else if (digits == 2) {
                        if (!number) {
                                has_error = TRUE;
                                break;
                        }
                        this_year = last_year;
                        digit_pair1 = number;
                } else {
                        has_error = TRUE;
                        break;
                }

                ptr += digits;

                if (*ptr && ',' != *ptr) {
                        if ('-' != *ptr++) {
                                has_error = TRUE;
                                break;
                        }

                        digits = gsgf_date_consume_digits (ptr, &number);
                        if (digits != 2|| !number) {
                                has_error = TRUE;
                                break;
                        }

                        ptr += 2;
                        if (digit_pair1)
                                digit_pair2 = number;
                        else
                                digit_pair1 = number;
                }

                if (!digit_pair2 && *ptr && ',' != *ptr) {
                        if ('-' != *ptr++) {
                                has_error = TRUE;
                                break;
                        }
                        digits = gsgf_date_consume_digits (ptr, &number);
                        if (digits != 2|| !number) {
                                has_error = TRUE;
                                break;
                        }
                        ptr += 2;
                        digit_pair2 = number;
                }

                if (digit_pair1 && digit_pair2) {
                        this_month = digit_pair1;
                        this_day = digit_pair2;
                } else if (digit_pair1 && last_day) {
                        this_day = digit_pair1;
                        this_month = last_month;
                } else if (digit_pair1) {
                        this_month = digit_pair1;
                }

                if (!this_year)
                        this_year = last_year;
                if (!this_month)
                        this_month = last_month;
                if (!this_day)
                        this_day = last_day;

                /* Check validity of date, replacing possibly omitted data with
                 * safe choices.
                 */
                date = g_malloc (sizeof *date);
                dates = g_list_append (dates, date);

                date->year = last_year = this_year;
                date->month = last_month = this_month;
                date->day = last_day = this_day;

                if (!gsgf_date_valid_dmy (date)) {
                        has_error = 1;
                        break;
                }

                if (',' == *ptr) {
                        ++ptr;
                        if (!*ptr) {
                                has_error = 1;
                                break;
                        }
                }
        }

        if (*ptr)
                has_error = TRUE;

        if (has_error) {
                g_list_foreach (dates, (GFunc) g_free, NULL);
                g_list_free (dates);
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                             _("Invalid date specification '%s'"
                               " or out of range"),
                               value);
                dates = NULL;
        }

        self->priv->dates = dates;
        gsgf_date_sync_text (self);

        if (!has_error && !dates) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                             _("Empty dates are not allowed"));
                return FALSE;
        }

        return TRUE;
}

/**
 * gsgf_date_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Creates a new #GSGFDate from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFDate or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_date_new_from_raw (const GSGFRaw *raw, const GSGFFlavor *flavor,
                        const GSGFProperty *property, GError **error)
{
        const gchar *string;
        GSGFResult *self;

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);

        if (1 != gsgf_raw_get_number_of_values (raw)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                             _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value (raw, 0);

        self = g_object_new (GSGF_TYPE_DATE, NULL);
        if (!gsgf_text_set_value (GSGF_TEXT (self), string, TRUE, error)) {
                g_object_unref (self);
                return NULL;
        }

        return GSGF_COOKED_VALUE (self);
}

static void
gsgf_date_sync_text (GSGFDate *self)
{
        GSGFTextClass* text_class;
        guint last_day = 0;
        guint last_month = 0;
        guint last_year = 0;
        GString *string;
        GList *iter = self->priv->dates;
        GSGFDateDMY *date;
        gint mask;

        string = g_string_sized_new (10);

        while (iter) {
                if (iter != self->priv->dates)
                        g_string_append_c (string, ',');

                date = iter->data;

                /* We use a bit mask to specify which part of a date can be
                 * suppressed.
                 */
                mask = 0;

                if (date->year != last_year)
                        mask += 4;

                if ((date->year != last_year || date->month != last_month)
                    && date->month != G_DATE_BAD_MONTH)
                        mask += 2;

                if ((date->year != last_year || date->month != last_month
                                || date->day != last_day)
                    && date->day != G_DATE_BAD_DAY)
                        ++mask;

                switch (mask) {
                        case 6:
                                g_string_append_printf (string, "%04d-%02d",
                                                        date->year,
                                                        date->month);
                                break;
                        case 4:
                                g_string_append_printf (string, "%04d",
                                                        date->year);
                                break;
                        case 3:
                                g_string_append_printf (string, "%02d-%02d",
                                                        date->month, date->day);
                                break;
                        case 2:
                                g_string_append_printf (string, "%02d",
                                                        date->month);
                                break;
                        case 1:
                                g_string_append_printf (string, "%02d",
                                                        date->day);
                                break;
                        case 5:
                                g_critical ("Combination of year and month"
                                            "a in a GSGFDate encountered");
                        default:
                                g_string_append_printf (string,
                                                        "%04d-%02d-%02d",
                                                        date->year,
                                                        date->month,
                                                        date->day);
                }

                last_day = date->day;
                last_month = date->month;
                last_year = date->year;
                iter = iter->next;
        }

        text_class = g_type_class_peek_parent (GSGF_RESULT_GET_CLASS (self));
        text_class->set_value (GSGF_TEXT (self), string->str, FALSE, NULL);

        g_string_free (string, FALSE);
}

static gint
gsgf_date_consume_digits (const gchar *string, guint *number)
{
        gint digits = 0;
        *number = 0;

        while (*string) {
                if (*string >= '0' && *string <= '9') {
                        *number *= 10;
                        *number += *string - '0';
                        ++digits;
                        ++string;
                } else {
                        break;
                }
        }

        if (digits > 4)
                return -1;

        return digits;
}

static gboolean
gsgf_date_valid_dmy (GSGFDateDMY *dmy)
{
        GDate *g_date = g_date_new ();
        gboolean retval;

        g_date_clear (g_date, 1);

        g_date_set_year (g_date, dmy->year);
        g_date_set_month (g_date, dmy->month ? dmy->month : 1);
        g_date_set_day (g_date, dmy->day ? dmy->day : 1);

        retval = g_date_valid (g_date);
        g_date_free (g_date);

        return retval;
}

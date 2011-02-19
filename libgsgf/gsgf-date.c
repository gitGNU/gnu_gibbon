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
 * SECTION:gsgf-date
 * @short_description: Representation of a date (or dates!) in SGF
 *
 * Since: 0.1.1
 *
 * A #GSGFDate represents a list of dates with at least one value.
 *
 * The maximum accuracy is one day.  If you need more you have to store this
 * externally, for example in the file name.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFDatePrivate GSGFDatePrivate;
struct _GSGFDatePrivate {
        GList *dates;
};

#define GSGF_DATE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_DATE, GSGFDatePrivate))

G_DEFINE_TYPE (GSGFDate, gsgf_date, GSGF_TYPE_SIMPLE_TEXT)

static gboolean
gsgf_date_set_value (GSGFText *self, const gchar *value,
                     gboolean copy, GError **error);
static void gsgf_date_sync_text (GSGFDate *self);

static void 
gsgf_date_init (GSGFDate *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_DATE, GSGFDatePrivate);

        self->priv->dates = NULL;
}

static void
gsgf_date_finalize (GObject *object)
{
        GSGFDate *self = GSGF_DATE (object);

        if (self->priv->dates) {
                g_list_foreach (self->priv->dates, (GFunc) g_date_free, NULL);
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
 * @date: An initial #GDate to store or %NULL.
 *
 * Create a new #GSGFDate.  You can initialize a #GSGFDate only with one
 * value.  Use gsgf_date_append() for events that span over multiple days.
 *
 * The #GDate you pass is <emphasis>not</emphasis> copied.  You should not
 * free it yourself!
 *
 * Returns: The new #GSGFDate.
 */
GSGFDate *
gsgf_date_new (GDate* date)
{
        GSGFDate *self = g_object_new (GSGF_TYPE_DATE, NULL);

        if (date)
                self->priv->dates = g_list_append (self->priv->dates, date);

        return self;
}

static gboolean
gsgf_date_set_value (GSGFText *_self, const gchar *value,
                     gboolean copy, GError **error)
{
        GSGFDate *self = GSGF_DATE (_self);
        GList *dates = NULL;

        if (error)
                *error = NULL;

        dates = g_list_append (dates, g_date_new_dmy (25, 3, 1976));

        if (self->priv->dates) {
                g_list_foreach (self->priv->dates, (GFunc) self->priv->dates,
                                NULL);
                g_list_free (self->priv->dates);
        }
        self->priv->dates = dates;

        gsgf_date_sync_text (self);

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

        g_return_val_if_fail (GSGF_IS_RAW (raw), NULL);

        if (error)
                *error = NULL;

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
        gchar *text;

        text = g_strdup ("2010-01-30,31,02-01");

        text_class = g_type_class_peek_parent (GSGF_RESULT_GET_CLASS (self));
        text_class->set_value (GSGF_TEXT (self), text, TRUE, NULL);

        g_free (text);
}

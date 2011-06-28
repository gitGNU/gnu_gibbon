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
 * SECTION:gibbon-geo-ip-updater
 * @short_description: Class for updating the Gibbon GeoIP database.
 *
 * Since: 0.1.1
 *
 * Class for updating the Gibbon GeoIP database.
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-geo-ip-updater.h"
/* Ouch, change that name.  */
#include "gibbon-geoip-data.h"

typedef struct _GibbonGeoIPUpdaterPrivate GibbonGeoIPUpdaterPrivate;
struct _GibbonGeoIPUpdaterPrivate {
        const GibbonApp *app;
        GibbonDatabase *database;

        GtkWidget *dialog;
        gulong cancel_handler;
};

#define GIBBON_GEO_IP_UPDATER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate))

G_DEFINE_TYPE (GibbonGeoIPUpdater, gibbon_geo_ip_updater, G_TYPE_OBJECT)

static void gibbon_geo_ip_updater_on_cancel (GibbonGeoIPUpdater *self);

static void 
gibbon_geo_ip_updater_init (GibbonGeoIPUpdater *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate);

        self->priv->app = NULL;
        self->priv->database = NULL;

        self->priv->dialog = NULL;
        self->priv->cancel_handler = 0;
}

static void
gibbon_geo_ip_updater_finalize (GObject *object)
{
        GibbonGeoIPUpdater *self = GIBBON_GEO_IP_UPDATER (object);

        if (self->priv->dialog)
                gtk_widget_destroy (self->priv->dialog);

        G_OBJECT_CLASS (gibbon_geo_ip_updater_parent_class)->finalize(object);
}

static void
gibbon_geo_ip_updater_class_init (GibbonGeoIPUpdaterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGeoIPUpdaterPrivate));

        object_class->finalize = gibbon_geo_ip_updater_finalize;
}

/**
 * gibbon_geo_ip_updater_new:
 * @app: The #GibbonApp.
 * @database: The #GibbonDatabase.
 *
 * Creates a new #GibbonGeoIPUpdater.
 *
 * Returns: The newly created #GibbonGeoIPUpdater or %NULL in case of failure.
 */
GibbonGeoIPUpdater *
gibbon_geo_ip_updater_new (const GibbonApp *app,
                           GibbonDatabase *database,
                           gint64 _last_update)
{
        GibbonGeoIPUpdater *self = g_object_new (GIBBON_TYPE_GEO_IP_UPDATER, NULL);
        GtkWindow *main_window;
        GCallback callback;
        guint64 diff;
        GtkDialogFlags flags;
        GtkMessageType mtype;
        GtkButtonsType btype;
        guint64 months;
        gchar *question;
        gint64 last_update;
        gint reply;
        gboolean download = FALSE;
        gchar *uri;

        self->priv->app = app;
        self->priv->database = database;

        main_window = GTK_WINDOW (gibbon_app_get_window (app));

        if (_last_update == 0)
                last_update = GIBBON_GEOIP_DATA_UPDATE;
        else
                last_update = _last_update;

        diff = (gint64) time (NULL) - last_update;
        if (diff > 30 * 24 * 60 * 60) {
                months = diff / (30 * 24 * 60 * 60);
                flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
                mtype = GTK_MESSAGE_QUESTION;
                btype = GTK_BUTTONS_YES_NO;
                if (_last_update) {
                        question = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                    "Your database used for"
                                                    " locating other players"
                                                    " geographically is"
                                                    " more than one month old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    "Your database used for"
                                                    " locating other players"
                                                    " geographically is"
                                                    " more than %llu months old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    months), months);
                } else {
                        question = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                    "The database used for"
                                                    " locating other players"
                                                    " geographically that"
                                                    " was installed with this"
                                                    " program is more than"
                                                    " one month old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    "The database used for"
                                                    " locating other players"
                                                    " geographically that"
                                                    " was installed with this"
                                                    " program is more than"
                                                    " %llu months old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    months), months);
                }

                self->priv->dialog = gtk_message_dialog_new (main_window,
                                                             flags, mtype,
                                                             btype,
                                                             "%s", question);
                g_free (question);
                gtk_dialog_set_default_response (GTK_DIALOG (self->priv->dialog),
                                                 GTK_RESPONSE_YES);
                reply = gtk_dialog_run (GTK_DIALOG (self->priv->dialog));
                gtk_widget_destroy (self->priv->dialog);
                self->priv->dialog = NULL;

                if (reply == GTK_RESPONSE_YES) {
                        download = TRUE;
                } else if (_last_update) {
                        g_object_unref (self);
                        return NULL;
                }
        }

#define GEO_IP_DEFAULT_URI "http://gibbon.guido-flohr.net/ip2country.csv.gz"
        if (download) {
                uri = g_strdup (GEO_IP_DEFAULT_URI);
        } else {
                uri = g_build_filename (GIBBON_DATADIR, PACKAGE,
                                        "ip2country.csv.gz", NULL);
        }

        self->priv->dialog =
                        gtk_dialog_new_with_buttons (_("Update Geo IP database"),
                                                     main_window,
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     GTK_STOCK_CANCEL,
                                                     GTK_RESPONSE_CANCEL,
                                                     NULL);
        callback = G_CALLBACK (gibbon_geo_ip_updater_on_cancel);
        self->priv->cancel_handler =
                        g_signal_connect_swapped (G_OBJECT (self->priv->dialog),
                                                  "response",
                                                  callback,
                                                  G_OBJECT (self));

        gtk_widget_show_all (self->priv->dialog);

        g_free (uri);

        return self;
}

static void
gibbon_geo_ip_updater_on_cancel (GibbonGeoIPUpdater *self)
{
        if (GIBBON_IS_GEO_IP_UPDATER (self)) {
                gibbon_app_display_info (self->priv->app, "%s",
                                         _("The information about other"
                                           " players' geographic location"
                                           " will not be accurate."));
                g_object_unref (self);
        }
}

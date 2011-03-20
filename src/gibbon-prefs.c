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
 * SECTION:gibbon-prefs
 * @short_description: Management of user preferences!
 *
 * Since: 0.1.1
 *
 * The #GibbonPrefs object is used to store user'preferences, query them,
 * and to intialize the GUI according to these preferences.
 **/

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glib/gi18n.h>

#include "gibbon-prefs.h"

typedef struct _GibbonPrefsPrivate GibbonPrefsPrivate;
struct _GibbonPrefsPrivate {
        GConfClient *client;
};

#define GIBBON_PREFS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_PREFS, GibbonPrefsPrivate))

G_DEFINE_TYPE (GibbonPrefs, gibbon_prefs, G_TYPE_OBJECT)

#define GIBBON_GCONF_PREFIX "/apps/gibbon/"
#define GIBBON_GCONF_PREFS_PREFIX "/apps/gibbon/preferences/"
#define GIBBON_GCONF_SERVER_PREFS_PREFIX "/apps/gibbon/preferences/server/"
#define GIBBON_GCONF_DEBUG_PREFS_PREFIX \
        "/apps/gibbon/preferences/debugging/"

static const gchar *gibbon_prefs_get_string_key (const GibbonPrefs *self,
                                                 enum GibbonPrefsString key);
static const gchar *gibbon_prefs_get_int_key (const GibbonPrefs *self,
                                              enum GibbonPrefsInt key);
static const gchar *gibbon_prefs_get_boolean_key (const GibbonPrefs *self,
                                                  enum GibbonPrefsBoolean key);

static void gibbon_prefs_init (GibbonPrefs *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GIBBON_TYPE_PREFS, GibbonPrefsPrivate);

        self->priv->client = NULL;
}

static void gibbon_prefs_finalize (GObject *object)
{
        GibbonPrefs *self = GIBBON_PREFS (object);

        if (self->priv->client)
                g_object_unref (self->priv->client);
        self->priv->client = NULL;

        G_OBJECT_CLASS (gibbon_prefs_parent_class)->finalize (object);
}

static void gibbon_prefs_class_init (GibbonPrefsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonPrefsPrivate));

        object_class->finalize = gibbon_prefs_finalize;
}

GibbonPrefs *
gibbon_prefs_new ()
{
        GibbonPrefs *self = g_object_new (GIBBON_TYPE_PREFS, NULL);

        self->priv->client = gconf_client_get_default ();

        return self;
}

static const gchar *
gibbon_prefs_get_string_key (const GibbonPrefs *self, 
                             enum GibbonPrefsString key)
{
        switch (key) {
                case GIBBON_PREFS_HOST:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "host";
                case GIBBON_PREFS_LOGIN:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "login";
                case GIBBON_PREFS_PASSWORD:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "password";
                case GIBBON_PREFS_MAIL_ADDRESS:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "address";
        }

        g_return_val_if_reached (NULL);
}

static const gchar *
gibbon_prefs_get_boolean_key (const GibbonPrefs *self,
                              enum GibbonPrefsBoolean key)
{
        switch (key) {
                case GIBBON_PREFS_SAVE_PASSWORD:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "save_pwd";
                case GIBBON_PREFS_DEBUG_TIMESTAMPS:
                        return GIBBON_GCONF_DEBUG_PREFS_PREFIX "timestamps";
                case GIBBON_PREFS_DEBUG_SERVER_COMM:
                        return GIBBON_GCONF_DEBUG_PREFS_PREFIX
                               "server_communication";
        }

        g_return_val_if_reached (NULL);
}

static const gchar *
gibbon_prefs_get_int_key (const GibbonPrefs *self,
                          enum GibbonPrefsInt key)
{
        switch (key) {
                case GIBBON_PREFS_PORT:
                        return GIBBON_GCONF_SERVER_PREFS_PREFIX "port";
        }

        g_return_val_if_reached (NULL);
}

gchar *
gibbon_prefs_get_string (const GibbonPrefs *self, enum GibbonPrefsString key)
{
        const gchar *conf_key;

        g_return_val_if_fail (GIBBON_IS_PREFS (self), NULL);

        conf_key = gibbon_prefs_get_string_key (self, key);
        if (!conf_key)
                return NULL;

        return gconf_client_get_string (self->priv->client, conf_key, NULL);
}

void
gibbon_prefs_set_string (const GibbonPrefs *self,
                         enum GibbonPrefsString key, const gchar *value)
{
        const gchar *conf_key;

        g_return_if_fail (GIBBON_IS_PREFS (self));

        conf_key = gibbon_prefs_get_string_key (self, key);
        if (!conf_key)
                return;

        if (value && *value)
                gconf_client_set_string (self->priv->client, conf_key, value,
                                         NULL);
        else
                gconf_client_unset (self->priv->client, conf_key, NULL);
}

void
gibbon_prefs_string_update_entry (const GibbonPrefs *self, GtkEntry *entry,
                                  enum GibbonPrefsString key)
{
        gchar *value;

        g_return_if_fail (GIBBON_IS_PREFS (self));
        g_return_if_fail (GTK_IS_ENTRY (entry));

        value = gibbon_prefs_get_string (self, key);
        if (value && *value)
                gtk_entry_set_text (entry, value);
        else
                gtk_entry_set_text (entry, "");

        if (value)
                g_free (value);
}

const gchar *
gibbon_prefs_string_read_entry (GibbonPrefs *self, GtkEntry *entry,
                                enum GibbonPrefsString key,
                                gboolean literally)
{
        g_return_val_if_fail (GIBBON_IS_PREFS (self), "");
        g_return_val_if_fail (GTK_IS_ENTRY (entry), "");
        gchar *trimmed;
        const gchar *value;

        if (!literally) {
                trimmed = pango_trim_string (gtk_entry_get_text (entry));
                gtk_entry_set_text (GTK_ENTRY (entry), trimmed);
                g_free (trimmed);
        }

        value = gtk_entry_get_text (entry);
        gibbon_prefs_set_string (self, key, value);

        return value;
}

gboolean
gibbon_prefs_get_boolean (const GibbonPrefs *self, enum GibbonPrefsBoolean key)
{
        const gchar *conf_key;

        g_return_val_if_fail (GIBBON_IS_PREFS (self), FALSE);

        conf_key = gibbon_prefs_get_boolean_key (self, key);
        if (!conf_key)
                return FALSE;

        return gconf_client_get_bool (self->priv->client, conf_key, NULL);
}

void
gibbon_prefs_set_boolean (const GibbonPrefs *self,
                          enum GibbonPrefsBoolean key, gboolean value)
{
        const gchar *conf_key;

        g_return_if_fail (GIBBON_IS_PREFS (self));

        conf_key = gibbon_prefs_get_boolean_key (self, key);
        if (!conf_key)
                return;

        gconf_client_set_bool (self->priv->client, conf_key, value, NULL);
}

void
gibbon_prefs_boolean_update_toggle_button (const GibbonPrefs *self,
                                           GtkToggleButton *toggle,
                                           enum GibbonPrefsBoolean key)
{
        gboolean value;

        g_return_if_fail (GIBBON_IS_PREFS (self));
        g_return_if_fail (GTK_IS_TOGGLE_BUTTON (toggle));

        value = gibbon_prefs_get_boolean (self, key);
        gtk_toggle_button_set_active (toggle, value);
}

gboolean
gibbon_prefs_boolean_read_toggle_button (GibbonPrefs *self,
                                         GtkToggleButton *button,
                                         enum GibbonPrefsBoolean key)
{
        gboolean retval;

        g_return_val_if_fail (GIBBON_IS_PREFS (self), FALSE);
        g_return_val_if_fail (GTK_IS_TOGGLE_BUTTON (button), FALSE);

        retval = gtk_toggle_button_get_active (button);
        gibbon_prefs_set_boolean (self, key, retval);

        return retval;
}

void
gibbon_prefs_set_int (const GibbonPrefs *self, enum GibbonPrefsInt key,
                      gint value)
{
        const gchar *conf_key;

        g_return_if_fail (GIBBON_IS_PREFS (self));

        conf_key = gibbon_prefs_get_int_key (self, key);
        if (!conf_key)
                return;

        gconf_client_set_int (self->priv->client, conf_key, value, NULL);
}

gboolean
gibbon_prefs_get_int (const GibbonPrefs *self, enum GibbonPrefsInt key)
{
        const gchar *conf_key;

        g_return_val_if_fail (GIBBON_IS_PREFS (self), FALSE);

        conf_key = gibbon_prefs_get_int_key (self, key);
        if (!conf_key)
                return FALSE;

        return gconf_client_get_int (self->priv->client, conf_key, NULL);
}


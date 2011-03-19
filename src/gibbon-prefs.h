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

#ifndef _GIBBON_PREFS_H
# define _GIBBON_PREFS_H

#include <gtk/gtk.h>

#define GIBBON_TYPE_PREFS \
        (gibbon_prefs_get_type ())
#define GIBBON_PREFS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_PREFS, \
                GibbonPrefs))
#define GIBBON_PREFS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_PREFS, GibbonPrefsClass))
#define GIBBON_IS_PREFS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_PREFS))
#define GIBBON_IS_PREFS_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_PREFS))
#define GIBBON_PREFS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_PREFS, GibbonPrefsClass))

/**
 * GibbonPrefs:
 *
 * One instance of a #GibbonPrefs.  All properties are private.
 **/
typedef struct _GibbonPrefs GibbonPrefs;
struct _GibbonPrefs
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonPrefsPrivate *priv;
};

/**
 * GibbonPrefsClass:
 *
 * Management of user preferences!
 **/
typedef struct _GibbonPrefsClass GibbonPrefsClass;
struct _GibbonPrefsClass
{
        /* <private >*/
        GObjectClass parent_class;
};

enum GibbonPrefsString {
        GIBBON_PREFS_HOST,
        GIBBON_PREFS_LOGIN,
        GIBBON_PREFS_PASSWORD,
        GIBBON_PREFS_MAIL_ADDRESS,
};

enum GibbonPrefsInt {
        GIBBON_PREFS_PORT,
};

enum GibbonPrefsBoolean {
        GIBBON_PREFS_SAVE_PASSWORD,
        GIBBON_PREFS_DEBUG_TIMESTAMPS,
};

GType gibbon_prefs_get_type (void) G_GNUC_CONST;

GibbonPrefs *gibbon_prefs_new (void);

gchar *gibbon_prefs_get_string (const GibbonPrefs *self,
                                enum GibbonPrefsString key);
void gibbon_prefs_set_string (const GibbonPrefs *self,
                              enum GibbonPrefsString key,
                              const gchar *value);
void gibbon_prefs_string_update_entry (const GibbonPrefs *self, GtkEntry *entry,
                                       enum GibbonPrefsString key);
const gchar *gibbon_prefs_string_read_entry (GibbonPrefs *self,
                                             GtkEntry *entry,
                                             enum GibbonPrefsString key,
                                             gboolean literally);

gint gibbon_prefs_get_int (const GibbonPrefs *self, enum GibbonPrefsInt key);
void gibbon_prefs_set_int (const GibbonPrefs *self, enum GibbonPrefsInt key,
                           gint value);

gboolean gibbon_prefs_get_boolean (const GibbonPrefs *self,
                                   enum GibbonPrefsBoolean key);
void gibbon_prefs_set_boolean (const GibbonPrefs *self,
                               enum GibbonPrefsBoolean key,
                               gboolean value);
void gibbon_prefs_boolean_update_toggle_button (const GibbonPrefs *self,
                                                GtkToggleButton *toggle,
                                                enum GibbonPrefsBoolean key);
gboolean gibbon_prefs_boolean_read_toggle_button (GibbonPrefs *self,
                                                  GtkToggleButton *button,
                                                  enum GibbonPrefsBoolean key);

#endif

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

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-prefs.h"

typedef struct _GibbonPrefsPrivate GibbonPrefsPrivate;
struct _GibbonPrefsPrivate {
        GtkBuilder *builder;
};

#define GIBBON_PREFS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_PREFS, GibbonPrefsPrivate))

G_DEFINE_TYPE (GibbonPrefs, gibbon_prefs, G_TYPE_OBJECT)

static void 
gibbon_prefs_init (GibbonPrefs *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_PREFS, GibbonPrefsPrivate);

        self->priv->builder = NULL;
}

static void
gibbon_prefs_finalize (GObject *object)
{
        GibbonPrefs *self = GIBBON_PREFS (object);

        self->priv->builder = NULL;

        G_OBJECT_CLASS (gibbon_prefs_parent_class)->finalize(object);
}

static void
gibbon_prefs_class_init (GibbonPrefsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonPrefsPrivate));

        object_class->finalize = gibbon_prefs_finalize;
}

GibbonPrefs *
gibbon_prefs_new (GtkBuilder *builder)
{
        GibbonPrefs *self = g_object_new (GIBBON_TYPE_PREFS, NULL);

        self->priv->builder = builder;

        return self;
}

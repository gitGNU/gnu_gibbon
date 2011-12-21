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
 * SECTION:gibbon-action
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.0
 *
 * FIXME! Long description missing!
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-action.h"

typedef struct _GibbonActionPrivate GibbonActionPrivate;
struct _GibbonActionPrivate {
        /* FIXME! Replace with the real structure of the private data! */
        gchar *dummy;
};

#define GIBBON_ACTION_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ACTION, GibbonActionPrivate))

G_DEFINE_TYPE (GibbonAction, gibbon_action, G_TYPE_OBJECT)

static void 
gibbon_action_init (GibbonAction *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ACTION, GibbonActionPrivate);

        /* FIXME! Initialize private data! */
        self->priv->dummy = NULL;
}

static void
gibbon_action_finalize (GObject *object)
{
        GibbonAction *self = GIBBON_ACTION (object);

        /* FIXME! Free private data! */
        if (self->priv->dummy)
                g_free (self->priv->dummy);
        self->priv->dummy = NULL;

        G_OBJECT_CLASS (gibbon_action_parent_class)->finalize(object);
}

static void
gibbon_action_class_init (GibbonActionClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonActionPrivate));

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GibbonAction_do_that; */

        object_class->finalize = gibbon_action_finalize;
}

/**
 * gibbon_action_new:
 * @dummy: The argument.
 *
 * Creates a new #GibbonAction.
 *
 * Returns: The newly created #GibbonAction or %NULL in case of failure.
 */
GibbonAction *
gibbon_action_new (/* FIXME! Argument list! */ const gchar *dummy)
{
        GibbonAction *self = g_object_new (GIBBON_TYPE_ACTION, NULL);

        /* FIXME! Initialize private data! */
        /* self->priv->dummy = g_strdup (dummy); */

        return self;
}

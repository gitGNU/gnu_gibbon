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
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.1
 *
 * FIXME! Long description missing!
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFDatePrivate GSGFDatePrivate;
struct _GSGFDatePrivate {
        /* FIXME! Replace with the real structure of the private data! */
        gchar *dummy;
};

#define GSGF_DATE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_DATE, GSGFDatePrivate))

G_DEFINE_TYPE (GSGFDate, gsgf_date, GSGF_TYPE_SIMPLE_TEXT)

static void 
gsgf_date_init (GSGFDate *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_DATE, GSGFDatePrivate);

        /* FIXME! Initialize private data! */
        self->priv->dummy = NULL;
}

static void
gsgf_date_finalize (GObject *object)
{
        GSGFDate *self = GSGF_DATE (object);

        /* FIXME! Free private data! */
        if (self->priv->dummy)
                g_free (self->priv->dummy);
        self->priv->dummy = NULL;

        G_OBJECT_CLASS (gsgf_date_parent_class)->finalize(object);
}

static void
gsgf_date_class_init (GSGFDateClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFSimpleTextClass *gsgf_simple_text_class = GSGF_SIMPLE_TEXT_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gsgf_simple_text_class->do_this = gsgf_date_do_this; */
        
        g_type_class_add_private(klass, sizeof (GSGFDatePrivate));

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GSGFDate_do_that; */

        object_class->finalize = gsgf_date_finalize;
}

GSGFDate *
gsgf_date_new (/* FIXME! Argument list! */ const gchar *dummy)
{
        GSGFDate *self = g_object_new (GSGF_TYPE_DATE, NULL);

        /* FIXME! Initialize private data! */
        /* self->priv->dummy = g_strdup(dummy); */

        return self;
}

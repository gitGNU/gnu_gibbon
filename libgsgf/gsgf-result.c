/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gsgf-result
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.1
 *
 * FIXME! Long description missing!
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFResultPrivate GSGFResultPrivate;
struct _GSGFResultPrivate {
        /* FIXME! Replace with the real structure of the private data! */
        gchar *dummy;
};

#define GSGF_RESULT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_RESULT, GSGFResultPrivate))

G_DEFINE_TYPE (GSGFResult, gsgf_result, GSGF_TYPE_SIMPLE_TEXT)

static void 
gsgf_result_init (GSGFResult *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_RESULT, GSGFResultPrivate);

        /* FIXME! Initialize private data! */
        self->priv->dummy = NULL;
}

static void
gsgf_result_finalize (GObject *object)
{
        GSGFResult *self = GSGF_RESULT (object);

        /* FIXME! Free private data! */
        if (self->priv->dummy)
                g_free (self->priv->dummy);
        self->priv->dummy = NULL;

        G_OBJECT_CLASS (gsgf_result_parent_class)->finalize(object);
}

static void
gsgf_result_class_init (GSGFResultClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFSimpleTextClass *gsgf_simple_text_class = GSGF_SIMPLE_TEXT_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gsgf_simple_text_class->do_this = gsgf_result_do_this; */
        
        g_type_class_add_private(klass, sizeof (GSGFResultPrivate));

        /* FIXME! Initialize pointers to methods! */
        /* klass->do_that = GSGFResult_do_that; */

        object_class->finalize = gsgf_result_finalize;
}

GSGFResult *
gsgf_result_new (/* FIXME! Argument list! */ const gchar *dummy)
{
        GSGFResult *self = g_object_new (GSGF_TYPE_RESULT, NULL);

        /* FIXME! Initialize private data! */
        /* self->priv->dummy = g_strdup(dummy); */

        return self;
}

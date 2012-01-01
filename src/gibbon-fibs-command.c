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
 * SECTION:gibbon-fibs-command
 * @short_description: Entry for an output command queue!
 *
 * Since: 0.1.0
 *
 * We have to queue commands to the server.  Each individual element in the
 * queue is represented by an GibbonFIBSCommand.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <string.h>

#include "gibbon-fibs-command.h"

typedef struct _GibbonFIBSCommandPrivate GibbonFIBSCommandPrivate;
struct _GibbonFIBSCommandPrivate {
        gchar *line;
        gboolean is_manual;
        size_t length;
        gsize offset;
};

#define GIBBON_FIBS_COMMAND_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_FIBS_COMMAND, GibbonFIBSCommandPrivate))

G_DEFINE_TYPE (GibbonFIBSCommand, gibbon_fibs_command, G_TYPE_OBJECT)

static void 
gibbon_fibs_command_init (GibbonFIBSCommand *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_FIBS_COMMAND, GibbonFIBSCommandPrivate);

        self->priv->line = NULL;
        self->priv->is_manual = FALSE;
        self->priv->length = 0;
        self->priv->offset = 0;
}

static void
gibbon_fibs_command_finalize (GObject *object)
{
        GibbonFIBSCommand *self = GIBBON_FIBS_COMMAND (object);

        if (self->priv->line)
                g_free (self->priv->line);

        self->priv->length = 0;
        self->priv->is_manual = FALSE;
        self->priv->offset = 0;

        G_OBJECT_CLASS (gibbon_fibs_command_parent_class)->finalize (object);
}

static void
gibbon_fibs_command_class_init (GibbonFIBSCommandClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonFIBSCommandPrivate));

        object_class->finalize = gibbon_fibs_command_finalize;
}

/**
 * gibbon_fibs_command_new:
 * @line: Command to send.
 * @is_manual: %TRUE if sent manually, %FALSE otherwise.
 *
 * Creates a new #GibbonFIBSCommand.
 *
 * Returns: The newly created #GibbonFIBSCommand or %NULL in case of failure.
 */
GibbonFIBSCommand *
gibbon_fibs_command_new (const gchar *line, gboolean is_manual)
{
        GibbonFIBSCommand *self = g_object_new (GIBBON_TYPE_FIBS_COMMAND, NULL);

        self->priv->line = g_strdup (line);
        self->priv->length = strlen (line);
        self->priv->offset = 0;
        self->priv->is_manual = is_manual;

        return self;
}

const gchar *
gibbon_fibs_command_get_pointer (const GibbonFIBSCommand *self)
{
        g_return_val_if_fail (GIBBON_IS_FIBS_COMMAND (self), NULL);

        return self->priv->line + self->priv->offset;
}

gsize
gibbon_fibs_command_get_pending (const GibbonFIBSCommand *self)
{
        g_return_val_if_fail (GIBBON_IS_FIBS_COMMAND (self), 0);

        if (self->priv->offset >= self->priv->length)
                return 0;

        return self->priv->length - self->priv->offset;
}

void
gibbon_fibs_command_write (GibbonFIBSCommand *self, gsize num_bytes)
{
        g_return_if_fail (GIBBON_IS_FIBS_COMMAND (self));

        self->priv->offset += num_bytes;

        g_return_if_fail (self->priv->offset <= self->priv->length);
}

gboolean
gibbon_fibs_command_is_manual (const GibbonFIBSCommand *self)
{
        g_return_val_if_fail (GIBBON_IS_FIBS_COMMAND (self), TRUE);

        return self->priv->is_manual;
}

const gchar *
gibbon_fibs_command_get_line (const GibbonFIBSCommand *self)
{
        g_return_val_if_fail (GIBBON_IS_FIBS_COMMAND (self), NULL);

        return self->priv->line;
}

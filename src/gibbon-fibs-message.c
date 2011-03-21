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
 * SECTION:gibbon-fibs-message
 * @short_description: Boxed type for FIBS messages!
 *
 * Since: 0.1.1
 *
 * A message on FIBS has a sender and a text.  The text can not contain
 * newlines.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-fibs-message.h"
#include "html-entities.h"

static GibbonFIBSMessage *gibbon_fibs_message_copy (GibbonFIBSMessage *self);

G_DEFINE_BOXED_TYPE (GibbonFIBSMessage, gibbon_fibs_message,            \
                     gibbon_fibs_message_copy, gibbon_fibs_message_free)

static GibbonFIBSMessage *
gibbon_fibs_message_copy (GibbonFIBSMessage *self)
{
        GibbonFIBSMessage *copy = g_malloc (sizeof *self);

        copy->sender = g_strdup (self->sender);
        copy->message = g_strdup (self->message);

        return copy;
}

void
gibbon_fibs_message_free (GibbonFIBSMessage *self)
{
        if (self) {
                if (self->sender)
                        g_free (self->sender);
                if (self->message)
                        g_free (self->message);
                g_free (self);
        }
}

GibbonFIBSMessage *
gibbon_fibs_message_new (const gchar *raw)
{
        GibbonFIBSMessage *self;
        gchar **tokens = g_strsplit_set (raw, " \t\r\v\f", 2);

        if (!tokens)
                return NULL;

        self = g_malloc (sizeof *self);
        self->sender = g_strdup (tokens[0]);
        if (!self->sender) {
                g_free (self);
                return NULL;
        }
        self->message = g_strdup (tokens[1]);
        if (!self->message) {
                g_free (self->sender);
                g_free (self);
                return NULL;
        }

        g_strfreev (tokens);

        return self;
}

gchar *
gibbon_fibs_message_formatted (const GibbonFIBSMessage *self)
{
        return decode_html_entities (self->message);
}

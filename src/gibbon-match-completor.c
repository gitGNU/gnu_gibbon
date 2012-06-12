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
 * SECTION:gibbon-match-completor
 * @short_description: Guess missing game actions.
 *
 * Since: 0.2.0
 *
 * Recording online matches can suffer from network problems.  It is possible
 * that the server has recorded a certain match action but was not able
 * to transmit that action.
 *
 * A #GibbonMatchCompletor tries to remedy this by filling up the gap or gaps.
 * It only tries to fill up small gaps that can be computed in reasonable time.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-completor.h"

typedef struct _GibbonMatchCompletorPrivate GibbonMatchCompletorPrivate;
struct _GibbonMatchCompletorPrivate {
        const GibbonPosition *target;
};

#define GIBBON_MATCH_COMPLETOR_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_COMPLETOR, GibbonMatchCompletorPrivate))

G_DEFINE_TYPE (GibbonMatchCompletor, gibbon_match_completor, G_TYPE_OBJECT)

static void 
gibbon_match_completor_init (GibbonMatchCompletor *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH_COMPLETOR, GibbonMatchCompletorPrivate);

        self->priv->target = NULL;
}

static void
gibbon_match_completor_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_completor_parent_class)->finalize(object);
}

static void
gibbon_match_completor_class_init (GibbonMatchCompletorClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchCompletorPrivate));

        object_class->finalize = gibbon_match_completor_finalize;
}

/**
 * gibbon_match_completor_new:
 * @target: The #GibbonPosition we want to get to.
 *
 * Creates a new #GibbonMatchCompletor.
 *
 * Returns: The newly created #GibbonMatchCompletor.
 */
GibbonMatchCompletor *
gibbon_match_completor_new (const GibbonPosition *target)
{
        GibbonMatchCompletor *self = g_object_new (GIBBON_TYPE_MATCH_COMPLETOR,
                                                   NULL);

        self->priv->target = target;

        return self;
}

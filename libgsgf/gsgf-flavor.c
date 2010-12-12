/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gsgf-flavor
 * @short_description: A particular flavor of SGF.
 *
 * A #GSGFFlavor defines how to handle properties for a particular kind
 * of SGF.  Different games have different sets of properties.  In SGF
 * files, the flavor is defined by the GM property.
 *
 * This #GSFFlavor class is actually internal to libgsgf.  You only have to bother
 * about these details if you want to implement your own flavors of SGF.
 *
 * When you want to add your own flavor, please check the following list mostly taken
 * from <ulink url="http://www.red-bean.com/sgf/properties.html#FF">
 * http://www.red-bean.com/sgf/properties.html#FF</ulink> for a reference:
 *
 * <itemizedlist>
 *   <listitem><para>0: No type ("")</para></listitem>
 *   <listitem><para>1: Go ("GO")</para></listitem>
 *   <listitem><para>2: Othello ("OTHELLO")</para></listitem>
 *   <listitem><para>3: Chess ("CHESS")</para></listitem>
 *   <listitem><para>4: Gomoku+Renju ("GOMOKO+RENJU")</para></listitem>
 *   <listitem><para>5: Nine Men's Morris ("NINE_MENS_MORRIS")</para></listitem>
 *   <listitem><para>6: Backgammon ("BACKGAMMON")</para></listitem>
 *   <listitem><para>7: Chinese Chess ("CHINESE_CHESS")</para></listitem>
 *   <listitem><para>8: Shogi ("SHOGI")</para></listitem>
 *   <listitem><para>9: Lines of Action ("LINES_OF_ACTION")</para></listitem>
 *   <listitem><para>etc. See the above link for more flavors</para></listitem>
 * </itemizedlist>
 *
 * The strings in parentheses define the libgsgf identifiers for these
 * games.  The default flavor (in absence of an FF property) is 1
 * (for Go). 
 *
 * The flavor 0 is not defined by the SGF specification.  It is used
 * by libgsgf for properties that are common to all other flavors.
 *
 * The only flavors currently implemented are 0 and 6 (for Backgammon).
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFFlavorPrivate {
        const gchar *id;
        const gchar *name;
        const GSGFFlavor *parent;

        /* GSGFCookedValue (*value_constructor) (const GSGFRaw *raw, GError **error); */
        GHashTable *handlers;
};

#define GSGF_FLAVOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_FLAVOR,           \
                                      GSGFFlavorPrivate))

G_DEFINE_TYPE (GSGFFlavor, gsgf_flavor, G_TYPE_OBJECT)

static void
gsgf_flavor_init(GSGFFlavor *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_FLAVOR,
                        GSGFFlavorPrivate);

        self->priv->id = NULL;
        self->priv->name = NULL;
        self->priv->handlers = NULL;
}

static void
gsgf_flavor_finalize(GObject *object)
{
        GSGFFlavor *self = GSGF_FLAVOR (object);

        self->priv->id = NULL;

        self->priv->name = NULL;

        if (self->priv->handlers)
                g_hash_table_destroy(self->priv->handlers);
        self->priv->handlers = NULL;

        G_OBJECT_CLASS (gsgf_flavor_parent_class)->finalize(object);
}

static void
gsgf_flavor_class_init(GSGFFlavorClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFFlavorPrivate));

        object_class->finalize = gsgf_flavor_finalize;
}

/**
 * gsgf_flavor_new:
 * @id: ID for the flavor.
 * @name: Human-readable name.
 *
 * Creates an empty #GSGFFlavor.
 *
 * Returns: The new #GSGFFlavor.
 */
GSGFFlavor *
gsgf_flavor_new (const gchar *id, const gchar *name, const GSGFFlavor *parent)
{
        GSGFFlavor *self = g_object_new(GSGF_TYPE_FLAVOR, NULL);

        self->priv->id = id;
        self->priv->name = name;
        self->priv->handlers = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                     g_free, g_object_unref);
        self->priv->parent = parent;

        return self;
}

gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const gchar *id,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error)
{
        *cooked = gsgf_text_new_from_raw(raw, error);

        if (!*cooked) {
                g_prefix_error(error, _("Property '%s':"), id);
                return FALSE;
        }

        return TRUE;
}

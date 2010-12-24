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
 * @short_description: General SGF properties.
 *
 * A #GSGFFlavor defines how to handle properties for a particular kind
 * of SGF.  Different games have different sets of properties.  In SGF
 * files, the flavor is defined by the GM property.
 *
 * It is the base class for other flavors, and defines properties that are
 * common to all specialized SGF flavors.
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

#include "gsgf-flavor-protected.h"

G_DEFINE_TYPE (GSGFFlavor, gsgf_flavor, G_TYPE_OBJECT)

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const gchar *id,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error);

static GSGFCookedValue *gsgf_flavor_positive_number_new(const GSGFRaw *raw,
                                                        GError **error);

GSGFFlavorTypeDef gsgf_flavor_CA = {
                gsgf_simple_text_new_from_raw, NULL
};

GSGFFlavorTypeDef gsgf_flavor_FF = {
                gsgf_flavor_positive_number_new, NULL
};

GSGFFlavorTypeDef gsgf_flavor_GM = {
                gsgf_flavor_positive_number_new, NULL
};

static GSGFFlavorTypeDef *gsgf_c_handlers[26] = {
                &gsgf_flavor_CA, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_f_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_FF,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_g_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                &gsgf_flavor_GM, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef **gsgf_handlers[26] = {
                NULL,
                NULL,
                gsgf_c_handlers,
                NULL,
                NULL,
                gsgf_f_handlers,
                gsgf_g_handlers,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
};

static void
gsgf_flavor_init(GSGFFlavor *self)
{
}

static void
gsgf_flavor_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_flavor_parent_class)->finalize(object);
}

static void
gsgf_flavor_class_init(GSGFFlavorClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        klass->get_cooked_value = _gsgf_flavor_get_cooked_value;

        object_class->finalize = gsgf_flavor_finalize;
}

/**
 * gsgf_flavor_new:
 *
 * Creates an empty #GSGFFlavor.
 *
 * Returns: The new #GSGFFlavor.
 */
GSGFFlavor *
gsgf_flavor_new (void)
{
        GSGFFlavor *self = g_object_new(GSGF_TYPE_FLAVOR, NULL);

        return self;
}

/**
 * gsgf_flavor_get_cooked_value:
 * @self: The #GSGFFlavor
 * @id: The property id
 * @raw: The #GSGFRaw to cook.
 * @cooked: Location to store the cooked value
 * @error: Optional #GError location or %NULL to ignore.
 *
 * Cook a #GSGFRaw into a #GSGFCooked.
 *
 * This function is internal and only interesting for implementors of new
 * flavors.  A return value of %FALSE does not necessarily mean failure,
 * but can also signify that the specific property id is not defined for
 * the particular flavor.  Check @error for details.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_flavor_get_cooked_value(const GSGFFlavor *self, const gchar *id,
                             const GSGFRaw *raw, GSGFCookedValue **cooked,
                             GError **error)
{
        if (!GSGF_IS_FLAVOR(self)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Invalid cast to GSGFFlavor"));
                return FALSE;
        }

        if (!GSGF_FLAVOR_GET_CLASS(self)->get_cooked_value) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Method get_cooked_value not implemented"));
                return FALSE;
        }

        return GSGF_FLAVOR_GET_CLASS(self)->get_cooked_value(self, id, raw,
                                                             cooked, error);
}

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const gchar *id,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error)
{
        GSGFFlavorTypeDef* def;

        if (id[0] < 'A' || id[0] > 'Z' || id[1] < 'A' || id[1] > 'Z' || id[2] != 0)
                return FALSE;

        if (!gsgf_handlers[id[0] - 'A'])
                return FALSE;

        def = gsgf_handlers[id[0] - 'A'][id[1] - 'A'];

        if (!def)
                return FALSE;

        *cooked = def->constructor(raw, error);

        if (!*cooked) {
                g_prefix_error(error, _("Property '%s': "), id);
                return FALSE;
        }

        return TRUE;
}

static
GSGFCookedValue *gsgf_flavor_positive_number_new(const GSGFRaw *raw, GError **error)
{
        GSGFCookedValue *retval = gsgf_number_new_from_raw(raw, error);
        GSGFNumber *number;

        if (!retval)
                return NULL;

        number = GSGF_NUMBER(retval);
        if (gsgf_number_get_value(number) < 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Value must be greater than 0 but is %lld"),
                            gsgf_number_get_value(number));
                g_object_unref(retval);
                return NULL;
        }

        return retval;
}

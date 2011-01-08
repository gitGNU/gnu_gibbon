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

static GSGFMove *gsgf_flavor_create_move(const GSGFFlavor *self,
                                         const GSGFRaw *raw,
                                         GError **error);
static GSGFPoint *gsgf_flavor_create_point(const GSGFFlavor *self,
                                           const GSGFRaw *raw,
                                           GError **error);

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const GSGFProperty *property,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error);

static GSGFCookedValue *gsgf_B_or_W_new_from_raw(const GSGFRaw* raw,
                                                 const GSGFFlavor *flavor,
                                                 const GSGFProperty *property,
                                                 GError **error);
GSGFFlavorTypeDef gsgf_flavor_B_or_W = {
                gsgf_B_or_W_new_from_raw, {
                                NULL
                }
};

static GSGFCookedValue *gsgf_list_of_points_new_from_raw(const GSGFRaw *raw,
                                                         const GSGFFlavor *flavor,
                                                         const GSGFProperty *property,
                                                         GError **error);
GSGFFlavorTypeDef gsgf_flavor_AB = {
                gsgf_list_of_points_new_from_raw, {
                                NULL
                }
};

static GSGFCookedValue *gsgf_AP_new_from_raw(const GSGFRaw* raw,
                                             const GSGFFlavor *flavor,
                                             const GSGFProperty *property,
                                             GError **error);
GSGFFlavorTypeDef gsgf_flavor_AP = {
                gsgf_AP_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

GSGFFlavorTypeDef gsgf_flavor_CA = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

GSGFFlavorTypeDef gsgf_flavor_FF = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

GSGFFlavorTypeDef gsgf_flavor_GM = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

GSGFFlavorTypeDef gsgf_flavor_KO = {
                gsgf_empty_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

GSGFFlavorTypeDef gsgf_flavor_MN = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static gboolean gsgf_constraint_is_ST_number(const GSGFCookedValue *cooked,
                                             const GSGFRaw *raw,
                                             const GSGFProperty *property,
                                             GError **error);
GSGFFlavorTypeDef gsgf_flavor_ST = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_ST_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFCookedValue *gsgf_SZ_new_from_raw(const GSGFRaw* raw,
                                             const GSGFFlavor *flavor,
                                             const GSGFProperty *property,
                                             GError **error);
GSGFFlavorTypeDef gsgf_flavor_SZ = {
                gsgf_SZ_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef *gsgf_single_char_handlers[26] = {
                NULL, &gsgf_flavor_B_or_W, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, &gsgf_flavor_B_or_W, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_a_handlers[26] = {
                NULL, &gsgf_flavor_AB, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, &gsgf_flavor_AP, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
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

static GSGFFlavorTypeDef *gsgf_k_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &gsgf_flavor_KO, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_m_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_MN, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_s_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_ST, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_SZ,
};

static GSGFFlavorTypeDef **gsgf_handlers[26] = {
                gsgf_a_handlers,
                NULL,
                gsgf_c_handlers,
                NULL,
                NULL,
                gsgf_f_handlers,
                gsgf_g_handlers,
                NULL,
                NULL,
                NULL,
                gsgf_k_handlers,
                NULL,
                gsgf_m_handlers,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                gsgf_s_handlers,
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
        klass->create_move = NULL;
        klass->create_point = NULL;

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
 * @self: The #GSGFFlavor.
 * @property: The #GSGFProperty where the @raw comes from.
 * @raw: The #GSGFRaw to cook.
 * @cooked: Location to store the cooked value.
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
gsgf_flavor_get_cooked_value(const GSGFFlavor *self, const GSGFProperty *property,
                             const GSGFRaw *raw, GSGFCookedValue **cooked,
                             GError **error)
{
        if (!GSGF_IS_FLAVOR(self)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Invalid cast to GSGFFlavor"));
                /* Print standard error message and return.  */
                g_return_val_if_fail(GSGF_IS_FLAVOR(self), FALSE);
        }

        return GSGF_FLAVOR_GET_CLASS(self)->get_cooked_value(self, property, raw,
                                                             cooked, error);
}

static GSGFMove *
gsgf_flavor_create_move(const GSGFFlavor *self,
                        const GSGFRaw *raw,
                        GError **error)
{
        if (!GSGF_IS_FLAVOR(self)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Invalid cast to GSGFFlavor"));
                /* Print standard error message and return.  */
                g_return_val_if_fail(GSGF_IS_FLAVOR(self), FALSE);
        }

        if (!GSGF_FLAVOR_GET_CLASS(self)->create_move) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Method create_move is not implemented"));
                /* Print standard error message and return.  */
                g_return_val_if_fail(GSGF_FLAVOR_GET_CLASS(self)->create_move,
                                     FALSE);
        }

        return GSGF_FLAVOR_GET_CLASS(self)->create_move(self, raw, error);
}

static GSGFPoint *
gsgf_flavor_create_point(const GSGFFlavor *self,
                         const GSGFRaw *raw,
                         GError **error)
{
        if (!GSGF_IS_FLAVOR(self)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Invalid cast to GSGFFlavor"));
                /* Print standard error message and return.  */
                g_return_val_if_fail(GSGF_IS_FLAVOR(self), FALSE);
        }

        if (!GSGF_FLAVOR_GET_CLASS(self)->create_point) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                            _("Method create_point is not implemented"));
                /* Print standard error message and return.  */
                g_return_val_if_fail(GSGF_FLAVOR_GET_CLASS(self)->create_point,
                                     FALSE);
        }

        return GSGF_FLAVOR_GET_CLASS(self)->create_point(self, raw, error);
}

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const GSGFProperty *property,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error)
{
        GSGFFlavorTypeDef *def = NULL;
        GSGFCookedConstraint *constraint;
        const gchar *id;

        id = gsgf_property_get_id(property);
        if (id[0] >= 'A' && id[0] <= 'Z' && id[1] == 0) {
                def = gsgf_single_char_handlers[id[0] - 'A'];
        } else {
                if (id[0] < 'A' || id[0] > 'Z'
                    || id[1] < 'A' || id[1] > 'Z'
                    || id[2] != 0)
                return FALSE;

                if (!gsgf_handlers[id[0] - 'A'])
                        return FALSE;
                def = gsgf_handlers[id[0] - 'A'][id[1] - 'A'];
        }

        if (!def)
                return FALSE;

        *cooked = def->constructor(raw, flavor, property, error);

        if (!*cooked) {
                g_prefix_error(error, _("Property '%s': "), id);
                return FALSE;
        }

        constraint = def->constraints;

        while (*constraint) {
                if (!(*constraint)(*cooked, raw, property, error)) {
                        g_prefix_error(error, _("Property '%s': "), id);
                        g_object_unref(*cooked);
                        return FALSE;
                }
                ++constraint;
        }
        return TRUE;
}

gboolean
gsgf_constraint_is_positive_number(const GSGFCookedValue *value,
                                   const GSGFRaw *raw,
                                   const GSGFProperty *property, GError **error)
{
        GSGFNumber *number;

        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), FALSE);
        g_return_val_if_fail(GSGF_IS_RAW(raw), FALSE);
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE);

        number = GSGF_NUMBER(value);

        if (gsgf_number_get_value(number) < 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Value must be greater than 0 but is %lld"),
                            gsgf_number_get_value(number));
                return FALSE;
        }

        return TRUE;
}

gboolean
gsgf_constraint_is_root_property(const GSGFCookedValue *value,
                                 const GSGFRaw *raw,
                                 const GSGFProperty *property, GError **error)
{
        GSGFNode *node;
        GSGFNode *previous;

        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), FALSE);
        g_return_val_if_fail(GSGF_IS_RAW(raw), FALSE);
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE);

        node = gsgf_property_get_node(property);
        previous = gsgf_node_get_previous_node(node);

        if (previous) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Property only allowed in root node"));
                return FALSE;
        }

        return TRUE;
}

gboolean
gsgf_constraint_is_single_value(const GSGFCookedValue *value,
                                const GSGFRaw *raw,
                                const GSGFProperty *property, GError **error)
{
        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), FALSE);
        g_return_val_if_fail(GSGF_IS_RAW(raw), FALSE);
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE);

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Exactly one value required for property"));
                return FALSE;
        }

        return TRUE;
}

static GSGFCookedValue *
gsgf_AP_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                     const GSGFProperty *property, GError **error)
{
        gchar *raw_string = gsgf_raw_get_value(raw, 0);
        gchar *ap = NULL;
        const gchar *version = NULL;
        GSGFCompose *retval;

        ap = gsgf_util_read_simple_text(raw_string, &version, ':');
        if (!ap || !*ap) {
                if (ap) g_free(ap);
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Empty property"));
                return FALSE;
        }

        if (!version || !version[0] || !version[1] || version == ap) {
                g_free(ap);
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Empty version information"));
                return FALSE;
        }

        retval = gsgf_compose_new(GSGF_COOKED_VALUE(gsgf_simple_text_new(ap)),
                                  GSGF_COOKED_VALUE(gsgf_simple_text_new(version + 1)),
                                  NULL);
        g_free(ap);

        return GSGF_COOKED_VALUE(retval);
}

static gboolean
gsgf_constraint_is_ST_number(const GSGFCookedValue *value,
                             const GSGFRaw *raw,
                             const GSGFProperty *property, GError **error)
{
        GSGFNumber *number;

        g_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), FALSE);
        g_return_val_if_fail(GSGF_IS_RAW(raw), FALSE);
        g_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE);

        number = GSGF_NUMBER(value);

        if (gsgf_number_get_value(number) < 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Value must be greater than 0 but is %lld"),
                            gsgf_number_get_value(number));
                return FALSE;
        }

        return TRUE;
}

static GSGFCookedValue *
gsgf_SZ_new_from_raw(const GSGFRaw *raw, const GSGFFlavor *flavor,
                     const GSGFProperty *property, GError **error)
{
        gchar *raw_string = gsgf_raw_get_value(raw, 0);
        gchar *columns_as_string = NULL;
        const gchar *rows_as_string = NULL;
        GSGFRaw *dummy;
        GSGFCookedValue *cooked;
        GSGFNumber *columns = NULL;
        GSGFNumber *rows = NULL;
        GSGFCookedValue *retval;

        columns_as_string = gsgf_util_read_simple_text(raw_string, &rows_as_string, ':');
        if (!columns_as_string || !*columns_as_string) {
                if (columns_as_string) g_free(columns_as_string);
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Empty property"));
                return FALSE;
        }

        dummy = gsgf_raw_new(columns_as_string);
        cooked = gsgf_number_new_from_raw(dummy, flavor, property, error);
        g_free(columns_as_string);
        g_object_unref(dummy);

        if (!cooked)
                return FALSE;
        columns = GSGF_NUMBER(cooked);

        if (rows_as_string && rows_as_string[0]
            && rows_as_string[1] && rows_as_string != columns_as_string) {
                dummy = gsgf_raw_new(rows_as_string + 1);
                cooked = gsgf_number_new_from_raw(dummy, flavor, property, error);
                g_object_unref(dummy);

                if (!cooked) {
                        g_object_unref(columns);
                        return FALSE;
                }
                rows = GSGF_NUMBER(cooked);
        }

        if (rows) {
                /*
                 * Be graceful when reading, strict when writing.  Two equal values
                 * are not allowed by the SGF specification.  We convert them to
                 * one number.
                 */
                if (gsgf_number_get_value(columns) == gsgf_number_get_value(rows)) {
                        g_object_unref(rows);
                        rows = NULL;
                }
        }

        if (rows) {
                retval = GSGF_COOKED_VALUE(
                                gsgf_compose_new(GSGF_COOKED_VALUE(columns),
                                                 GSGF_COOKED_VALUE(rows),
                                                 NULL));
        } else {
                retval = GSGF_COOKED_VALUE(columns);
        }

        return retval;
}

static GSGFCookedValue *
gsgf_B_or_W_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                         const GSGFProperty *property, GError **error)
{
        GSGFMove *move;
        GSGFNode *node;
        gchar *id;

        id = gsgf_property_get_id(property);
        node = gsgf_property_get_node(property);

        if (id[0] == 'B') {
                if (gsgf_node_get_property(node, "W")) {
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("Only one move property (B or W) in node allowed"));
                        return FALSE;
                }
        } else if (id[0] == 'W') {
                if (gsgf_node_get_property(node, "B")) {
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("Only one move property (B or W) in node allowed"));
                        return FALSE;
                }
        }

        move = gsgf_flavor_create_move(flavor, raw, error);
        if (!move)
                return NULL;

        return GSGF_COOKED_VALUE(move);
}

static GSGFCookedValue *
gsgf_list_of_points_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                                 const GSGFProperty *property, GError **error)
{
        GType type = gsgf_point_get_type();
        GSGFListOf *list_of = gsgf_list_of_new(type);

        return list_of;
}

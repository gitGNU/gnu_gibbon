/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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
 * This #GSGFFlavor class is actually internal to libgsgf.  You only have to bother
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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "gsgf-flavor-protected.h"
#include "gsgf-private.h"

G_DEFINE_TYPE (GSGFFlavor, gsgf_flavor, G_TYPE_OBJECT)

static GSGFMove *gsgf_flavor_create_move(const GSGFFlavor *self,
                                         const GSGFRaw *raw,
                                         GError **error);
static GSGFStone *gsgf_flavor_create_stone(const GSGFFlavor *self,
                                           const GSGFRaw *raw,
                                           gsize i,
                                           GError **error);
static GSGFPoint *gsgf_flavor_create_point(const GSGFFlavor *self,
                                           const GSGFRaw *raw,
                                           gsize i,
                                           GError **error);
static gboolean gsgf_flavor_append_points(const GSGFFlavor *self,
                                          GSGFListOf *list_of,
                                          const GSGFRaw *raw,
                                          gsize i,
                                          GError **error);

static GSGFCookedValue *gsgf_list_of_stones_new_from_raw(const GSGFRaw *raw,
                                                         const GSGFFlavor *flavor,
                                                         const GSGFProperty *property,
                                                         GError **error);
static GSGFCookedValue *gsgf_list_of_points_new_from_raw(const GSGFRaw *raw,
                                                         const GSGFFlavor *flavor,
                                                         const GSGFProperty *property,
                                                         GError **error);
static GSGFCookedValue *gsgf_list_of_point_labels_new_from_raw
        (const GSGFRaw *raw, const GSGFFlavor *flavor,
         const GSGFProperty *property, GError **error);
static GSGFCookedValue *gsgf_elist_of_points_new_from_raw(const GSGFRaw *raw,
                                                          const GSGFFlavor *flavor,
                                                          const GSGFProperty *property,
                                                          GError **error);
static GSGFCookedValue *gsgf_list_of_lines_new_from_raw(const GSGFRaw *raw,
                                                         const GSGFFlavor *flavor,
                                                         const GSGFProperty *property,
                                                         GError **error);
static gboolean gsgf_list_of_points_check_unique(const GSGFListOf *list_of,
                                                 GError **error);

static int compare_gint(const void *a, const void *b);

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const GSGFProperty *property,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error);

static gboolean
gsgf_constraint_node_annotation_unique (const GSGFCookedValue *value,
                                        const GSGFRaw *raw,
                                        const GSGFProperty *property,
                                        GError **error);
static gboolean
gsgf_constraint_move_annotation_unique (const GSGFCookedValue *value,
                                        const GSGFRaw *raw,
                                        const GSGFProperty *property,
                                        GError **error);
static gboolean
gsgf_constraint_move_annotation_with_move (const GSGFCookedValue *value,
                                           const GSGFRaw *raw,
                                           const GSGFProperty *property,
                                           GError **error);

static gboolean
gsgf_constraint_markup_unique (const GSGFCookedValue *value,
                               const GSGFRaw *raw,
                               const GSGFProperty *property,
                               GError **error);

static GSGFCookedValue *gsgf_B_or_W_new_from_raw(const GSGFRaw* raw,
                                                 const GSGFFlavor *flavor,
                                                 const GSGFProperty *property,
                                                 GError **error);
static GSGFFlavorTypeDef gsgf_flavor_B_or_W = {
                gsgf_B_or_W_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_C = {
                gsgf_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_N = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_V = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_AB = {
                gsgf_list_of_stones_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_AE = {
                gsgf_list_of_points_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_AN = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFCookedValue *gsgf_AP_new_from_raw(const GSGFRaw* raw,
                                             const GSGFFlavor *flavor,
                                             const GSGFProperty *property,
                                             GError **error);
static GSGFFlavorTypeDef gsgf_flavor_AP = {
                gsgf_AP_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_AR = {
                gsgf_list_of_lines_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_AW = {
                gsgf_list_of_stones_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_BL = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_BM = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_move_annotation_with_move,
                                gsgf_constraint_move_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_BR = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_BT = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_CA = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_CP = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_CR = {
                gsgf_list_of_points_new_from_raw, {
                                gsgf_constraint_markup_unique,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_DD = {
                gsgf_elist_of_points_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_DM = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_node_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_DO = {
                gsgf_empty_new_from_raw, {
                                gsgf_constraint_move_annotation_with_move,
                                gsgf_constraint_move_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_DT = {
                gsgf_date_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_EV = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_FF = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFCookedValue *gsgf_FG_new_from_raw (const GSGFRaw* raw,
                                              const GSGFFlavor *flavor,
                                              const GSGFProperty *property,
                                              GError **error);
static GSGFFlavorTypeDef gsgf_flavor_FG = {
                gsgf_FG_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_GB = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_node_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_GC = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_GM = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_GN = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_GW = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_node_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_HO = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_IT = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_move_annotation_with_move,
                                gsgf_constraint_move_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_KO = {
                gsgf_empty_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_LB = {
                gsgf_list_of_point_labels_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_LN = {
                gsgf_list_of_lines_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_MA = {
                gsgf_list_of_points_new_from_raw, {
                                gsgf_constraint_markup_unique,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_MN = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_OB = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_ON = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_OT = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_OW = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_PB = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_PC = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_PL = {
                gsgf_color_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_PM = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_positive_number,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_PW = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_RE = {
                gsgf_result_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_RO = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_RU = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_SL = {
                gsgf_list_of_points_new_from_raw, {
                                gsgf_constraint_markup_unique,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_SO = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static gboolean gsgf_constraint_is_ST_number(const GSGFCookedValue *cooked,
                                             const GSGFRaw *raw,
                                             const GSGFProperty *property,
                                             GError **error);
static GSGFFlavorTypeDef gsgf_flavor_ST = {
                gsgf_number_new_from_raw, {
                                gsgf_constraint_is_ST_number,
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_SQ = {
                gsgf_list_of_points_new_from_raw, {
                                gsgf_constraint_markup_unique,
                                NULL
                }
};

static GSGFCookedValue *gsgf_SZ_new_from_raw(const GSGFRaw* raw,
                                             const GSGFFlavor *flavor,
                                             const GSGFProperty *property,
                                             GError **error);
static GSGFFlavorTypeDef gsgf_flavor_SZ = {
                gsgf_SZ_new_from_raw, {
                                gsgf_constraint_is_root_property,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_TE = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_move_annotation_with_move,
                                gsgf_constraint_move_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_TM = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_TR = {
                gsgf_list_of_points_new_from_raw, {
                                gsgf_constraint_markup_unique,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_UC = {
                gsgf_double_new_from_raw, {
                                gsgf_constraint_node_annotation_unique,
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_US = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_VW = {
                gsgf_elist_of_points_new_from_raw, {
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_WL = {
                gsgf_real_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_WR = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef gsgf_flavor_WT = {
                gsgf_simple_text_new_from_raw, {
                                gsgf_constraint_is_single_value,
                                NULL
                }
};

static GSGFFlavorTypeDef *gsgf_single_char_handlers[26] = {
                NULL, &gsgf_flavor_B_or_W, &gsgf_flavor_C, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_N, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, &gsgf_flavor_V, &gsgf_flavor_B_or_W, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_a_handlers[26] = {
                NULL, &gsgf_flavor_AB, NULL, NULL, &gsgf_flavor_AE, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_AN, NULL, &gsgf_flavor_AP, NULL, &gsgf_flavor_AR,
                NULL, NULL, NULL, NULL, &gsgf_flavor_AW, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_b_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_BL,
                &gsgf_flavor_BM, NULL, NULL, NULL, NULL, &gsgf_flavor_BR,
                NULL, &gsgf_flavor_BT, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_c_handlers[26] = {
                &gsgf_flavor_CA, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, &gsgf_flavor_CP, NULL, &gsgf_flavor_CR,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_d_handlers[26] = {
                NULL, NULL, NULL, &gsgf_flavor_DD, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                &gsgf_flavor_DM, NULL, &gsgf_flavor_DO, NULL, NULL, NULL,
                NULL, &gsgf_flavor_DT, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_e_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, &gsgf_flavor_EV, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_f_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_FF,
                &gsgf_flavor_FG, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_g_handlers[26] = {
                NULL, &gsgf_flavor_GB, &gsgf_flavor_GC, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                &gsgf_flavor_GM, &gsgf_flavor_GN, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, &gsgf_flavor_GW, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_h_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &gsgf_flavor_HO, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_i_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_IT, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_k_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &gsgf_flavor_KO, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_l_handlers[26] = {
                NULL, &gsgf_flavor_LB, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_LN, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_m_handlers[26] = {
                &gsgf_flavor_MA, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_MN, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_o_handlers[26] = {
                NULL, &gsgf_flavor_OB, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_ON, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_OT, NULL, NULL, &gsgf_flavor_OW, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_p_handlers[26] = {
                NULL, &gsgf_flavor_PB, &gsgf_flavor_PC, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_PL,
                &gsgf_flavor_PM, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, &gsgf_flavor_PW, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_r_handlers[26] = {
                NULL, NULL, NULL, NULL, &gsgf_flavor_RE, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &gsgf_flavor_RO, NULL, NULL, NULL,
                NULL, NULL, &gsgf_flavor_RU, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_s_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_SL,
                NULL, NULL, &gsgf_flavor_SO, NULL, &gsgf_flavor_SQ, NULL,
                NULL, &gsgf_flavor_ST, NULL, NULL, NULL, NULL,
                NULL, &gsgf_flavor_SZ,
};

static GSGFFlavorTypeDef *gsgf_t_handlers[26] = {
                NULL, NULL, NULL, NULL, &gsgf_flavor_TE, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                &gsgf_flavor_TM, NULL, NULL, NULL, NULL, &gsgf_flavor_TR,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_u_handlers[26] = {
                NULL, NULL, &gsgf_flavor_UC, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                &gsgf_flavor_US, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_v_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, &gsgf_flavor_VW, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef *gsgf_w_handlers[26] = {
                NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_WL,
                NULL, NULL, NULL, NULL, NULL, &gsgf_flavor_WR,
                NULL, &gsgf_flavor_WT, NULL, NULL, NULL, NULL,
                NULL, NULL,
};

static GSGFFlavorTypeDef **gsgf_handlers[26] = {
                gsgf_a_handlers,
                gsgf_b_handlers,
                gsgf_c_handlers,
                gsgf_d_handlers,
                gsgf_e_handlers,
                gsgf_f_handlers,
                gsgf_g_handlers,
                gsgf_h_handlers,
                gsgf_i_handlers,
                NULL,
                gsgf_k_handlers,
                gsgf_l_handlers,
                gsgf_m_handlers,
                NULL,
                gsgf_o_handlers,
                gsgf_p_handlers,
                NULL,
                gsgf_r_handlers,
                gsgf_s_handlers,
                gsgf_t_handlers,
                gsgf_u_handlers,
                gsgf_v_handlers,
                gsgf_w_handlers,
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
        klass->stone_type = G_TYPE_INVALID;
        klass->create_stone = NULL;
        klass->create_point = NULL;
        klass->append_points = NULL;
        klass->write_compressed_list = NULL;

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
 * Cook a #GSGFRaw into a #GSGFCookedValue.
 *
 * This function is internal and only interesting for implementors of new
 * flavors.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_flavor_get_cooked_value(const GSGFFlavor *self, const GSGFProperty *property,
                             const GSGFRaw *raw, GSGFCookedValue **cooked,
                             GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);

        return GSGF_FLAVOR_GET_CLASS(self)->get_cooked_value(self, property, raw,
                                                             cooked, error);
}

static GSGFMove *
gsgf_flavor_create_move(const GSGFFlavor *self,
                        const GSGFRaw *raw,
                        GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_FLAVOR_GET_CLASS(self)->create_move,
                                 FALSE, error);

        return GSGF_FLAVOR_GET_CLASS(self)->create_move(self, raw, error);
}

static GSGFStone *
gsgf_flavor_create_stone(const GSGFFlavor *self,
                         const GSGFRaw *raw,
                         gsize i,
                         GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_FLAVOR_GET_CLASS(self)->create_stone,
                                 FALSE, error);

        return GSGF_FLAVOR_GET_CLASS(self)->create_stone(self, raw, i, error);
}

static GSGFPoint *
gsgf_flavor_create_point(const GSGFFlavor *self,
                         const GSGFRaw *raw,
                         gsize i,
                         GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_FLAVOR_GET_CLASS (self)->create_point,
                                 FALSE, error);

        return GSGF_FLAVOR_GET_CLASS(self)->create_point(self, raw, i, error);
}

static gboolean
gsgf_flavor_append_points(const GSGFFlavor *self,
                          GSGFListOf *list_of,
                          const GSGFRaw *raw,
                          gsize i,
                          GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_FLAVOR_GET_CLASS (self)->append_points,
                                 FALSE, error);
        return GSGF_FLAVOR_GET_CLASS(self)->append_points(self, list_of,
                                                          raw, i, error);
}

/**
 * gsgf_flavor_get_game_id:
 * @self: The #GSGFFlavor.
 * @error: An optional location for storing an error.
 *
 * SGF defines well-known positive ids for supported games, for example
 * 0 for Go or 6 backgammon.
 *
 * Returns: The game id or 0 in case of failure.
 */
guint
gsgf_flavor_get_game_id (const GSGFFlavor *self, GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (self), FALSE, error);

        /* We assume Go if flavor does not override the method.  */
        if (!GSGF_FLAVOR_GET_CLASS(self)->get_game_id)
                return 1;

        return GSGF_FLAVOR_GET_CLASS(self)->get_game_id (self);
}

static gboolean
_gsgf_flavor_get_cooked_value(const GSGFFlavor *flavor, const GSGFProperty *property,
                              const GSGFRaw *raw, GSGFCookedValue **cooked,
                              GError **error)
{
        GSGFFlavorTypeDef *def = NULL;
        GSGFCookedConstraint *constraint;
        const gchar *id;

        gsgf_return_val_if_fail (cooked, FALSE, error);

        *cooked = NULL;

        id = gsgf_property_get_id(property);
        if (id[0] >= 'A' && id[0] <= 'Z' && id[1] == 0) {
                def = gsgf_single_char_handlers[id[0] - 'A'];
        } else {
                if (id[0] < 'A' || id[0] > 'Z'
                    || id[1] < 'A' || id[1] > 'Z'
                    || id[2] != 0)
                return TRUE;

                if (!gsgf_handlers[id[0] - 'A'])
                        return TRUE;
                def = gsgf_handlers[id[0] - 'A'][id[1] - 'A'];
        }

        if (!def) {
                g_warning (_("Unsupported property `%s'."), id);
                return TRUE;
        }

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

        gsgf_return_val_if_fail(GSGF_IS_COOKED_VALUE(value), FALSE, error);
        gsgf_return_val_if_fail(GSGF_IS_RAW(raw), FALSE, error);
        gsgf_return_val_if_fail(GSGF_IS_PROPERTY(property), FALSE, error);

        number = GSGF_NUMBER(value);

        if (gsgf_number_get_value(number) < 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Value must be greater than 0 but is %lld"),
                            (long long int) gsgf_number_get_value(number));
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
        GSGFGameTree *game_tree;

        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE(value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW(raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY(property), FALSE, error);

        node = gsgf_property_get_node(property);
        game_tree = gsgf_node_get_game_tree (node);
        if (gsgf_game_tree_get_parent (game_tree)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Property only allowed in root node"));
                return FALSE;
        }

        previous = gsgf_node_get_previous_node(node);

        if (previous) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Property only allowed in root node"));
                return FALSE;
        }

        return TRUE;
}

/* This function is too special to export it to individual flavors.  */
static gboolean
gsgf_constraint_node_annotation_unique (const GSGFCookedValue *value,
                                        const GSGFRaw *raw,
                                        const GSGFProperty *property,
                                        GError **error)
{
        GSGFNode *node;
        const gchar *id;
        const gchar *ids[4] = { "DM", "GB", "GW", "UC" };
        int i;

        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE(value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW(raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY(property), FALSE, error);

        id = gsgf_property_get_id (property);
        node = gsgf_property_get_node(property);

        for (i = 0; i < 4; ++i) {
                if (strcmp (id, ids[i])) {
                        if (gsgf_node_get_property (node, ids[i])) {
                                g_set_error (error, GSGF_ERROR,
                                             GSGF_ERROR_SEMANTIC_ERROR,
                                             _("The properties 'DM', 'GB',"
                                               " 'GW', and 'UC' are mutually"
                                               " exclusive within one node"));
                                return FALSE;
                        }
                }
        }

        return TRUE;
}

/* This function is too special to export it to individual flavors.  */
static gboolean
gsgf_constraint_move_annotation_unique (const GSGFCookedValue *value,
                                        const GSGFRaw *raw,
                                        const GSGFProperty *property,
                                        GError **error)
{
        GSGFNode *node;
        const gchar *id;
        const gchar *ids[4] = { "BM", "DO", "IT", "TE" };
        int i;

        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE (value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), FALSE, error);

        id = gsgf_property_get_id (property);
        node = gsgf_property_get_node(property);

        for (i = 0; i < 4; ++i) {
                if (strcmp (id, ids[i])) {
                        if (gsgf_node_get_property (node, ids[i])) {
                                g_set_error (error, GSGF_ERROR,
                                             GSGF_ERROR_SEMANTIC_ERROR,
                                             _("The properties 'BM', 'DO',"
                                               " 'IT', and 'TE' are mutually"
                                               " exclusive within one node"));
                                return FALSE;
                        }
                }
        }

        return TRUE;
}

/* This function is too special to export it to individual flavors.  */
static gboolean
gsgf_constraint_markup_unique (const GSGFCookedValue *value,
                               const GSGFRaw *raw,
                               const GSGFProperty *property,
                               GError **error)
{
        GSGFNode *node;
        const gchar *id;
        const gchar *ids[5] = { "CR", "MA", "SQ", "SL", "TR" };
        int i;
        GSGFProperty *other_prop;
        GSGFValue *cooked_value;
        GSGFListOf *list_of = NULL;
        gsize j, k, num_items;
        GSGFPoint *point;
        gint nv;
        GSGFListOf *this_list;
        gint *these_points;
        gsize num_these_points;

        gsgf_return_val_if_fail (GSGF_IS_LIST_OF (value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), FALSE, error);

        this_list = GSGF_LIST_OF (value);
        num_these_points = gsgf_list_of_get_number_of_items (this_list);
        these_points = g_alloca (num_these_points * sizeof *these_points);
        for (k = 0; k < num_these_points; ++k) {
                these_points[k] = gsgf_point_get_normalized_value (
                                        GSGF_POINT (gsgf_list_of_get_nth_item
                                                        (this_list, k)));
        }
        id = gsgf_property_get_id (property);
        node = gsgf_property_get_node (property);

        for (i = 0; i < 5; ++i) {
                list_of = NULL;
                cooked_value = NULL;
                other_prop = NULL;
                if (strcmp (id, ids[i]))
                        other_prop = gsgf_node_get_property (node, ids[i]);
                if (other_prop)
                        cooked_value = gsgf_property_get_value (other_prop);
                if (cooked_value && GSGF_IS_LIST_OF (cooked_value)) {
                        list_of = GSGF_LIST_OF (cooked_value);
                        num_items = gsgf_list_of_get_number_of_items (list_of);
                        for (j = 0; j < num_items; ++j) {
                                point = GSGF_POINT (gsgf_list_of_get_nth_item
                                                    (list_of, j));
                                nv = gsgf_point_get_normalized_value (point);
                                for (k = 0; k < num_these_points; ++k) {
                                        if (these_points[k] == nv) {
                                                g_set_error (error, GSGF_ERROR,
                                                      GSGF_ERROR_SEMANTIC_ERROR,
                                                      _("The properties '%s'"
                                                        " and '%s' are not"
                                                        " allowed on the same"
                                                        " point within one"
                                                        " node"),
                                                        id, ids[i]);
                                                return FALSE;
                                        }
                                }
                        }
                }
        }

        return TRUE;
}

/* This function is too special to export it to individual flavors.  */
static gboolean
gsgf_constraint_move_annotation_with_move (const GSGFCookedValue *value,
                                           const GSGFRaw *raw,
                                           const GSGFProperty *property,
                                           GError **error)
{
        GSGFNode *node;

        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE (value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), FALSE, error);

        node = gsgf_property_get_node(property);

        if (!gsgf_node_get_property (node, "B")
            && !gsgf_node_get_property (node, "W"))
                _gsgf_node_mark_loser_property (node,
                                                gsgf_property_get_id (property));

        return TRUE;
}

gboolean
gsgf_constraint_is_single_value(const GSGFCookedValue *value,
                                const GSGFRaw *raw,
                                const GSGFProperty *property, GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE (value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), FALSE, error);

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

static GSGFCookedValue *
gsgf_FG_new_from_raw (const GSGFRaw* raw, const GSGFFlavor *flavor,
                      const GSGFProperty *property, GError **error)
{
        gchar *raw_string = gsgf_raw_get_value (raw, 0);
        GSGFCompose *retval;
        gint64 value;
        gchar *name;

        if (!*raw_string)
                return GSGF_COOKED_VALUE (gsgf_empty_new ());

        errno = 0;
        value = g_ascii_strtoll (raw_string, &name, 10);
        if (errno) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                             _("Invalid number '%s': %s"),
                             raw_string, strerror (errno));
                return FALSE;
        }

        if (value < 0) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Negativ number not allowed: %lld"),
                             (long long) value);
                return FALSE;
        }

        if (!name || !*name ||  ':' != *name++) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                             _("Expected colon after number in '%s'"),
                             raw_string);
                return FALSE;
        }

        retval = gsgf_compose_new(GSGF_COOKED_VALUE (gsgf_number_new (value)),
                                  GSGF_COOKED_VALUE (gsgf_simple_text_new (name)),
                                  NULL);

        return GSGF_COOKED_VALUE(retval);
}

static gboolean
gsgf_constraint_is_ST_number(const GSGFCookedValue *value,
                             const GSGFRaw *raw,
                             const GSGFProperty *property, GError **error)
{
        GSGFNumber *number;

        gsgf_return_val_if_fail (GSGF_IS_COOKED_VALUE (value), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), FALSE, error);

        number = GSGF_NUMBER(value);

        if (gsgf_number_get_value(number) < 1) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Value must be greater than 0 but is %lld"),
                            (long long int) gsgf_number_get_value(number));
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

        columns_as_string = gsgf_util_read_simple_text (raw_string,
                                                        &rows_as_string, ':');
        if (!columns_as_string || !*columns_as_string) {
                if (columns_as_string) g_free(columns_as_string);
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                            _("Empty property"));
                return NULL;
        }

        dummy = gsgf_raw_new(columns_as_string);
        cooked = gsgf_number_new_from_raw(dummy, flavor, property, error);
        g_free(columns_as_string);
        g_object_unref(dummy);

        if (!cooked)
                return NULL;
        columns = GSGF_NUMBER(cooked);

        if (rows_as_string && rows_as_string[0]
            && rows_as_string[1] && rows_as_string != columns_as_string) {
                dummy = gsgf_raw_new(rows_as_string + 1);
                cooked = gsgf_number_new_from_raw(dummy, flavor, property, error);
                g_object_unref(dummy);

                if (!cooked) {
                        g_object_unref(columns);
                        return NULL;
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
        const gchar *id;

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
gsgf_list_of_stones_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                                 const GSGFProperty *property, GError **error)
{
        GType type = GSGF_FLAVOR_GET_CLASS(flavor)->stone_type;
        GSGFListOf *list_of = gsgf_list_of_new(type, flavor);
        GSGFStone *stone;
        gsize i, num_stones;

        num_stones = gsgf_raw_get_number_of_values(raw);
        if (!num_stones) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_EMPTY,
                                _("List of stones must not be empty"));
                g_object_unref(list_of);
                return NULL;
        }
        for (i = 0; i < gsgf_raw_get_number_of_values(raw); ++i) {
                stone = gsgf_flavor_create_stone(flavor, raw, i, error);
                if (!stone) {
                        g_object_unref(list_of);
                        return NULL;
                }
                if (!gsgf_list_of_append(list_of, GSGF_COOKED_VALUE(stone), error)) {
                        g_object_unref(list_of);
                        return NULL;
                }
        }

        return GSGF_COOKED_VALUE(list_of);
}

static GSGFCookedValue *
gsgf_list_of_points_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                                 const GSGFProperty *property, GError **error)
{
        GType type = GSGF_FLAVOR_GET_CLASS(flavor)->point_type;
        GSGFListOf *list_of = gsgf_list_of_new(type, flavor);
        gsize i, num_points;

        num_points = gsgf_raw_get_number_of_values(raw);
        if (!num_points) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_EMPTY,
                                _("List of points must not be empty"));
                g_object_unref(list_of);
                return NULL;
        }
        for (i = 0; i < gsgf_raw_get_number_of_values(raw); ++i) {
                if (!gsgf_flavor_append_points(flavor, list_of, raw, i, error)) {
                        g_object_unref(list_of);
                        return NULL;
                }
        }

        if (!gsgf_list_of_points_check_unique(list_of, error)) {
                g_object_unref(list_of);
                return NULL;
        }

        return GSGF_COOKED_VALUE(list_of);
}

static GSGFCookedValue *
gsgf_elist_of_points_new_from_raw(const GSGFRaw* raw, const GSGFFlavor *flavor,
                                  const GSGFProperty *property, GError **error)
{
        GSGFListOf *list_of;
        gsize num_values = gsgf_raw_get_number_of_values (raw);
        gsize i;

        num_values = gsgf_raw_get_number_of_values(raw);
        for (i = 0; i < num_values; ++i) {
                if (*(gsgf_raw_get_value (raw, i)))
                        return gsgf_list_of_points_new_from_raw (raw, flavor,
                                                                 property,
                                                                 error);
        }

        list_of = gsgf_list_of_new (gsgf_empty_get_type (), flavor);
        gsgf_list_of_append (list_of, GSGF_COOKED_VALUE (gsgf_empty_new ()),
                             NULL);

        return GSGF_COOKED_VALUE (list_of);
}

static GSGFCookedValue *
gsgf_list_of_lines_new_from_raw (const GSGFRaw* raw,
                                  const GSGFFlavor *flavor,
                                  const GSGFProperty *property,
                                  GError **error)
{
        GType type = GSGF_TYPE_COMPOSE;
        GSGFListOf *list_of = gsgf_list_of_new (type, flavor);
        GSGFCompose *compose;
        gsize i, num_pairs;
        gchar *raw_string;
        gchar *start_string;
        const gchar *end_string;
        GSGFPoint *start_point;
        GSGFPoint *end_point;
        gint start_normalized, end_normalized;
        GSGFRaw *tmp_raw;
        GList *prev_points = NULL;
        GList *iter;
        gint start_prev, end_prev;

        num_pairs = gsgf_raw_get_number_of_values (raw);
        if (!num_pairs) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_EMPTY,
                                _("List of point pairs must not be empty"));
                g_object_unref(list_of);
                return NULL;
        }

        for (i = 0; i < gsgf_raw_get_number_of_values (raw); ++i) {
                raw_string = gsgf_raw_get_value (raw, i);
                start_string = gsgf_util_read_simple_text (raw_string,
                                                           &end_string, ':');
                if (!start_string || !*start_string) {
                        if (start_string) g_free(start_string);
                        g_object_unref (list_of);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("Empty property"));
                        return FALSE;
                }

                if (!end_string || !end_string[0] || !end_string[1]
                    || end_string == start_string) {
                        g_free (start_string);
                        g_object_unref (list_of);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("No end point"));
                        return FALSE;
                }
                ++end_string;

                /* We cannot create a point from a string.  We therefore need
                 * a temporary GSGFRaw.
                 */
                tmp_raw = gsgf_raw_new (start_string);
                g_free (start_string);
                start_point = gsgf_flavor_create_point (flavor, tmp_raw,
                                                        0, error);
                g_object_unref (tmp_raw);
                if (!start_point) {
                        g_object_unref (list_of);
                        return FALSE;
                }

                tmp_raw = gsgf_raw_new (end_string);
                end_point = gsgf_flavor_create_point (flavor, tmp_raw,
                                                      0, error);
                g_object_unref (tmp_raw);
                if (!end_point) {
                        g_object_unref (start_point);
                        g_object_unref (list_of);
                        return FALSE;
                }

                start_normalized = gsgf_point_get_normalized_value (start_point);
                end_normalized = gsgf_point_get_normalized_value (end_point);

                if (start_normalized == end_normalized) {
                        g_object_unref (start_point);
                        g_object_unref (end_point);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("Start and end point must differ"));
                        return FALSE;
                }

                iter = prev_points;
                while (iter) {
                		start_prev = GPOINTER_TO_INT (iter->data);
                		iter = iter->next;
                		end_prev = GPOINTER_TO_INT (iter->data);
                		iter = iter->next;
                		if (start_normalized == start_prev
                		    && end_normalized == end_prev) {
                				g_object_unref (start_point);
                				g_object_unref (end_point);
                				g_set_error(error, GSGF_ERROR,
                					    GSGF_ERROR_SEMANTIC_ERROR,
                					    _("Lines and arrows"
                					      " must be unique"));
                				return FALSE;
                		}
                }

                prev_points = g_list_append (prev_points,
                                             GINT_TO_POINTER (start_normalized));
                prev_points = g_list_append (prev_points,
                                             GINT_TO_POINTER (end_normalized));

                compose = gsgf_compose_new (GSGF_COOKED_VALUE (start_point),
                                            GSGF_COOKED_VALUE (end_point),
                                            NULL);
                if (!compose) {
                        g_object_unref (start_point);
                        g_object_unref (end_point);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                                    _("Invalid points in property"));
                        return FALSE;
                }
                if (!gsgf_list_of_append (list_of, GSGF_COOKED_VALUE (compose),
                                          error)) {
                        g_object_unref (compose);
                        return FALSE;
                }
        }

        g_list_free (prev_points);

        return GSGF_COOKED_VALUE (list_of);
}

static GSGFCookedValue *
gsgf_list_of_point_labels_new_from_raw (const GSGFRaw* raw,
                                        const GSGFFlavor *flavor,
                                        const GSGFProperty *property,
                                        GError **error)
{
        GType type = GSGF_TYPE_COMPOSE;
        GSGFListOf *list_of = gsgf_list_of_new (type, flavor);
        GSGFCompose *compose;
        gsize i, num_pairs;
        gchar *raw_string;
        gchar *start_string;
        const gchar *end_string;
        GSGFPoint *start_point;
        GSGFSimpleText *end_label;
        gint start_normalized;
        GSGFRaw *tmp_raw;
        GList *prev_points = NULL;
        GList *iter;
        gint start_prev;

        num_pairs = gsgf_raw_get_number_of_values (raw);
        if (!num_pairs) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_EMPTY,
                                _("List of point labels must not be empty"));
                g_object_unref(list_of);
                return NULL;
        }

        for (i = 0; i < gsgf_raw_get_number_of_values (raw); ++i) {
                raw_string = gsgf_raw_get_value (raw, i);
                start_string = gsgf_util_read_simple_text (raw_string,
                                                           &end_string, ':');
                if (!start_string || !*start_string) {
                        if (start_string) g_free(start_string);
                        g_object_unref (list_of);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("Empty property"));
                        return FALSE;
                }

                if (!end_string || !end_string[0] || !end_string[1]
                    || end_string == start_string) {
                        g_free (start_string);
                        g_object_unref (list_of);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                                    _("No end point"));
                        return FALSE;
                }
                ++end_string;

                /* We cannot create a point from a string.  We therefore need
                 * a temporary GSGFRaw.
                 */
                tmp_raw = gsgf_raw_new (start_string);
                g_free (start_string);
                start_point = gsgf_flavor_create_point (flavor, tmp_raw,
                                                        0, error);
                g_object_unref (tmp_raw);
                if (!start_point) {
                        g_object_unref (list_of);
                        return FALSE;
                }

                tmp_raw = gsgf_raw_new (end_string);
                end_label = GSGF_SIMPLE_TEXT (
                                gsgf_simple_text_new_from_raw (tmp_raw, flavor,
                                                               property,
                                                               error));
                g_object_unref (tmp_raw);
                if (!end_label) {
                        g_object_unref (start_point);
                        g_object_unref (list_of);
                        return FALSE;
                }

                start_normalized = gsgf_point_get_normalized_value (start_point);

                iter = prev_points;
                while (iter) {
                                start_prev = GPOINTER_TO_INT (iter->data);
                                iter = iter->next;
                                if (start_normalized == start_prev) {
                                                g_object_unref (start_point);
                                                g_object_unref (end_label);
                                                g_set_error(error, GSGF_ERROR,
                                                            GSGF_ERROR_SEMANTIC_ERROR,
                                                            _("Points must be unique"));
                                                return FALSE;
                                }
                }

                prev_points = g_list_append (prev_points,
                                             GINT_TO_POINTER (start_normalized));

                compose = gsgf_compose_new (GSGF_COOKED_VALUE (start_point),
                                            GSGF_COOKED_VALUE (end_label),
                                            NULL);
                if (!compose) {
                        g_object_unref (start_point);
                        g_object_unref (end_label);
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_INTERNAL_ERROR,
                                    _("Invalid point labels in property"));
                        return FALSE;
                }
                if (!gsgf_list_of_append (list_of, GSGF_COOKED_VALUE (compose),
                                          error)) {
                        g_object_unref (compose);
                        return FALSE;
                }
        }

        g_list_free (prev_points);

        return GSGF_COOKED_VALUE (list_of);
}

static gboolean
gsgf_list_of_points_check_unique(const GSGFListOf *list_of,
                                 GError **error)
{
        gsize num_items = gsgf_list_of_get_number_of_items(list_of);
        gint *normalized = g_alloca(num_items * sizeof (gint));
        gsize i;
        GSGFCookedValue *point;

        for (i = 0; i < num_items; ++i) {
                point = gsgf_list_of_get_nth_item(list_of, i);
                normalized[i] = gsgf_point_get_normalized_value(GSGF_POINT(point));
        }

        qsort(normalized, num_items, sizeof (gint), compare_gint);

        for (i = 1; i < num_items; ++i) {
                if (normalized[i] == normalized[i - 1]) {
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_NON_UNIQUE_POINT,
                                    _("Points in list must be unique"));
                        return FALSE;
                }
        }

        return TRUE;
}

static int
compare_gint(const void *a, const void *b)
{
        return *(gint *)a - *(gint *)b;
}

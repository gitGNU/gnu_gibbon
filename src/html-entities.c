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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>

#include <string.h>
#include "html-entities.h"

struct mapping {
        gunichar c;
        gchar *ent;
};

/* We prefer named entities over decimal entities because they are less
 * obstrusive to users of other clients that are still on 7 bit.  "That costs
 * you one &euro;" is better understandable than "That costs you on &#x80ac;".
 *
 * On the other hand, scripts that use only characters outside of US-ASCII
 * are completely expressed in numerical entities.  Users of 7 bit clients
 * would not understand anything at all, either way.
 */
struct mapping mappings[] = {
                { 0x22, "quot" },
                { 0x26, "amp" },
                { 0x27, "apos" },
                { 0x3c, "lt" },
                { 0x3e, "gt" },
                { 0xa0, "nbsp" },
                { 0xa1, "iexcl" },
                { 0xa2, "cent" },
                { 0xa3, "pound" },
                { 0xa4, "curren" },
                { 0xa5, "yen" },
                { 0xa6, "brvbar" },
                { 0xa7, "sect" },
                { 0xa8, "uml" },
                { 0xa9, "copy" },
                { 0xaa, "ordf" },
                { 0xab, "laquo" },
                { 0xac, "not" },
                { 0xad, "shy" },
                { 0xae, "reg" },
                { 0xaf, "macr" },
                { 0xb0, "deg" },
                { 0xb1, "plusmn" },
                { 0xb2, "sup2" },
                { 0xb3, "sup3" },
                { 0xb4, "acute" },
                { 0xb5, "micro" },
                { 0xb6, "para" },
                { 0xb7, "middot" },
                { 0xb8, "cedil" },
                { 0xb9, "sup1" },
                { 0xba, "ordm" },
                { 0xbb, "raquo" },
                { 0xbc, "frac14" },
                { 0xbd, "frac12" },
                { 0xbe, "frac34" },
                { 0xbf, "iquest" },
                { 0xc0, "Agrave" },
                { 0xc1, "Aacute" },
                { 0xc2, "Acirc" },
                { 0xc3, "Atilde" },
                { 0xc4, "Auml" },
                { 0xc5, "Aring" },
                { 0xc6, "AElig" },
                { 0xc7, "Ccedil" },
                { 0xc8, "Egrave" },
                { 0xc9, "Eacute" },
                { 0xca, "Ecirc" },
                { 0xcb, "Euml" },
                { 0xcc, "Igrave" },
                { 0xcd, "Iacute" },
                { 0xce, "Icirc" },
                { 0xcf, "Iuml" },
                { 0xd0, "ETH" },
                { 0xd1, "Ntilde" },
                { 0xd2, "Ograve" },
                { 0xd3, "Oacute" },
                { 0xd4, "Ocirc" },
                { 0xd5, "Otilde" },
                { 0xd6, "Ouml" },
                { 0xd7, "times" },
                { 0xd8, "Oslash" },
                { 0xd9, "Ugrave" },
                { 0xda, "Uacute" },
                { 0xdb, "Ucirc" },
                { 0xdc, "Uuml" },
                { 0xdd, "Yacute" },
                { 0xde, "THORN" },
                { 0xdf, "szlig" },
                { 0xe0, "agrave" },
                { 0xe1, "aacute" },
                { 0xe2, "acirc" },
                { 0xe3, "atilde" },
                { 0xe4, "auml" },
                { 0xe5, "aring" },
                { 0xe6, "aelig" },
                { 0xe7, "ccedil" },
                { 0xe8, "egrave" },
                { 0xe9, "eacute" },
                { 0xea, "ecirc" },
                { 0xeb, "euml" },
                { 0xec, "igrave" },
                { 0xed, "iacute" },
                { 0xee, "icirc" },
                { 0xef, "iuml" },
                { 0xf0, "eth" },
                { 0xf1, "ntilde" },
                { 0xf2, "ograve" },
                { 0xf3, "oacute" },
                { 0xf4, "ocirc" },
                { 0xf5, "otilde" },
                { 0xf6, "ouml" },
                { 0xf7, "divide" },
                { 0xf8, "oslash" },
                { 0xf9, "ugrave" },
                { 0xfa, "uacute" },
                { 0xfb, "ucirc" },
                { 0xfc, "uuml" },
                { 0xfd, "yacute" },
                { 0xfe, "thorn" },
                { 0xff, "yuml" },
                { 0x152, "OElig" },
                { 0x153, "oelig" },
                { 0x160, "Scaron" },
                { 0x161, "scaron" },
                { 0x178, "Yuml" },
                { 0x192, "fnof" },
                { 0x2c6, "circ" },
                { 0x2dc, "tilde" },
                { 0x391, "Alpha" },
                { 0x392, "Beta" },
                { 0x393, "Gamma" },
                { 0x394, "Delta" },
                { 0x395, "Epsilon" },
                { 0x396, "Zeta" },
                { 0x397, "Eta" },
                { 0x398, "Theta" },
                { 0x399, "Iota" },
                { 0x39a, "Kappa" },
                { 0x39b, "Lambda" },
                { 0x39c, "Mu" },
                { 0x39d, "Nu" },
                { 0x39e, "Xi" },
                { 0x39f, "Omicron" },
                { 0x3a0, "Pi" },
                { 0x3a1, "Rho" },
                { 0x3a3, "Sigma" },
                { 0x3a4, "Tau" },
                { 0x3a5, "Upsilon" },
                { 0x3a6, "Phi" },
                { 0x3a7, "Chi" },
                { 0x3a8, "Psi" },
                { 0x3a9, "Omega" },
                { 0x3b1, "alpha" },
                { 0x3b2, "beta" },
                { 0x3b3, "gamma" },
                { 0x3b4, "delta" },
                { 0x3b5, "epsilon" },
                { 0x3b6, "zeta" },
                { 0x3b7, "eta" },
                { 0x3b8, "theta" },
                { 0x3b9, "iota" },
                { 0x3ba, "kappa" },
                { 0x3bb, "lambda" },
                { 0x3bc, "mu" },
                { 0x3bd, "nu" },
                { 0x3be, "xi" },
                { 0x3bf, "omicron" },
                { 0x3c0, "pi" },
                { 0x3c1, "rho" },
                { 0x3c2, "sigmaf" },
                { 0x3c3, "sigma" },
                { 0x3c4, "tau" },
                { 0x3c5, "upsilon" },
                { 0x3c6, "phi" },
                { 0x3c7, "chi" },
                { 0x3c8, "psi" },
                { 0x3c9, "omega" },
                { 0x3d1, "thetasym" },
                { 0x3d2, "upsih" },
                { 0x3d6, "piv" },
                { 0x2002, "ensp" },
                { 0x2003, "emsp" },
                { 0x2009, "thinsp" },
                { 0x200c, "zwnj" },
                { 0x200d, "zwj" },
                { 0x200e, "lrm" },
                { 0x200f, "rlm" },
                { 0x2013, "ndash" },
                { 0x2014, "mdash" },
                { 0x2018, "lsquo" },
                { 0x2019, "rsquo" },
                { 0x201a, "sbquo" },
                { 0x201c, "ldquo" },
                { 0x201d, "rdquo" },
                { 0x201e, "bdquo" },
                { 0x2020, "dagger" },
                { 0x2021, "Dagger" },
                { 0x2022, "bull" },
                { 0x2026, "hellip" },
                { 0x2030, "permil" },
                { 0x2032, "prime" },
                { 0x2033, "Prime" },
                { 0x2039, "lsaquo" },
                { 0x203a, "rsaquo" },
                { 0x203e, "oline" },
                { 0x2044, "frasl" },
                { 0x20ac, "euro" },
                { 0x2111, "image" },
                { 0x2118, "weierp" },
                { 0x211c, "real" },
                { 0x2122, "trade" },
                { 0x2135, "alefsym" },
                { 0x2190, "larr" },
                { 0x2191, "uarr" },
                { 0x2192, "rarr" },
                { 0x2193, "darr" },
                { 0x2194, "harr" },
                { 0x21b5, "crarr" },
                { 0x21d0, "lArr" },
                { 0x21d1, "uArr" },
                { 0x21d2, "rArr" },
                { 0x21d3, "dArr" },
                { 0x21d4, "hArr" },
                { 0x2200, "forall" },
                { 0x2202, "part" },
                { 0x2203, "exist" },
                { 0x2205, "empty" },
                { 0x2207, "nabla" },
                { 0x2208, "isin" },
                { 0x2209, "notin" },
                { 0x220b, "ni" },
                { 0x220f, "prod" },
                { 0x2211, "sum" },
                { 0x2212, "minus" },
                { 0x2217, "lowast" },
                { 0x221a, "radic" },
                { 0x221d, "prop" },
                { 0x221e, "infin" },
                { 0x2220, "ang" },
                { 0x2227, "and" },
                { 0x2228, "or" },
                { 0x2229, "cap" },
                { 0x222a, "cup" },
                { 0x222b, "int" },
                { 0x2234, "there4" },
                { 0x223c, "sim" },
                { 0x2245, "cong" },
                { 0x2248, "asymp" },
                { 0x2260, "ne" },
                { 0x2261, "equiv" },
                { 0x2264, "le" },
                { 0x2265, "ge" },
                { 0x2282, "sub" },
                { 0x2283, "sup" },
                { 0x2284, "nsub" },
                { 0x2286, "sube" },
                { 0x2287, "supe" },
                { 0x2295, "oplus" },
                { 0x2297, "otimes" },
                { 0x22a5, "perp" },
                { 0x22c5, "sdot" },
                { 0x2308, "lceil" },
                { 0x2309, "rceil" },
                { 0x230a, "lfloor" },
                { 0x230b, "rfloor" },
                { 0x2329, "lang" },
                { 0x232a, "rang" },
                { 0x25ca, "loz" },
                { 0x2660, "spades" },
                { 0x2663, "clubs" },
                { 0x2665, "hearts" },
                { 0x2666, "diams" }
};

static void init_tables (void);
static gunichar read_entity (const gchar *string, gsize *length);
static GHashTable *unichar2name = NULL;
static GHashTable *name2unichar = NULL;

gchar *
encode_html_entities (const gchar *original)
{
        GString *string = g_string_sized_new (strlen (original));
        const gchar *ptr = original;
        gunichar next_char;
        gchar *retval;
        gchar *ent;

        if (!unichar2name)
                init_tables ();

        while (*ptr) {
                if ('&' == *ptr) {
                        if (read_entity (ptr, NULL)) {
                                string = g_string_append (string, "&amp;");
                                ++ptr;
                                continue;
                        }
                }

                next_char = g_utf8_get_char_validated (ptr, -1);
                if (next_char < 0x80) {
                        string = g_string_append_unichar (string, next_char);
                        ++ptr;
                } else {
                        ent = (gchar *) g_hash_table_lookup (unichar2name,
                                                             &next_char);
                        if (ent)
                                g_string_append_printf (string, "&%s;", ent);
                        else
                                g_string_append_printf (string, "&#x%x;",
                                                        next_char);
                        /* A NULL parameter for the output buffer causes
                         * the function to just compute the length in bytes.
                         */
                        ptr += g_unichar_to_utf8 (next_char, NULL);
                }
        }

        retval = string->str;
        g_string_free (string, FALSE);

        return retval;
}

gchar *
decode_html_entities (const gchar *original)
{
        GString *string = g_string_new ("");
        const gchar *ptr = original;
        gchar *retval;
        gunichar decoded;
        gsize length;

        if (!unichar2name)
                init_tables ();

        while (*ptr) {
                decoded = read_entity (ptr, &length);
                if (decoded) {
                        ptr += length;
                        string = g_string_append_unichar (string, decoded);
                } else {
                        string = g_string_append_c (string, *ptr);
                        ++ptr;
                }
        }

        retval = string->str;
        g_string_free (string, FALSE);

        return retval;
}

static void
init_tables (void)
{
        gsize num_items = (sizeof mappings) / (sizeof *mappings);
        gsize i;
        struct mapping *mapping;

        unichar2name = g_hash_table_new (g_int_hash, g_int_equal);
        name2unichar = g_hash_table_new (g_str_hash, g_str_equal);

        for (i = 0; i < num_items; ++i) {
                mapping = mappings + i;
                g_hash_table_insert (unichar2name, &mapping->c, mapping->ent);
                g_hash_table_insert (name2unichar, mapping->ent, &mapping->c);
        }
}

static gunichar
read_entity (const gchar *string, gsize *length)
{
        gchar *endptr;
        gunichar retval;
        const gchar *ptr;
        gchar *ent;
        gpointer hash_value;
        gunichar *retptr;

        if ('&' != string[0])
                return 0;

        if ('#' == string[1]) {
                if ('x' == string[2]) {
                        retval = g_ascii_strtoull (string + 3, &endptr, 16);
                } else {
                        retval = g_ascii_strtoull (string + 2, &endptr, 10);
                }
                if (retval && ';' == *endptr && g_unichar_validate (retval)) {
                        if (length)
                                *length = 1 + endptr - string;
                        return retval;
                }
        } else {
                ptr = string + 1;
                while (1) {
                        if (';' == *ptr) {
                                ent = g_strndup (string + 1, ptr - string - 1);
                                hash_value = g_hash_table_lookup (name2unichar,
                                                                 (gpointer) ent);
                                retptr = (gunichar *) hash_value;
                                if (retptr && g_unichar_validate (*retptr)) {
                                        if (length)
                                                *length = 2 + strlen (ent);
                                        g_free (ent);
                                        return *retptr;
                                }
                                g_free (ent);
                                return 0;
                        }

                        if (!*ptr)
                                return 0;
                        if (*ptr < 'A')
                                return 0;
                        if (*ptr > 'z')
                                return 0;
                        if (*ptr > 'Z' && *ptr < 'a')
                                return 0;

                        ++ptr;
                }
        }

        return 0;
}

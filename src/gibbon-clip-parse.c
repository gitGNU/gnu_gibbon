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

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip-parse.h"
#include "gibbon-util.h"

static gboolean gibbon_clip_parse_clip (const gchar *line,
                                        gchar **tokens,
                                        GSList **result);
static gboolean gibbon_clip_parse_clip_welcome (const gchar *line,
                                                gchar **tokens,
                                                GSList **result);

static GSList *gibbon_clip_parse_alloc_int (GSList *list,
                                            enum GibbonClipType type,
                                            gint64 value);
static GSList *gibbon_clip_parse_alloc_string (GSList *list,
                                               enum GibbonClipType type,
                                               const gchar *value);

GSList *
gibbon_clip_parse (const gchar *line)
{
        gboolean success = FALSE;
        GSList *result = NULL;
        gchar **tokens = gibbon_strsplit_ws (line);
        gchar *first = tokens[0];

        if (first[0] >= '1' && first[0] <= '9' && !first[1])
                success = gibbon_clip_parse_clip (line, tokens, &result);
        else if (first[0] == '1' && first[1] >= '1' && first[1] <= '9'
                 && !first[2])
                success = gibbon_clip_parse_clip (line, tokens, &result);

        g_strfreev (tokens);
        if (!success) {
                gibbon_clip_free_result (result);
                return NULL;
        }

        return g_slist_reverse (result);
}

static gboolean
gibbon_clip_parse_clip (const gchar *line, gchar **tokens,
                        GSList **result)
{
        enum GibbonClipCode code;
        gchar *first = tokens[0];

        code = first[0] - '0';
        if (first[1]) {
                code *= 10;
                code += first[1] - '0';
        }

        *result = gibbon_clip_parse_alloc_int (*result, GIBBON_CLIP_TYPE_UINT,
                                               (gint64) code);

        switch (code) {
                case GIBBON_CLIP_CODE_WELCOME:
                        return gibbon_clip_parse_clip_welcome (line, tokens,
                                                               result);
                default:
                        break;
        }

        return FALSE;
}

static gboolean
gibbon_clip_parse_clip_welcome (const gchar *line, gchar **tokens,
                                GSList **result)
{
        if (4 != g_strv_length (tokens))
                return FALSE;

        *result = gibbon_clip_parse_alloc_string (*result,
                                                  GIBBON_CLIP_TYPE_NAME,
                                                  tokens[1]);

        return TRUE;
}

static GSList *
gibbon_clip_parse_alloc_int (GSList *list,
                             enum GibbonClipType type,
                             gint64 value)
{
        struct GibbonClipTokenSet *token = g_malloc (sizeof *token);

        token->type = type;
        token->v.i64 = value;

        return g_slist_prepend (list, token);
}

static GSList *
gibbon_clip_parse_alloc_string (GSList *list,
                                enum GibbonClipType type,
                                const gchar *value)
{
        struct GibbonClipTokenSet *token = g_malloc (sizeof *token);

        token->type = type;
        token->v.s = g_strdup (value);

        return g_slist_prepend (list, token);
}

void
gibbon_clip_free_result (GSList *result)
{
        GSList *iter;
        struct GibbonClipTokenSet *set;

        if (G_UNLIKELY (!result))
                return;

        iter = result;
        while (iter) {
                set = (struct GibbonClipTokenSet *) iter->data;
                switch (set->type) {
                        case GIBBON_CLIP_TYPE_NAME:
                        case GIBBON_CLIP_TYPE_STRING:
                                g_free (set->v.s);
                                break;
                        default:
                                break;
                }
                iter = iter->next;
        }

        g_slist_free (result);
}

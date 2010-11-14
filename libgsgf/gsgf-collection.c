/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-collection
 * @short_description: A collection of games.
 *
 * A #GSGFCollection represents a collection of games, for example a match.
 * It is the root in the SGF hierarchy.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <errno.h>

#include <libgsgf/gsgf.h>

#include "gsgf-internal.h"

enum gsgf_parser_state {
        GSGF_PARSER_STATE_INIT,
        GSGF_PARSER_STATE_NODE,
        GSGF_PARSER_STATE_PROPERTY,
        GSGF_PARSER_STATE_PROP_VALUE,
        GSGF_PARSER_STATE_VALUE,
        GSGF_PARSER_STATE_PROP_CLOSE,
        GSGF_PARSER_STATE_PROPERTIES,
        GSGF_PARSER_STATE_PROP_VALUE_READ,
        GSGF_PARSER_STATE_GAME_TREES,
};

typedef struct {
        GInputStream *stream;
        GCancellable *cancellable;
        guint lineno;
        guint colno;
        guint start_lineno;
        guint start_colno;
        gchar buffer[8192];
        gsize bufsize;
        gsize bufpos;
        GError **error;
        GSGFError estatus;
        enum gsgf_parser_state state;
} GSGFParserContext;

struct _GSGFCollectionPrivate {
        GList* game_trees;
};

#define GSGF_COLLECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COLLECTION,           \
                                      GSGFCollectionPrivate))
G_DEFINE_TYPE (GSGFCollection, gsgf_collection, G_TYPE_OBJECT)
;

#define GSGF_TOKEN_EOF 256
#define GSGF_TOKEN_PROP_IDENT 257
#define GSGF_TOKEN_VALUE_TEXT 258
#define GSGF_TOKEN_VALUE_NUMBER 259
#define GSGF_TOKEN_VALUE_REAL 260

static gint gsgf_yylex(GSGFParserContext *ctx, GString **value);
static gint gsgf_yylex_c_value_type(GSGFParserContext *ctx, GString **value);
static gssize gsgf_yyread(GSGFParserContext *ctx);
static gint gsgf_yyread_prop_ident(GSGFParserContext *ctx, gchar c,
                                   GString **value);
static void gsgf_yyread_linebreak(GSGFParserContext *ctx, gchar c);
static void gsgf_yyerror(GSGFParserContext *ctx, const gchar *expect,
                         gint token, GError **error);

static GRegex *double_pattern = NULL;

/**
 * The SGF specification stipulates that a collection has one ore more game trees,
 * and that a game tree has one or more nodes.  For practical purposes we allow
 * empty collections and empty node lists (sequences).  When implementing the
 * serialization code we have to accomodate that.  Either we throw an error
 * or we spit out the smallest collections conforming to the SGF specification
 * which would be the string "(;)".
 */
static void gsgf_collection_init(GSGFCollection *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_COLLECTION,
                        GSGFCollectionPrivate);

        self->priv->game_trees = NULL;
}

static void gsgf_collection_finalize(GObject *object)
{
        GSGFCollection *self = GSGF_COLLECTION (object);

        if (self->priv->game_trees) {
                g_list_foreach(self->priv->game_trees, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->game_trees);
        }
        self->priv->game_trees = NULL;

        G_OBJECT_CLASS (gsgf_collection_parent_class)->finalize(object);
}

static void gsgf_collection_class_init(GSGFCollectionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFCollectionPrivate));

        double_pattern = g_regex_new("^[+-]?[0-9]+(?:\\.[0-9]+)?$", 0, 0, NULL);

        object_class->finalize = gsgf_collection_finalize;
}

/**
 * gsgf_collection_new:
 *
 * Build an empty #GSGFCollection in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFCollection.
 */
GSGFCollection *
gsgf_collection_new()
{
        GSGFCollection *self = g_object_new(GSGF_TYPE_COLLECTION, NULL);

        /* self->priv->move_number = 0; */

        return self;
}

/**
 * gsgf_collection_parse_stream:
 * @stream: a #GInputStream to parse.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Parses an input stream into a #GSGFCollection in memory.  A return value of
 * non-%NULL does not necessarily mean success.  A parse error normally results
 * in a valid #GSGFCollection object that holds only parts of the information
 * from the stream.  Use @error for error checking instead.
 *
 * See also gsgf_collection_parse_file ().
 *
 * Returns: A #GSGFCollection or %NULL on error.
 */
GSGFCollection *
gsgf_collection_parse_stream(GInputStream *stream, GCancellable *cancellable,
                             GError **error)
{
        GSGFCollection *self = gsgf_collection_new();
        gint token = 0;
        GString *value;
        GSGFParserContext ctx;

        GSGFGameTree *game_tree = NULL;
        GSGFNode *node = NULL;

        ctx.stream = stream;
        ctx.cancellable = cancellable;
        ctx.error = error;
        ctx.estatus = GSGF_ERROR_NONE;
        ctx.lineno = ctx.start_lineno = 1;
        ctx.colno = ctx.start_colno = 0;
        ctx.bufsize = 0;
        ctx.bufpos = 0;
        ctx.state = GSGF_PARSER_STATE_INIT;

        do {
                if (token == '[')
                        token = gsgf_yylex_c_value_type(&ctx, &value);
                else
                        token = gsgf_yylex(&ctx, &value);


#if (0)
                if (value) {
                        g_print("%d:%d: Token: %d \"%s\"\n",
                                ctx.start_lineno, ctx.start_colno + 1, token, value->str);
                } else if (token < 256 && token >= 32)
                        g_print("%d:%d: Token: %c NONE\n",
                                ctx.start_lineno, ctx.start_colno + 1, (char) token);
                else
                        g_print("%d:%d: Token: %d NONE\n",
                                ctx.start_lineno, ctx.start_colno + 1, token);
#endif

                if (token == -1) {
                        if (value)
                                g_string_free(value, TRUE);
                        break;
                }

                /* FIXME! We need a test case that checks that ((;);) is illegal.
                 * A NodeList cannot follow a (sub-)GameTree.
                 */

                switch (ctx.state) {
                        case GSGF_PARSER_STATE_INIT:
                                if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_collection_add_game_tree(self);
                                } else {
                                        gsgf_yyerror(&ctx, _("'('"), token, error);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_NODE:
                                if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                        node = gsgf_game_tree_add_node(game_tree);
                                } else {
                                        gsgf_yyerror(&ctx, _("';'"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROPERTY:
                                if (token == GSGF_TOKEN_PROP_IDENT) {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE;
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                } else {
                                        gsgf_yyerror(&ctx, _("property, ';', or '('"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_VALUE:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else {
                                        gsgf_yyerror(&ctx, _("'['"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_VALUE:
                                if (token == ']')
                                        ctx.state = GSGF_PARSER_STATE_PROPERTIES;
                                else if (token == GSGF_TOKEN_VALUE_TEXT)
                                        ctx.state = GSGF_PARSER_STATE_PROP_CLOSE;
                                else if (token == GSGF_TOKEN_VALUE_NUMBER)
                                        ctx.state = GSGF_PARSER_STATE_PROP_CLOSE;
                                else if (token == GSGF_TOKEN_VALUE_REAL)
                                        ctx.state = GSGF_PARSER_STATE_PROP_CLOSE;
                                else {
                                        gsgf_yyerror(&ctx, _("value or ']'"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }

                                break;
                        case GSGF_PARSER_STATE_PROPERTIES:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                } else {
                                        gsgf_yyerror(&ctx, _("'[', ';', or '('"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_CLOSE:
                                if (token == ']') {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE_READ;
                                } else {
                                        gsgf_yyerror(&ctx, _("']'"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_VALUE_READ:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                } else if (token == GSGF_TOKEN_PROP_IDENT) {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE;
                                } else {
                                        gsgf_yyerror(&ctx, _("'[', ';', '(', ')', or property"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                        case GSGF_PARSER_STATE_GAME_TREES:
                                if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                } else if (token == ')') {
                                        /* State does not change! */
                                        if (!game_tree) {
                                                gsgf_yyerror(&ctx,
                                                             _("Trailing garbage"), 
                                                             token, error);
                                        }
                                        game_tree = gsgf_game_tree_get_parent(game_tree);
                                } else if (token == GSGF_TOKEN_EOF) {
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                } else {
                                        gsgf_yyerror(&ctx, _("'('"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        return self;
                                }
                                break;
                }

                if (value)
                        g_string_free(value, TRUE);

        } while (token != GSGF_TOKEN_EOF);

        if (!game_tree) {
                g_set_error(ctx.error, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                            _("Empty SGF collections are not allowed"));
        } /* else if (!game_tree->first_node ..), actually just else. */

        return self;
}

/**
 * gsgf_collection_parse_file:
 * @file: a #GFile to parse.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Parses a #GFile into a #GSGFCollection in memory.  A return value of
 * non-%NULL does not necessarily mean success.  A parse error normally results
 * in a valid #GSGFCollection object that holds only parts of the information
 * from the stream.  Use @error for error checking instead.
 *
 * See also gsgf_collection_parse_stream ().
 *
 * Returns: A #GSGFCollection or %NULL on error.
 */
GSGFCollection *
gsgf_collection_parse_file(GFile *file, GCancellable *cancellable,
                           GError **error)
{
        GInputStream *stream = G_INPUT_STREAM (g_file_read (file, cancellable, error));

        if (!stream)
                return NULL;

        return gsgf_collection_parse_stream(stream, cancellable, error);
}

static gint gsgf_yylex(GSGFParserContext *ctx, GString **value)
{
        gchar c;

        *value = NULL;

        ctx->start_lineno = ctx->lineno;
        ctx->start_colno = ctx->colno;

        while (1) {
                if (ctx->bufsize == 0 || ctx->bufpos >= ctx->bufsize) {
                        if (0 >= gsgf_yyread(ctx))
                                return -1;
                }

                ++ctx->colno;
                c = ctx->buffer[ctx->bufpos++];

                if (c >= 'A' && c <= 'Z')
                        return gsgf_yyread_prop_ident(ctx, c, value);

                switch (c) {
                        case '(':
                        case ')':
                        case '[':
                        case ']':
                        case ';':
                                return c;
                        case ' ':
                        case '\f':
                        case '\v':
                        case '\t':
                                ctx->start_colno = ctx->colno;
                                break;
                        case '\r':
                        case '\n':
                                c = '\n';
                                gsgf_yyread_linebreak(ctx, c);
                                ctx->colno = ctx->start_colno = 0;
                                ++ctx->lineno;
                                ctx->start_lineno = ctx->lineno;
                                break;
                        default:
                                if (c < ' ' || c >= 127)
                                        g_set_error(
                                                        ctx->error,
                                                        GSGF_ERROR,
                                                        GSGF_ERROR_SYNTAX,
                                                        _("%d:%d:Illegal binary character '#%d'"),
                                                        ctx->start_lineno,
                                                        ctx->start_colno, c);
                                else
                                        g_set_error(
                                                        ctx->error,
                                                        GSGF_ERROR,
                                                        GSGF_ERROR_SYNTAX,
                                                        _("%d:%d:Illegal character '%c'"),
                                                        ctx->start_lineno,
                                                        ctx->start_colno, c);
                                return -1;
                }
        }

        return -1;
}

static gssize gsgf_yyread(GSGFParserContext *ctx)
{
        gssize read_bytes = g_input_stream_read(ctx->stream, ctx->buffer,
                                                sizeof ctx->buffer, ctx->cancellable,
                                                ctx->error);

        if (read_bytes <= 0)
                return read_bytes;

        ctx->bufsize = (gsize) read_bytes;
        ctx->bufpos = 0;

        return read_bytes;
}

static gint gsgf_yyread_prop_ident(GSGFParserContext *ctx, gchar c,
                                   GString **value)
{
        gchar init[2];

        init[0] = c;
        init[1] = 0;
        *value = g_string_new(init);

        while (1) {
                if (ctx->bufsize == 0 || ctx->bufpos >= ctx->bufsize) {
                        if (0 >= gsgf_yyread(ctx)) {
                                g_string_free(*value, TRUE);
                                *value = NULL;
                                return GSGF_TOKEN_EOF;
                        }
                }

                ++ctx->colno;
                c = ctx->buffer[ctx->bufpos++];

                if (c < 'A' || c > 'Z') {
                        --ctx->colno;
                        /* Cannot be zero because we just read a character.  */
                        --ctx->bufpos;
                        break;
                }

                *value = g_string_append_c(*value, c);
        }

        return GSGF_TOKEN_PROP_IDENT;
}

static gint
gsgf_yylex_c_value_type(GSGFParserContext *ctx, GString **value)
{
        gchar c;
        gboolean escaped = FALSE;
        gint token = GSGF_TOKEN_VALUE_TEXT;
        gchar *str;

        *value = g_string_new("");

        ctx->start_lineno = ctx->lineno;
        ctx->start_colno = ctx->colno;

        while (1) {
                if (ctx->bufsize == 0 || ctx->bufpos >= ctx->bufsize) {
                        if (0 >= gsgf_yyread(ctx)) {
                                g_string_free(*value, TRUE);
                                *value = NULL;
                                return GSGF_TOKEN_EOF;
                        }
                }

                ++ctx->colno;
                c = ctx->buffer[ctx->bufpos++];

                /* FIXME! Read all kinds of line endings!  */
                if (c == '\n' || c == '\r') {
                        gsgf_yyread_linebreak(ctx, c);
                        c = '\n';
                        ctx->colno = 0;
                        ++ctx->lineno;
                }

                if (escaped) {
                        escaped = FALSE;
                        if (c == '\n')
                                c  = ' ';
                } else if (c == ']') {
                        --ctx->colno;
                        /* Cannot be zero because we just read a character.  */
                        --ctx->bufpos;
                        break;
                } else if (c == '\\') {
                        escaped = TRUE;
                } else if (c == '\f' || c == '\v' || c == '\t') {
                        c = ' ';
                }

                if (!escaped) {
                        *value = g_string_append_c(*value, c);
                }
        }

        /* Now guess the token type.  Some of the types mentioned by the SGF specs
         * are not used because they cannot be distinguished from other types:
         *
         * - Compose is Text
         * - SimpleText is Text
         * - Double is a number
         * - Color is Text
         * - Point is whatever it looks like
         * - Stone is whatever it looks like
         * - Move is whatever it looks like
         */
        str = (*value)->str;

        if (str[0] == '+' || str[0] == '-' || (str[0] >= '0' && str[0] <= '9')) {
                if (g_regex_match(double_pattern, str, 0, NULL)) {
                        if (index (str, '.'))
                                token = GSGF_TOKEN_VALUE_REAL;
                        else
                                token = GSGF_TOKEN_VALUE_NUMBER;
                }
        }

        return token;
}

static void gsgf_yyread_linebreak(GSGFParserContext *ctx, gchar first)
{
        gchar second;

        if (ctx->bufsize == 0 || ctx->bufpos >= ctx->bufsize) {
                if (0 >= gsgf_yyread(ctx)) {
                        return;
                }
        }

        second = ctx->buffer[ctx->bufpos];
        if ((second == '\r' && first == '\n') || (second == '\n' && first == '\r'))
                ++ctx->bufpos;
}

static void
gsgf_yyerror(GSGFParserContext *ctx, const gchar *expect, gint token, GError **error)
{
        if (token == GSGF_TOKEN_EOF)
                g_set_error(ctx->error, GSGF_ERROR, GSGF_ERROR_SYNTAX,
                            _("%d:%d: Unexpected end of file"),
                            ctx->lineno, ctx->colno);
        else
                g_set_error(ctx->error,
                                GSGF_ERROR,
                                GSGF_ERROR_SYNTAX,
                                _("%d:%d: Expected %s"),
                                ctx->start_lineno,
                                ctx->start_colno + 1, expect);
}

GSGFGameTree *
gsgf_collection_add_game_tree(GSGFCollection *self)
{
        GSGFGameTree *game_tree = gsgf_game_tree_new();

        self->priv->game_trees = 
                g_list_append(self->priv->game_trees, game_tree);

        return game_tree;
}

/**
 * gsgf_collection_write_stream
 * @self: the #GSGFCollection.
 * @out: a #GOutputStream to write to.
 * @bytes_written: number of bytes written to the stream.
 * @close_stream: %TRUE if stream should be losed, %FALSE otherwise.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Serializes a #GSGFCollection and writes the serialized data into
 * a #GOuputStream.
 *
 * If there is an error during the operation FALSE is returned and @error
 * is set to indicate the error status, @bytes_written is updated to contain
 * the number of bytes written into the stream before the error occurred.
 *
 * See also gsgf_collection_write_file().
 *
 * Returns: %TRUE on success.  %FALSE if there was an error.
 **/
gboolean
gsgf_collection_write_stream(const GSGFCollection *self,
                             GOutputStream *out,
                             gsize *bytes_written, gboolean close_stream,
                             GCancellable *cancellable,
                             GError **error)
{
        gsize written_here;
        GList *iter = self->priv->game_trees;

        *bytes_written = 0;

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                            _("Attempt to write an empty collection"));
                return FALSE;
        }

        while (iter) {
                if (!_gsgf_game_tree_write_stream(GSGF_GAME_TREE(iter->data),
                                                  out, &written_here,
                                                  cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;
        }

        if (close_stream && !g_output_stream_close(out, cancellable, error))
                return FALSE;

        return TRUE;
}


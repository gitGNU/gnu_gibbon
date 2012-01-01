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

#include "gsgf-private.h"

enum gsgf_parser_state {
        GSGF_PARSER_STATE_INIT,
        GSGF_PARSER_STATE_PROPERTY,
        GSGF_PARSER_STATE_NODE,
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

typedef struct _GSGFCollectionPrivate GSGFCollectionPrivate;
struct _GSGFCollectionPrivate {
        GList* game_trees;
};

#define GSGF_COLLECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COLLECTION,           \
                                      GSGFCollectionPrivate))

static void gsgf_component_iface_init (GSGFComponentIface *iface);
G_DEFINE_TYPE_WITH_CODE (GSGFCollection, gsgf_collection, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSGF_TYPE_COMPONENT,
                                                gsgf_component_iface_init))

#define GSGF_TOKEN_EOF 256
#define GSGF_TOKEN_PROP_IDENT 257
#define GSGF_TOKEN_VALUE 258

static gint gsgf_yylex(GSGFParserContext *ctx, GString **value);
static gint gsgf_yylex_c_value_type(GSGFParserContext *ctx, GString **value);
static gssize gsgf_yyread(GSGFParserContext *ctx);
static gint gsgf_yyread_prop_ident(GSGFParserContext *ctx, gchar c,
                                   GString **value);
static void gsgf_yyread_linebreak(GSGFParserContext *ctx, gchar c);
static void gsgf_yyerror(GSGFParserContext *ctx, const gchar *expect,
                         gint token, GError **error);
static gboolean gsgf_collection_convert (GSGFComponent *collection,
                                         const gchar *charset,
                                         GError **error);
static gboolean gsgf_collection_cook (GSGFComponent *collection,
                                      GSGFComponent **culprit,
                                      GError **error);
static gboolean gsgf_collection_write_stream (const GSGFComponent *self,
                                              GOutputStream *out,
                                              gsize *bytes_written,
                                              GCancellable *cancellable,
                                              GError **error);

/*
 * The SGF specification stipulates that a collection must have one ore more 
 * game trees, and that a game tree has one or more nodes.  For practical
 * purposes we allow empty collections and empty node lists (sequences).
 * But if you try to serialize such an object into a stream, you will
 * get an error.
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

        _libgsgf_init();

        object_class->finalize = gsgf_collection_finalize;
}

static void
gsgf_component_iface_init (GSGFComponentIface *iface)
{
        iface->write_stream = gsgf_collection_write_stream;
        iface->cook = gsgf_collection_cook;
        iface->_convert = gsgf_collection_convert;
}

/**
 * gsgf_collection_new:
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Build an empty #GSGFCollection in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFCollection.
 */
GSGFCollection *
gsgf_collection_new(GError **error)
{
        GSGFCollection *self = g_object_new(GSGF_TYPE_COLLECTION, NULL);

        return self;
}

/**
 * gsgf_collection_parse_stream:
 * @stream: a #GInputStream to parse.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Parses an input stream into a #GSGFCollection in memory
 *
 * See also gsgf_collection_parse_file ().
 *
 * Returns: A #GSGFCollection or %NULL on error.
 */
GSGFCollection *
gsgf_collection_parse_stream(GInputStream *stream,
                             GCancellable *cancellable, GError **error)
{
        GSGFCollection *self;
        gint token = 0;
        GString *value;
        GSGFParserContext ctx;

        gsgf_return_val_if_fail (G_IS_INPUT_STREAM (stream), NULL, error);

        self = gsgf_collection_new(error);
        if (!self)
                return NULL;

        GSGFGameTree *game_tree = NULL;
        GSGFNode *node = NULL;
        GSGFProperty *property = NULL;

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
                                        game_tree =
                                                gsgf_collection_add_game_tree (
                                                                self, NULL);
                                        node = NULL;
                                        property = NULL;
                                } else {
                                        gsgf_yyerror(&ctx, _("'('"), token, error);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_NODE:
                                if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                        node = gsgf_game_tree_add_node(game_tree);
                                        property = NULL;
                                } else {
                                        gsgf_yyerror(&ctx, _("';'"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROPERTY:
                                if (token == GSGF_TOKEN_PROP_IDENT) {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE;
                                        property = gsgf_node_add_property(node,
                                                                          value->str,
                                                                          error);
                                        if (!property) {
                                                g_prefix_error(error, "%d:%d:",
                                                               ctx.lineno, ctx.colno);
                                                g_string_free(value, TRUE);
                                                g_object_unref (self);
                                                return NULL;
                                        }
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                        node = gsgf_game_tree_add_node(game_tree);
                                        property = NULL;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                        node = NULL;
                                        property = NULL;
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                        game_tree = gsgf_game_tree_get_parent(game_tree);
                                } else {
                                        gsgf_yyerror(&ctx, _("property, ';', or '('"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_VALUE:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else {
                                        gsgf_yyerror(&ctx, _("'['"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_VALUE:
                                if (token == ']') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTIES;
                                } else if (token == GSGF_TOKEN_VALUE) {
                                        ctx.state = GSGF_PARSER_STATE_PROP_CLOSE;
                                        _gsgf_property_add_value(property, value->str);
                                } else {
                                        gsgf_yyerror(&ctx, _("value or ']'"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }

                                break;
                        case GSGF_PARSER_STATE_PROPERTIES:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                        node = gsgf_game_tree_add_node(game_tree);
                                        property = NULL;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                        node = NULL;
                                        property = NULL;
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                        game_tree = gsgf_game_tree_get_parent(game_tree);
                                        node = NULL;
                                        property = NULL;
                                } else {
                                        gsgf_yyerror(&ctx, _("'[', ';', or '('"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_CLOSE:
                                if (token == ']') {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE_READ;
                                } else {
                                        gsgf_yyerror(&ctx, _("']'"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_PROP_VALUE_READ:
                                if (token == '[') {
                                        ctx.state = GSGF_PARSER_STATE_VALUE;
                                } else if (token == ';') {
                                        ctx.state = GSGF_PARSER_STATE_PROPERTY;
                                        node = gsgf_game_tree_add_node(game_tree);
                                        property = NULL;
                                } else if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        game_tree = gsgf_game_tree_add_child(game_tree);
                                        node = NULL;
                                        property = NULL;
                                } else if (token == ')') {
                                        ctx.state = GSGF_PARSER_STATE_GAME_TREES;
                                        game_tree = gsgf_game_tree_get_parent(game_tree);
                                        node = NULL;
                                        property = NULL;
                                } else if (token == GSGF_TOKEN_PROP_IDENT) {
                                        ctx.state = GSGF_PARSER_STATE_PROP_VALUE;
                                        property = gsgf_node_add_property(node,
                                                                          value->str,
                                                                          error);
                                        if (!property) {
                                                g_prefix_error(error, "%d:%d:",
                                                               ctx.lineno, ctx.colno);
                                                g_string_free(value, TRUE);
                                                g_object_unref (self);
                                                return NULL;
                                        }
                                } else {
                                        gsgf_yyerror(&ctx, _("'[', ';', '(', ')', or property"),
                                                     token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                        case GSGF_PARSER_STATE_GAME_TREES:
                                if (token == '(') {
                                        ctx.state = GSGF_PARSER_STATE_NODE;
                                        if (game_tree) {
                                                game_tree = gsgf_game_tree_add_child(game_tree);
                                        } else {
                                                game_tree = gsgf_collection_add_game_tree(self,
                                                                                          NULL);
                                        }
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
                                        g_object_unref (self);
                                        return NULL;
                                } else {
                                        gsgf_yyerror(&ctx, _("'('"), token, error);
                                        if (value)
                                                g_string_free(value, TRUE);
                                        g_object_unref (self);
                                        return NULL;
                                }
                                break;
                }

                if (value)
                        g_string_free(value, TRUE);

        } while (token != GSGF_TOKEN_EOF);

        if (!self->priv->game_trees) {
                g_set_error(ctx.error, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                            _("Empty SGF collections are not allowed"));
                g_object_unref (self);
                return NULL;
        }

        if (!gsgf_collection_convert (GSGF_COMPONENT (self), "ISO-8859-1",
                                      ctx.error)) {
                g_object_unref (self);
                return NULL;
        }

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
        GInputStream *stream;

        gsgf_return_val_if_fail (G_IS_FILE (file), NULL, error);
        stream = G_INPUT_STREAM (g_file_read (file, cancellable, error));
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
                                                        _("%d:%d: Illegal binary character '#%d'"),
                                                        ctx->start_lineno,
                                                        ctx->start_colno, c);
                                else
                                        g_set_error(
                                                        ctx->error,
                                                        GSGF_ERROR,
                                                        GSGF_ERROR_SYNTAX,
                                                        _("%d:%d: Illegal character '%c'"),
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
        gint token = GSGF_TOKEN_VALUE;

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

                if (c == '\n' || c == '\r') {
                        gsgf_yyread_linebreak(ctx, c);
                        c = '\n';
                        ctx->colno = 0;
                        ++ctx->lineno;
                }

                if (escaped) {
                        escaped = FALSE;
                } else if (c == ']') {
                        --ctx->colno;
                        /* Cannot be zero because we just read a character.  */
                        --ctx->bufpos;
                        break;
                } else if (c == '\\') {
                        escaped = TRUE;
                }

                *value = g_string_append_c(*value, c);
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

/**
 * gsgf_collection_add_game_tree:
 * @self: The #GSGFCollection to extend.
 * @flavor: The #GSGFFlavor to use.
 *
 * Adds a fresh, empty #GSGFGameTree instance to a #GSGFCollection.
 *
 * Returns: The freshly created #GSGFGameTree.
 */
GSGFGameTree *
gsgf_collection_add_game_tree (GSGFCollection *self, const GSGFFlavor *flavor)
{
        GSGFGameTree *game_tree;

        g_return_val_if_fail (GSGF_IS_COLLECTION (self), NULL);

        game_tree = _gsgf_game_tree_new (flavor);

        self->priv->game_trees = 
                g_list_append(self->priv->game_trees, game_tree);

        return game_tree;
}

static gboolean
gsgf_collection_write_stream (const GSGFComponent *_self,
                              GOutputStream *out,
                              gsize *bytes_written,
                              GCancellable *cancellable,
                              GError **error)
{
        gsize written_here;
        GSGFCollection *self = GSGF_COLLECTION (_self);
        GList *iter = self->priv->game_trees;

        gsgf_return_val_if_fail (bytes_written != NULL, FALSE, error);

        *bytes_written = 0;

        gsgf_return_val_if_fail (GSGF_IS_COLLECTION (self), FALSE, error);
        gsgf_return_val_if_fail (G_IS_OUTPUT_STREAM (out), FALSE, error);

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                            _("Attempt to write an empty collection"));
                return FALSE;
        }

        while (iter) {
                if (!gsgf_component_write_stream(GSGF_COMPONENT (iter->data),
                                                 out, &written_here,
                                                 cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                *bytes_written += written_here;

                if (!g_output_stream_write_all(out, "\n", 1, &written_here,
                                               cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                *bytes_written += written_here;

                iter = iter->next;
        }

        return TRUE;
}

static gboolean
gsgf_collection_convert (GSGFComponent *_self, const gchar *charset,
                         GError **error)
{
        GSGFCollection *self = GSGF_COLLECTION (_self);
        GList *iter = self->priv->game_trees;
        GSGFComponentIface *iface;

        if (error)
                *error = NULL;

        while (iter) {
                iface = GSGF_COMPONENT_GET_IFACE (iter->data);
                if (!iface->_convert(GSGF_COMPONENT (iter->data),
                                                     charset, error)) {
                        return FALSE;
                }
                iter = iter->next;
        }

        return TRUE;
}

static gboolean
gsgf_collection_cook (GSGFComponent *_self, GSGFComponent **culprit,
                      GError **error)
{
        GSGFCollection *self = GSGF_COLLECTION (_self);
        GList *iter = self->priv->game_trees;
        GSGFComponentIface *iface;

        if (error)
                *error = NULL;

        while (iter) {
                iface = GSGF_COMPONENT_GET_IFACE (iter->data);
                if (!iface->cook (GSGF_COMPONENT (iter->data), culprit,
                                  error)) {
                        if (culprit && !*culprit)
                                *culprit = _self;
                        return FALSE;
                }

                iter = iter->next;
        }

        return TRUE;
}

/**
 * gsgf_collection_get_game_trees
 * @self: the #GSGFCollection.
 *
 * Get the list of #GSGFGameTree objects stored in a #GSGFCollection.
 *
 * This list is not a copy.  You should not free it.  The list becomes invalid,
 * when you add or remove game trees.
 *
 * Returns: Returns a #GList of #GSGFGameTree objects..
 **/
GList *
gsgf_collection_get_game_trees (const GSGFCollection *self)
{
        g_return_val_if_fail (GSGF_IS_COLLECTION(self), NULL);

        return self->priv->game_trees;
}

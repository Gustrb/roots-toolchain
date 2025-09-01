/*
	Typex preprocessor

	The typex preprocessor is the software responsible for tokenizing the whole codebase, identifying the macros
	And applying them.
*/
#include "../lib/typex.h"

#include <string.h>

#define ISBLANK(c) ( (c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r' )
#define IS_BIN(c) ((c) == '0' || (c) == '1')
#define IS_HEX(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define IS_OCT(c) (((c) >= '0' && (c) <= '7'))

#define IS_ALPHABETIC(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) == '_')
#define IS_NUMERIC(c) (c) >= '0' && (c) <= '9'
#define IS_ALPHANUMERIC(c) IS_ALPHABETIC(c) || IS_NUMERIC(c)

#define __N_DIRECTIVES 1

typedef enum
{
	DIRECTIVE_ERROR,
	DIRECTIVE_DEFINE,
} directive_type_t;

typedef struct
{
	directive_type_t t;
	unsigned long long v;
} directive_pair_t;

static const directive_pair_t __directives[__N_DIRECTIVES] = {
	{ .t = DIRECTIVE_DEFINE, .v = 5500604938257788306ULL },
};

static const char *__directives_str[__N_DIRECTIVES] = {
	"define",
};

int typex_first_pass(const char *stream, size_t in_len, typex_context_t *root_ctx);
int __typex_define_macro(typex_lexer_t *, typex_context_t *ctx);

static unsigned long long __hash(const char *, size_t, size_t);
static directive_type_t __figure_out_directive(const char *, size_t linebegin, size_t lineend);
static char __str_eql(const char *b1, const char *e1, const char *b2, const char *e2);

int typex_preprocess(const char *in, size_t in_len, owned_str_t *out)
{
    typex_context_t root;
    int err = typex_new_ctx(&root, in);
    if (err)
    {
        return err;
    }

    if ((err == typex_first_pass(in, in_len, &root)) != 0)
    {
        return err;
    }

	(void) out;
	(void)__figure_out_directive;
	(void)__hash;

	return 0;
}
#include <stdio.h>

int typex_first_pass(const char *stream, size_t in_len, typex_context_t *root_ctx)
{
    (void)root_ctx;

    typex_lexer_t l = { .len=in_len, .stream=stream, .pos=0 };
    typex_token_t tok;
    int err;

    while ((err = typex_lexer_next_token(&l, &tok)) == 0)
    {
        if (err)
        {
            return err;
        }

        if (tok.t == TYPEX_TOKEN_TYPE_EOF)
        {
            break;
        }

        if (tok.t == TYPEX_TOKEN_TYPE_WORD)
        {
            continue;
        }


        if (tok.t == TYPEX_TOKEN_TYPE_MACRO)
        {
            // We need to figure out which directive this is...
            directive_type_t dirt = __figure_out_directive(stream, tok.begin, tok.end);
            switch (dirt)
            {
                case DIRECTIVE_ERROR:
                {
                    // TODO: handle the error message
                }; break;
                case DIRECTIVE_DEFINE:
                {
                    return __typex_define_macro(&l, root_ctx);
                }; break;
            }
        }
    }

    return 0;
}

int __typex_define_macro(typex_lexer_t *l, typex_context_t *ctx)
{
    typex_directive_define_t d;

    // We need to get the macro name, it is the next token
    typex_token_t t;
    int err = typex_lexer_next_token(l, &t);
    if (err)
    {
        return err;
    }

    d.name_begin = t.begin;
    d.name_end = t.end;

    err = typex_lexer_next_token(l, &t);
    if (err)
    {
        return err;
    }

    d.replacement_begin = t.begin;
    d.replacement_end = t.end;

    return typex_define_replacement(ctx, &d);
}

static directive_type_t __figure_out_directive(const char *stream, size_t linebegin, size_t lineend)
{
	unsigned long long h = __hash(stream, linebegin, lineend);

	size_t low = 0;
	size_t high = __N_DIRECTIVES-1;

	if (low == high)
	{
		if (__directives[low].v == h) return __directives[low].t;
	}

	while (low <= high)
	{
		size_t i = (low+high)/2;
		if (__directives[i].v == h)
		{
			size_t l = lineend-linebegin;
			if (l == strlen(__directives_str[i]))
			{
				return __directives[i].t;
			}
			return DIRECTIVE_ERROR;
		}
		else if (__directives[i].v < h)
		{
			low = i+1;
		}
		else
		{
			if (i == 0) break;
			high = i-1;
		}
	}

	return DIRECTIVE_ERROR;
}

int typex_new_ctx(typex_context_t *ctx, const char *stream)
{
	ctx->stream = stream;
	ctx->definitions_len = 0;
	for (size_t i = 0; i < __TYPEX_MAX_DEFINITIONS; ++i)
	{
		typex_directive_define_t *d = &ctx->definitions[i];

		d->next = -1;
		d->prev = -1;
	}

	for (size_t i = 0; i < __TYPEX_TABLE_SIZE; ++i)
	{
		ctx->buckets[i] = -1;
	}

	return 0;
}

int typex_define_replacement(typex_context_t *ctx, typex_directive_define_t *d)
{
	if (ctx->definitions_len == __TYPEX_MAX_DEFINITIONS)
	{
		return E_TYPEXTOOMANYDEFINES;
	}

	size_t new_define_idx = ctx->definitions_len;
	ctx->definitions[ctx->definitions_len++] = *d;

	unsigned long long h = __hash(ctx->stream, d->name_begin, d->name_end);
	size_t bucket_idx = (size_t)h % __TYPEX_TABLE_SIZE;

	if (ctx->buckets[bucket_idx] == -1)
	{
		ctx->buckets[bucket_idx] = new_define_idx;

		ctx->definitions[new_define_idx].prev = -1;
		ctx->definitions[new_define_idx].next = -1;
		return 0;
	}

	size_t aux_idx = bucket_idx;
	typex_directive_define_t *aux = &ctx->definitions[bucket_idx];
	while (aux->next != -1)
	{
		aux = &ctx->definitions[aux->next];
		aux_idx = aux->next;
	}

	aux->next = new_define_idx;
	ctx->definitions[new_define_idx].prev = aux_idx;

	return 0;
}

int typex_define_replacement_lookup(typex_context_t *ctx, size_t name_begin, size_t name_end, typex_directive_define_t *d)
{
	unsigned long long h = __hash(ctx->stream, name_begin, name_end);
	size_t bucket_idx = (size_t)h % __TYPEX_TABLE_SIZE;

	if (ctx->buckets[bucket_idx] == -1)
	{
		return E_KEYNOTFOUND;
	}

	typex_directive_define_t *aux = &ctx->definitions[ctx->buckets[bucket_idx]];
	while (!__str_eql(ctx->stream + aux->name_begin, ctx->stream + aux->name_end, ctx->stream + name_begin, ctx->stream + name_end))
	{
		if (aux->next == -1)
		{
			return E_KEYNOTFOUND;
		}

		aux = &ctx->definitions[aux->next];
	}

	d->name_begin = aux->name_begin;
	d->name_end = aux->name_end;

	d->replacement_begin = aux->replacement_begin;
	d->replacement_end = aux->replacement_end;

	return 0;
}

static unsigned long long __hash(const char *d, size_t f, size_t l)
{
	unsigned long long o = 0xcbf29ce484222325ULL;
	for (size_t i = f; i < l; ++i)
	{
		o = (o ^ d[i]) * 0x100000001b3ULL;
	}

	return o;
}

static char __str_eql(const char *b1, const char *e1, const char *b2, const char *e2)
{
	size_t s1 = e1 - b1;
	size_t s2 = e2 - b2;

	if (s1 != s2) return 0;

	for (size_t i = 0; i < s1; ++i)
	{
		if (b1[i] != b2[i])
		{
			return 0;
		}
	}

	return 1;
}

typedef enum
{
    __TYPEX_LEXER_STATE_START = 0,
    __TYPEX_LEXER_STATE_MACRO = 1,
    __TYPEX_LEXER_STATE_ALPHA = 2,
    __TYPEX_LEXER_STATE_STRING = 3,
    __TYPEX_LEXER_STATE_CHAR = 4,
    __TYPEX_LEXER_STATE_NUMERIC_LITERAL = 5,
    __TYPEX_LEXER_STATE_FLOAT_LITERAL = 6,
    __TYPEX_LEXER_STATE_EXPONENT_LITERAL = 7,
    __TYPEX_LEXER_STATE_DIFFERENT_NUMBER_SYSTEM = 8,
    __TYPEX_LEXER_STATE_BINARY_LITERAL = 9,
    __TYPEX_LEXER_STATE_OCTAL_LITERAL = 10,
    __TYPEX_LEXER_STATE_HEXADECIMAL_LITERAL = 11,
} __typex_lexer_state_t;

#include <stdio.h>

int typex_lexer_next_token(typex_lexer_t *lexer, typex_token_t *token)
{
    __typex_lexer_state_t state = __TYPEX_LEXER_STATE_START;

    size_t begin = lexer->pos;

    while (lexer->pos < lexer->len)
    {
        char curr = lexer->stream[lexer->pos];
        switch (state)
        {
            case __TYPEX_LEXER_STATE_START:
            {
                if (IS_ALPHABETIC(curr))
                {
                    state = __TYPEX_LEXER_STATE_ALPHA;
                    begin = lexer->pos;
                    break;
                }

                if (IS_NUMERIC(curr))
                {
                    state = __TYPEX_LEXER_STATE_NUMERIC_LITERAL;
                    begin = lexer->pos;
                    break;
                }

                switch (curr)
                {
                    case ' ': case '\t': case '\r': case '\n':
                    {
                        ++lexer->pos;
                    }; break;

                    case '"':
                    {
                        state = __TYPEX_LEXER_STATE_STRING;
                        begin = lexer->pos;
                        lexer->pos++;
                    }; break;

                    case '\'':
                    {
                        state = __TYPEX_LEXER_STATE_CHAR;
                        begin = lexer->pos;
                        lexer->pos++;
                    }; break;

                    case '#':
                    {
                        state = __TYPEX_LEXER_STATE_MACRO;
                        begin = lexer->pos + 1;
                    }; break;

                    default:
                    {
                        token->begin = lexer->pos;
                        token->end = ++lexer->pos;
                        token->t = TYPEX_TOKEN_TYPE_WORD;
                        return 0;
                    }; break;
               }
            }; break;
            case __TYPEX_LEXER_STATE_MACRO:
            {
                while (curr != ' ' && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                if (lexer->pos == lexer->len)
                {
                    return E_TYPEX_ERR_UNEXPECTED_EOF;
                }

                token->t = TYPEX_TOKEN_TYPE_MACRO;
                token->begin = begin;
                token->end = lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_ALPHA:
            {
                while (IS_ALPHANUMERIC(curr) && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_STRING:
            {
                while (curr != '"' && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                if (lexer->pos == lexer->len)
                {
                    return E_TYPEX_ERR_UNEXPECTED_EOF;
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = ++lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_CHAR:
            {
                while (curr != '\'' && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                if (lexer->pos == lexer->len)
                {
                    return E_TYPEX_ERR_UNEXPECTED_EOF;
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = ++lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_NUMERIC_LITERAL:
            {
                if (curr == '0')
                {
                    ++lexer->pos;
                    state = __TYPEX_LEXER_STATE_DIFFERENT_NUMBER_SYSTEM;
                    break;
                }

                while (IS_NUMERIC(curr) && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                if (curr == '.')
                {
                    ++lexer->pos;
                    state = __TYPEX_LEXER_STATE_FLOAT_LITERAL;
                    break;
                }

                if (curr == 'e' || curr == 'E')
                {
                    ++lexer->pos;
                    state = __TYPEX_LEXER_STATE_EXPONENT_LITERAL;
                    break;
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_FLOAT_LITERAL:
            {
                while (IS_NUMERIC(curr) && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                if (curr == 'e' || curr == 'E')
                {
                    ++lexer->pos;
                    state = __TYPEX_LEXER_STATE_EXPONENT_LITERAL;
                    break;
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_EXPONENT_LITERAL:
            {
                if (curr == '+' || curr == '-')
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                while (IS_NUMERIC(curr) && lexer->pos < lexer->len)
                {
                    ++lexer->pos;
                    curr = lexer->stream[lexer->pos];
                }

                token->t = TYPEX_TOKEN_TYPE_WORD;
                token->begin = begin;
                token->end = lexer->pos;
                return 0;
            }; break;

            case __TYPEX_LEXER_STATE_DIFFERENT_NUMBER_SYSTEM:
            {
                if (curr == 'b' || curr == 'B')
                {
                    lexer->pos++;
                    state = __TYPEX_LEXER_STATE_BINARY_LITERAL;
                    break;
                }
                if (curr == 'x' || curr == 'X')
                {
                    lexer->pos++;
                    state = __TYPEX_LEXER_STATE_HEXADECIMAL_LITERAL;
                    break;
                }

                // Assume octal
                state = __TYPEX_LEXER_STATE_OCTAL_LITERAL;
                lexer->pos++;
                break;
            }; break;

            case __TYPEX_LEXER_STATE_BINARY_LITERAL:
            {
                if (!IS_BIN(curr))
                {
                    token->t = TYPEX_TOKEN_TYPE_WORD;
                    token->begin = begin;
                    token->end = lexer->pos;
                    return 0;
                }

                ++lexer->pos;
                curr = lexer->stream[lexer->pos];
            }; break;

            case __TYPEX_LEXER_STATE_HEXADECIMAL_LITERAL:
            {
                if (!IS_HEX(curr))
                {
                    token->t = TYPEX_TOKEN_TYPE_WORD;
                    token->begin = begin;
                    token->end = lexer->pos;
                    return 0;
                }

                ++lexer->pos;
                curr = lexer->stream[lexer->pos];
            }; break;

            case __TYPEX_LEXER_STATE_OCTAL_LITERAL:
            {
                if (!IS_OCT(curr))
                {
                    token->t = TYPEX_TOKEN_TYPE_WORD;
                    token->begin = begin;
                    token->end = lexer->pos;
                    return 0;
                }

                ++lexer->pos;
                curr = lexer->stream[lexer->pos];
            }; break;
        }
    }

    if (state == __TYPEX_LEXER_STATE_BINARY_LITERAL || state == __TYPEX_LEXER_STATE_HEXADECIMAL_LITERAL || state == __TYPEX_LEXER_STATE_OCTAL_LITERAL)
    {
        token->t = TYPEX_TOKEN_TYPE_WORD;
        token->begin = begin;
        token->end = lexer->pos;
        return 0;
    }

    token->t = TYPEX_TOKEN_TYPE_EOF;

    return 0;
}

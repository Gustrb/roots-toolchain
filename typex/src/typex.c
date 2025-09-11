/*
	Typex preprocessor

	The typex preprocessor is the software responsible for tokenizing the whole codebase, identifying the macros
	And applying them.
*/
#include "../lib/typex.h"

#include <string.h>
#include <assert.h>

#define ISBLANK(c) ( (c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r' )
#define IS_BIN(c) ((c) == '0' || (c) == '1')
#define IS_HEX(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define IS_OCT(c) (((c) >= '0' && (c) <= '7'))

#define IS_ALPHABETIC(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) == '_')
#define IS_NUMERIC(c) (c) >= '0' && (c) <= '9'
#define IS_ALPHANUMERIC(c) IS_ALPHABETIC(c) || IS_NUMERIC(c)

#define __N_DIRECTIVES 2

typedef enum
{
	DIRECTIVE_ERROR,
	DIRECTIVE_DEFINE,
	DIRECTIVE_UNDEFINE,
} directive_type_t;

typedef struct
{
	directive_type_t t;
	unsigned long long v;
} directive_pair_t;

static const directive_pair_t __directives[__N_DIRECTIVES] = {
	{ .t = DIRECTIVE_DEFINE, .v = 5500604938257788306ULL },
	{ .t = DIRECTIVE_UNDEFINE, .v = 12228404724271304561ULL },
};

static const char *__directives_str[__N_DIRECTIVES] = {
	"define",
	"undef",
};

int owned_str_append(owned_str_t *, owned_str_t *);

int typex_first_pass(const char *stream, size_t in_len, typex_context_t *root_ctx);
int typex_second_pass(const char *stream, size_t in_len, typex_context_t *root_ctx, owned_str_t *out);

int __typex_consume_define(typex_lexer_t *);
int __typex_consume_undefine(typex_lexer_t *);

int __typex_define_macro(typex_lexer_t *, typex_context_t *ctx);
int __typex_undefine_macro(typex_lexer_t *, typex_context_t *ctx);

static unsigned long long __hash(const char *, size_t, size_t);
static directive_type_t __figure_out_directive(const char *, size_t linebegin, size_t lineend);
static char __str_eql(const char *b1, const char *e1, const char *b2, const char *e2);

#include <stdio.h>

int typex_preprocess(const char *in, size_t in_len, owned_str_t *out)
{
    typex_context_t root;
    int err = typex_new_ctx(&root, in);
    if (err)
    {
        return err;
    }

    if ((err = typex_first_pass(in, in_len, &root)) != 0)
    {
        return err;
    }


    if ((err = typex_second_pass(in, in_len, &root, out)) != 0)
    {
        return err;
    }

	return 0;
}


int typex_first_pass(const char *stream, size_t in_len, typex_context_t *root_ctx)
{
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

        if (tok.t == TYPEX_TOKEN_TYPE_WHITESPACE)
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
                    if ((err = __typex_define_macro(&l, root_ctx)) != 0)
                    {
                        return err;
                    }
                }; break;
                case DIRECTIVE_UNDEFINE:
                {
                    if ((err = __typex_undefine_macro(&l, root_ctx)) != 0)
                    {
                        return err;
                    }
                }; break;
            }
        }
    }

    return 0;
}

int typex_second_pass(const char *stream, size_t in_len, typex_context_t *root_ctx, owned_str_t *out)
{
    out->len = 0;
    out->buff = NULL;

    int err;
    typex_lexer_t l = { .len=in_len, .stream=stream, .pos=0 };
    typex_token_t t;

    while ((err = typex_lexer_next_token(&l, &t)) == 0)
    {
        if (t.t == TYPEX_TOKEN_TYPE_EOF) break;
        if (t.t == TYPEX_TOKEN_TYPE_MACRO)
        {
            directive_type_t dir = __figure_out_directive(stream, t.begin, t.end);
            switch (dir)
            {
                case DIRECTIVE_DEFINE:
                {
                    if ((err = __typex_consume_define(&l)) != 0)
                    {
                        return err;
                    }
                }; break;
                case DIRECTIVE_UNDEFINE:
                {
                    if ((err = __typex_consume_undefine(&l)) != 0)
                    {
                        return err;
                    }
                }; break;
                case DIRECTIVE_ERROR:
                {
                    return E_INVALIDMACRODIRECTIVE;
                }; break;
            }

            continue;
        }

        if (t.t == TYPEX_TOKEN_TYPE_WHITESPACE)
        {
            owned_str_t ws = { .len = t.end - t.begin, .buff = (char *) stream + t.begin };
            err = owned_str_append(out, &ws);
            if (err)
            {
                return err;
            }

            continue;
        }

        // if it is a word, we need to check if it is inside our context
        assert(t.t == TYPEX_TOKEN_TYPE_WORD);

        typex_directive_define_t d;
        int lookup = typex_define_replacement_lookup(root_ctx, t.begin, t.end, &d);
        if (lookup == E_KEYNOTFOUND)
        {
            // we need to copy the word to the output
            owned_str_t word = { .len = t.end - t.begin, .buff = (char *) stream + t.begin };
            err = owned_str_append(out, &word);
            if (err)
            {
                return err;
            }

            continue;
        }

        if (lookup != 0)
        {
            // something weird happened over here
            return lookup;
        }

        // Lookup succeeded, we need to replace
        owned_str_t word = { .len = d.replacement_end - d.replacement_begin, .buff = (char *) stream + d.replacement_begin };
        if ((err = owned_str_append(out, &word)) != 0)
        {
            return err;
        }
    }

    return 0;
}

// skips the whole define directive, from the directive, to the last whitespace
int __typex_consume_define(typex_lexer_t *l)
{
    // We need to consume tokens until we hit a newline character.
    // The newline character might be part of a TYPEX_TOKEN_TYPE_WHITESPACE token,
    // or it might be the start of one.
    while (l->pos < l->len)
    {
        char current_char = l->stream[l->pos];
        if (current_char == '\n')
        {
            l->pos++; // Consume the newline character itself
            return 0;
        }
        l->pos++; // Advance the lexer position
    }

    // If we reach EOF without a newline, that's also considered consumed
    return 0;
}

int __typex_consume_undefine(typex_lexer_t *l)
{
    while (l->pos < l->len)
    {
        char current_char = l->stream[l->pos];
        if (current_char == '\n')
        {
            l->pos++; // Consume the newline character itself
            return 0;
        }
        l->pos++; // Advance the lexer position
    }

    return 0;
}

int owned_str_append(owned_str_t *dst, owned_str_t *src)
{
    size_t new_len = dst->len + src->len;
    char *new_buff = realloc(dst->buff, new_len);

    if (!new_buff)
    {
        return E_TYPEX_OUTOFMEM;
    }

    for (size_t i = 0; i < src->len; i++)
    {
        new_buff[dst->len + i] = src->buff[i];
    }

    dst->len = new_len;
    dst->buff = new_buff;

    return 0;
}

int __typex_define_macro(typex_lexer_t *l, typex_context_t *ctx)
{
    typex_directive_define_t d;

    // We need to get the macro name, it is the next token
    typex_token_t t;
    int err;

    while (t.t != TYPEX_TOKEN_TYPE_WORD)
    {
        err = typex_lexer_next_token(l, &t);
        if (err)
        {
            return err;
        }
    }

    d.name_begin = t.begin;
    d.name_end = t.end;

    t.t = TYPEX_TOKEN_TYPE_EOF;

    while (t.t != TYPEX_TOKEN_TYPE_WORD)
    {
        err = typex_lexer_next_token(l, &t);
        if (err)
        {
            return err;
        }
    }

    d.replacement_begin = t.begin;
    d.replacement_end = t.end;

    return typex_define_replacement(ctx, &d);
}

int __typex_undefine_macro(typex_lexer_t *l, typex_context_t *ctx)
{
    typex_directive_undefine_t d;

    typex_token_t t = {.t = TYPEX_TOKEN_TYPE_EOF};
    int err;

    while (t.t != TYPEX_TOKEN_TYPE_WORD)
    {
        err = typex_lexer_next_token(l, &t);
        if (err)
        {
            return err;
        }
    }
    d.name_begin = t.begin;
    d.name_end = t.end;

    // Step 2: Perform the undefine operation in the context
    err = typex_undefine_replacement(ctx, &d);
    if (err) {
        return err; // Propagate any errors from undefining
    }

    // Step 3: Consume the rest of the line (up to and including the newline)
    // This is crucial for the lexer's state for the rest of the file
    while (l->pos < l->len)
    {
        char current_char = l->stream[l->pos];
        if (current_char == '\n')
        {
            l->pos++; // Consume the newline character itself
            return 0; // Directive line fully consumed
        }
        l->pos++; // Advance the lexer position
    }
    return 0;
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

int typex_undefine_replacement(typex_context_t *ctx, typex_directive_undefine_t *u)
{
    unsigned long long h = __hash(ctx->stream, u->name_begin, u->name_end);
    size_t bucket_idx = (size_t)h % __TYPEX_TABLE_SIZE;

    int current_def_idx = ctx->buckets[bucket_idx];
    int target_def_idx = -1;

    // Iterate through the linked list in the bucket to find the definition
    while (current_def_idx != -1)
    {
        typex_directive_define_t *current_def = &ctx->definitions[current_def_idx];

        if (__str_eql(ctx->stream + current_def->name_begin, ctx->stream + current_def->name_end,
                      ctx->stream + u->name_begin, ctx->stream + u->name_end))
        {
            target_def_idx = current_def_idx;
            break; // Found the definition to undefine
        }
        current_def_idx = current_def->next;
    }

    if (target_def_idx == -1)
    {
        return E_KEYNOTFOUND; // Macro not found in this bucket or at all
    }

    // Now, `target_def_idx` holds the index of the definition to remove.
    typex_directive_define_t *to_remove = &ctx->definitions[target_def_idx];

    // Update the 'next' pointer of the previous element or the bucket head
    if (to_remove->prev == -1) // This is the first element in the bucket's list
    {
        ctx->buckets[bucket_idx] = to_remove->next;
    }
    else // It has a previous element
    {
        ctx->definitions[to_remove->prev].next = to_remove->next;
    }

    // Update the 'prev' pointer of the next element (if it exists)
    if (to_remove->next != -1)
    {
        ctx->definitions[to_remove->next].prev = to_remove->prev;
    }

    // Optional: Reset prev/next of the removed element (good practice)
    to_remove->prev = -1;
    to_remove->next = -1;

    ctx->definitions_len--; // Decrement the count of active definitions

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

                        token->begin = lexer->pos;
                        token->end = ++lexer->pos;
                        token->t = TYPEX_TOKEN_TYPE_WHITESPACE;
                        return 0;
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

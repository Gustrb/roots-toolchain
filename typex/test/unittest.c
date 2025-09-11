#include <stdio.h>
#include <string.h>

#include "lib.h"
#include "../lib/typex.h"

int __should_initialize_the_context_properly(void);
int __should_be_able_to_define_a_directive_in_the_context(void);
int __should_be_able_to_undefine_a_directive_in_the_context(void);
int __should_be_able_to_find_a_directive_in_the_context(void);
int __should_be_able_to_list_all_tokens_of_an_input(void);

int __should_be_able_to_tokenize_a_string(void);
int __should_error_out_when_tokenizing_an_unterminated_string(void);

int __should_be_able_to_tokenize_a_char(void);
int __should_error_out_when_tokenizing_an_unterminated_char(void);

int __should_be_able_to_tokenize_an_integer(void);

int __should_be_able_to_support_binary(void);
int __should_be_able_to_support_octal(void);
int __should_be_able_to_support_hexadecimal(void);

int __should_be_able_to_tokenize_a_float(void);

int __should_be_able_to_do_a_first_pass(void);
int __should_be_able_to_allocate_the_correct_size_for_the_output(void);
int __should_be_able_to_define_and_undefine_a_directive_in_the_context(void);

int main(void)
{
	#define casename "typex"
	START_CASE;

	int err = 0;
	err = err || __should_initialize_the_context_properly();
	err = err || __should_be_able_to_define_a_directive_in_the_context();
	err = err || __should_be_able_to_undefine_a_directive_in_the_context();
	err = err || __should_be_able_to_find_a_directive_in_the_context();
	err = err || __should_be_able_to_list_all_tokens_of_an_input();
	err = err || __should_be_able_to_tokenize_a_string();
	err = err || __should_error_out_when_tokenizing_an_unterminated_string();
	err = err || __should_be_able_to_tokenize_a_char();
	err = err || __should_error_out_when_tokenizing_an_unterminated_char();
	err = err || __should_be_able_to_tokenize_an_integer();
	err = err || __should_be_able_to_support_binary();
	err = err || __should_be_able_to_support_hexadecimal();
	err = err || __should_be_able_to_support_octal();
	err = err || __should_be_able_to_tokenize_a_float();
	err = err || __should_be_able_to_do_a_first_pass();
	err = err || __should_be_able_to_allocate_the_correct_size_for_the_output();
	err = err || __should_be_able_to_define_and_undefine_a_directive_in_the_context();

	if (!err)
	{
		SUCCESS;
	}

	#undef casename
	return err;
}

int __should_initialize_the_context_properly(void)
{
	#define casename "should_initialize_the_context_properly"
	START_CASE;

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");
	ASSERT_EQ(ctx.definitions_len, 0, "the definitions_len should be 0 by default");

	for (size_t i = 0; i < __TYPEX_MAX_DEFINITIONS; ++i)
	{
		typex_directive_define_t *d = &ctx.definitions[i];
		ASSERT_EQ(d->next, -1, "should start the linked list empty");
		ASSERT_EQ(d->prev, -1, "should start the linked list empty");
	}

	for (size_t i = 0; i < __TYPEX_TABLE_SIZE; ++i)
	{
		ASSERT_EQ(ctx.buckets[i], -1, "all the buckets should be empty by default");
	}

	SUCCESS;
	#undef casename
}

int __should_be_able_to_define_a_directive_in_the_context(void)
{
	#define casename "should_be_able_to_define_a_directive_in_the_context"
	START_CASE;

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "#define x 1");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");
	ASSERT_EQ(ctx.definitions_len, 0, "the definitions_len should be 0 by default");

	typex_directive_define_t d = {
		.name_begin = 8,
		.name_end   = 9,

		.replacement_begin = 10,
		.replacement_end   = 11,
	};

	err = typex_define_replacement(&ctx, &d);
	ASSERT_EQ(err, 0, "should define a replacement macro in the context");

	ASSERT_EQ(ctx.definitions_len, 1, "should define a replacement macro in the context");

	typex_directive_define_t d_new = ctx.definitions[0];
	ASSERT_EQ(d_new.name_begin, d.name_begin, "should add the definition to the list");
	ASSERT_EQ(d_new.name_end, d.name_end, "should add the definition to the list");
	ASSERT_EQ(d_new.replacement_begin, d.replacement_begin, "should add the definition to the list");
	ASSERT_EQ(d_new.replacement_end, d.replacement_end, "should add the definition to the list");

	int idx = -1;
	for (size_t i = 0; i < __TYPEX_TABLE_SIZE; ++i)
	{
		if (ctx.buckets[i] != -1)
		{
			idx = ctx.buckets[i];
			break;
		}
	}

	ASSERT_EQ(idx, 0, "should add the definition to the list");
	ASSERT_EQ(d_new.prev, -1, "the previous element of the head should be -1");
	ASSERT_EQ(d_new.next, -1, "the next element of the head should be -1");

	SUCCESS;
	#undef casename
}

int __should_be_able_to_undefine_a_directive_in_the_context(void)
{
    #define casename "should_be_able_to_undefine_a_directive_in_the_context"
    START_CASE;

    typex_context_t ctx;
    int err = typex_new_ctx(&ctx, "#define x 1\n#undef x");
    ASSERT_EQ(err, 0, "should initialize a context with no errors");

	typex_directive_define_t d = {
		.name_begin = 8,
		.name_end   = 9,

		.replacement_begin = 10,
		.replacement_end   = 11,
	};

	err = typex_define_replacement(&ctx, &d);
	ASSERT_EQ(err, 0, "should define a replacement macro in the context");

	ASSERT_EQ(ctx.definitions_len, 1, "should define a replacement macro in the context");

	typex_directive_undefine_t u = {
		.name_begin = 8,
		.name_end   = 9,
	};

	err = typex_undefine_replacement(&ctx, &u);
	ASSERT_EQ(err, 0, "should be able to undefine a replacement in the context");

	typex_directive_define_t d_new;
	err = typex_define_replacement_lookup(&ctx, 8, 9, &d_new);
	ASSERT_EQ(err, E_KEYNOTFOUND, "it cant find a directive if none is in the list");

    SUCCESS;
    #undef casename
}

int __should_be_able_to_find_a_directive_in_the_context(void)
{
	#define casename "should_be_able_to_find_a_directive_in_the_context"
	START_CASE;

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "#define x 1");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");
	ASSERT_EQ(ctx.definitions_len, 0, "the definitions_len should be 0 by default");

	typex_directive_define_t d_new;
	err = typex_define_replacement_lookup(&ctx, 0, 0, &d_new);
	ASSERT_EQ(err, E_KEYNOTFOUND, "it cant find a directive if none is in the list");

	typex_directive_define_t d = {
		.name_begin = 8,
		.name_end   = 9,

		.replacement_begin = 10,
		.replacement_end   = 11,
	};

	err = typex_define_replacement(&ctx, &d);
	ASSERT_EQ(err, 0, "should define a replacement macro in the context");
	ASSERT_EQ(ctx.definitions_len, 1, "should define a replacement macro in the context");

	err = typex_define_replacement_lookup(&ctx, 8, 9, &d_new);
	ASSERT_EQ(err, 0, "it can find a directive if one is in the list");

	err = typex_define_replacement_lookup(&ctx, 10, 11, &d_new);
	ASSERT_EQ(err, E_KEYNOTFOUND, "it has to fail to find something that is not in the map");

	SUCCESS;
	#undef casename
}

#define ASSERT_TOKENS \
do \
{ \
while (tok_idx < num_toks && (err = typex_lexer_next_token(&l, &t)) == 0) \
{ \
    ASSERT_EQ(tok_idx < num_toks, 1, "should not have more tokens than expected"); \
    ASSERT_EQ(err, 0, "should not fail to get the next token"); \
    typex_token_t *expected = &expected_tokens[tok_idx]; \
    ASSERT_EQ(expected->begin, t.begin , "token does not match"); \
    ASSERT_EQ(expected->end, t.end , "token does not match"); \
    ASSERT_EQ(expected->t, t.t , "token does not match"); \
    tok_idx++; \
} \
\
ASSERT_EQ(err, 0, "should not fail to get the next token"); \
ASSERT_EQ(tok_idx, num_toks, "should have all tokens"); \
} while (0); \


int __should_be_able_to_list_all_tokens_of_an_input(void)
{
    #define casename "should_be_able_to_list_all_tokens_of_an_input"
    START_CASE;

    const char *program = "#define a 1\nint main(void)\n{\nreturn a;\n}";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 1, .end = 7, .t = TYPEX_TOKEN_TYPE_MACRO },
        { .begin = 7, .end = 8, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 8, .end = 9, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 9, .end = 10, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 10, .end = 11, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 11, .end = 12, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 12, .end = 15, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 15, .end = 16, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 16, .end = 20, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 20, .end = 21, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 21, .end = 25, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 25, .end = 26, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 26, .end = 27, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 27, .end = 28, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 28, .end = 29, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 29, .end = 35, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 35, .end = 36, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 36, .end = 37, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 37, .end = 38, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 38, .end = 39, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 39, .end = 40, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS
    #undef casename
}

int __should_be_able_to_tokenize_a_string(void)
{
    #define casename "should_be_able_to_tokenize_a_string"
    START_CASE

    const char *program = "\"hello world\"";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 13, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS
    #undef casename
}

int __should_error_out_when_tokenizing_an_unterminated_string(void)
{
    #define casename "should_error_out_when_tokenizing_an_unterminated_string"
    START_CASE

    const char *program = "\"hello world";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t t;

    int err = typex_lexer_next_token(&l, &t);

    ASSERT_EQ(E_TYPEX_ERR_UNEXPECTED_EOF, err, "should fail parsing unterminated strings");

    SUCCESS
    #undef casename
}

int __should_be_able_to_tokenize_a_char(void)
{
    #define casename "should_be_able_to_tokenize_a_char"
    START_CASE;

    const char *program = "'a'";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 3, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_error_out_when_tokenizing_an_unterminated_char(void)
{
    #define casename "should_error_out_when_tokenizing_an_unterminated_char"
    START_CASE;

    const char *program = "'hello world";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t t;

    int err = typex_lexer_next_token(&l, &t);

    ASSERT_EQ(E_TYPEX_ERR_UNEXPECTED_EOF, err, "should fail parsing unterminated chars");

    SUCCESS;
    #undef casename
}

int __should_be_able_to_tokenize_an_integer(void)
{
    #define casename "should_be_able_to_tokenize_an_integer"
    START_CASE;

    const char *program = "1 20 42";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 1, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 1, .end = 2, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 2, .end = 4, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 4, .end = 5, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 5, .end = 7, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_be_able_to_tokenize_a_float(void)
{
    #define casename "should_be_able_to_tokenize_a_float"
    START_CASE;

    const char *program = "1.2 1.2e-4 1e4";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 3, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 3, .end = 4, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 4, .end = 10, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 10, .end = 11, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 11, .end = 14, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_be_able_to_support_binary(void)
{
    #define casename "should_be_able_to_support_binary"
    START_CASE;

    const char *program = "0b1010 0B1111";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 6, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 6, .end = 7, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 7, .end = 13, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_be_able_to_support_octal(void)
{
    #define casename "should_be_able_to_support_octal"
    START_CASE;

    const char *program = "0123 0777";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 4, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 4, .end = 5, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 5, .end = 9, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_be_able_to_support_hexadecimal(void)
{
    #define casename "should_be_able_to_support_hexadecimal"
    START_CASE;

    const char *program = "0x123 0XABC";
    size_t len = strlen(program);

    typex_lexer_t l = {.len=len, .pos=0, .stream=program};
    typex_token_t expected_tokens[] = {
        { .begin = 0, .end = 5, .t = TYPEX_TOKEN_TYPE_WORD },
        { .begin = 5, .end = 6, .t = TYPEX_TOKEN_TYPE_WHITESPACE },
        { .begin = 6, .end = 11, .t = TYPEX_TOKEN_TYPE_WORD },
    };

    typex_token_t t;
    size_t num_toks = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
    size_t tok_idx = 0;
    int err;

    ASSERT_TOKENS

    SUCCESS;
    #undef casename
}

int __should_be_able_to_do_a_first_pass(void)
{
    #define casename "should_be_able_to_do_a_first_pass"
    START_CASE;

    const char *program = "#define a 1\nint main(void)\n{\nreturn a;\n}";
    size_t len = strlen(program);

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");

    err = typex_first_pass(program, len, &ctx);
    ASSERT_EQ(err, 0, "should not fail to do a first pass on a valid program");

    typex_directive_define_t d_new;
	err = typex_define_replacement_lookup(&ctx, 8, 9, &d_new);

	ASSERT_EQ(err, 0, "it can find a directive if one is in the list");
	ASSERT_EQ(d_new.replacement_begin, 10, "it should have the correct replacement_begin");
	ASSERT_EQ(d_new.replacement_end, 11, "it should have the correct replacement_end");

    SUCCESS;
    #undef casename
}

int __should_be_able_to_allocate_the_correct_size_for_the_output(void)
{
    #define casename "should_be_able_to_allocate_the_correct_size_for_the_output"
    START_CASE;

    const char *program = "#define a 1\nint main(void)\n{\n\treturn a;\n}";
    size_t len = strlen(program);

    const char *expected_program = "int main(void)\n{\n\treturn 1;\n}";
    size_t expected_len = strlen(expected_program);

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");

	owned_str_t out;
    err = typex_preprocess(program, len, &out);
    ASSERT_EQ(err, 0, "should not fail to do a first pass on a valid program");
    ASSERT_EQ(expected_len, out.len, "the preprocessor should allocate the correct size");
    for (size_t i = 0; i < out.len; i++)
    {
        ASSERT_EQ(expected_program[i], out.buff[i], "the preprocessed program should match the expected program");
    }

    free(out.buff);

    SUCCESS;
    #undef casename
}

int __should_be_able_to_define_and_undefine_a_directive_in_the_context(void)
{
    #define casename "should_be_able_to_define_and_undefine_a_directive_in_the_context"
    START_CASE;

    const char *program = "#define a 1\n#undef a\nint main(void)\n{\n\treturn a;\n}";
    size_t len = strlen(program);

    const char *expected_program = "int main(void)\n{\n\treturn a;\n}";
    size_t expected_len = strlen(expected_program);

	typex_context_t ctx;
	int err = typex_new_ctx(&ctx, "");
	ASSERT_EQ(err, 0, "should initialize a context with no errors");

	owned_str_t out;
    err = typex_preprocess(program, len, &out);
    ASSERT_EQ(err, 0, "should not fail to do a first pass on a valid program");
    ASSERT_EQ(expected_len, out.len, "the preprocessor should allocate the correct size");
    for (size_t i = 0; i < out.len; i++)
    {
        ASSERT_EQ(expected_program[i], out.buff[i], "the preprocessed program should match the expected program");
    }

    free(out.buff);

    SUCCESS;
    #undef casename
}

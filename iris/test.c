#include <stdio.h>
#include <string.h>

#include "./lib/iris.h"

#define START_CASE(name) printf("[TEST " name "]: Running tests...\n");
#define SUCCESS(name) \
	do \
	{ \
		printf("[TEST " name "]: All tests passes!!\n"); \
		return 0; \
	} while(0); 

#define FAIL(name, message) \
	do \
	{ \
		fprintf(stderr, "[FAIL " name "]: " message "\n");	\
		return 1; \
	} while (0);

#define ASSERT_EQ(v, err, name, message) \
	do \
	{ \
		if (v != err) FAIL(name, message); \
	} while (0);

#define ASSERT_TOKEN_EQ(name, s, e, l, c, type, tok) \
	do \
	{ \
		token_t t1; \
		t1.start = s; \
		t1.end = e; \
		t1.line = l; \
		t1.col = c; \
		t1.t = type; \
		if (__tok_equal(name, &t1, &tok)) \
		{ \
			FAIL(name, "tokens didn't match"); \
		} \
	} while (0);

static char __tok_equal(const char *, token_t *e, token_t *g);

static int __should_init_a_lexer(void);
static int __should_be_able_to_lex_programs(void);
static int __should_not_be_eager_to_find_tokens(void);

int main(void)
{
	START_CASE("lexer");

	int err = 0;
	err = err || __should_init_a_lexer();
	err = err || __should_be_able_to_lex_programs();
	err = err || __should_not_be_eager_to_find_tokens();
	
	if (!err)
	{
		SUCCESS("lexer");
	}

	return 0;
}

static int __should_init_a_lexer(void)
{
	START_CASE("should_init_a_lexer");

	int err;
	err = lexer_init(NULL, "a", 1);
	ASSERT_EQ(E_INVALIDPARAM, err, "should_init_a_lexer", "should error out if initializing with empty lexer");

	lexer_t l = {0};
	err = lexer_init(&l, NULL, 1);
	ASSERT_EQ(E_INVALIDPARAM, err, "should_init_a_lexer", "should error out if initializing with empty string");

	const char *input = "int main(void) {}";
	size_t len = strlen(input);

	err = lexer_init(&l, input, len);
	ASSERT_EQ(0, err, "should_init_a_lexer", "should succeed to initialize a valid input");

	ASSERT_EQ(0, l.line, "should_init_a_lexer", "should initialize the line as 0");
	ASSERT_EQ(1, l.col, "should_init_a_lexer", "should initialize the column at 1");
	ASSERT_EQ(0, l.pos, "should_init_a_lexer", "should initialize the position at 0");
	
	ASSERT_EQ(input, l.data, "should_init_a_lexer", "should initialize the string pointer to the input");
	ASSERT_EQ(len, l.data_len, "should_init_a_lexer", "should initialize the string length to the equal the input");
	
	SUCCESS("should_init_a_lexer");
}

static int __should_be_able_to_lex_programs(void)
{
	START_CASE("should_be_able_to_lex_programs");

	lexer_t lexer;

	int err;	
	err = lexer_init(&lexer, "", 0);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to initialize a valid input");

	token_t t;
	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should be able to lex an empty string");

	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 0, 0, 0, 1, TOKEN_TYPE_EOF, t);

	const char *sample_program = "int main(void) {}";
	size_t sample_program_len = strlen(sample_program);

	err = lexer_init(&lexer, sample_program, sample_program_len);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to initialize a valid input");

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 0, 3, 0, 1, TOKEN_TYPE_INT, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 4, 8, 0, 4, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 8, 8, 0, 9, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 9, 13, 0, 10, TOKEN_TYPE_VOID, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 13, 13, 0, 14, TOKEN_TYPE_RIGHT_PAREN, t);


	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 14, 15, 0, 16, TOKEN_TYPE_LEFT_BRACE, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_be_able_to_lex_programs", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_be_able_to_lex_programs", 16, 16, 0, 17, TOKEN_TYPE_RIGHT_BRACE, t);

	SUCCESS("should_be_able_to_lex_programs");
}

static int __should_not_be_eager_to_find_tokens(void)
{
	START_CASE("should_not_be_eager_to_find_tokens");

	int err;	
	const char *sample_program = "float fn(double x) {\ndouble doubled = x*2;\n return doubled; \n}";
	size_t sample_program_len = strlen(sample_program);

	lexer_t lexer;
	token_t t;
	err = lexer_init(&lexer, sample_program, sample_program_len);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to initialize a valid input");

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 0, 5, 0, 1, TOKEN_TYPE_FLOAT, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 6, 8, 0, 6, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 8, 8, 0, 9, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 9, 15, 0, 10, TOKEN_TYPE_DOUBLE, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 16, 17, 0, 16, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 17, 17, 0, 18, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 18, 19, 0, 20, TOKEN_TYPE_LEFT_BRACE, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 21, 27, 1, 21, TOKEN_TYPE_DOUBLE, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 28, 35, 1, 7, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 35, 36, 1, 16, TOKEN_TYPE_EQUALS, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 38, 39, 1, 17, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 39, 39, 1, 19, TOKEN_TYPE_STAR, t);

	err = lexer_next_token(&lexer, &t);
	ASSERT_EQ(0, err, "should_not_be_eager_to_find_tokens", "should succeed to lex a valid program");
	ASSERT_TOKEN_EQ("should_not_be_eager_to_find_tokens", 40, 41, 1, 20, TOKEN_TYPE_NUMERIC_LITERAL, t);

	SUCCESS("should_not_be_eager_to_find_tokens(");
}

static char __tok_equal(const char *suitename, token_t *e, token_t *g)
{
	if (e == NULL && g == NULL) return 1;

	if (e == NULL && g != NULL) return 0;
	if (e != NULL && g == NULL) return 0;

	if (e->t != g->t)
	{
		fprintf(stderr, "[FAIL %s]: expected type to be: %d but got: %d\n", suitename, e->t, g->t); 
		return 1;
	}

	if (e->start != g->start)
	{
		fprintf(stderr, "[FAIL %s]: expected start to be: %zu but got: %zu\n", suitename, e->start, g->start); 
		return 1;
	}

	if (e->end != g->end)
	{
		fprintf(stderr, "[FAIL %s]: expected end to be: %zu but got: %zu\n", suitename, e->end, g->end); 
		return 1;
	}

	if (e->line != g->line)
	{
		fprintf(stderr, "[FAIL %s]: expected line to be: %zu but got: %zu\n", suitename, e->line, g->line); 
		return 1;
	}

	if (e->col != g->col)
	{
		fprintf(stderr, "[FAIL %s]: expected column to be: %zu but got: %zu\n", suitename, e->col, g->col); 
		return 1;
	}

	return 0;
}


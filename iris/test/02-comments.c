#include <stdio.h>
#include <stdlib.h>

#include "lib.h"
#include "../lib/iris.h"

int main(void)
{
	START_CASE("02-comments.c")

	io_string_t str = {0};

	int err = file_into_memory("./test/examples/02-comments.c", &str);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to load file into memory");	

	lexer_t l = {0};
	err = lexer_init(&l, str.buff, str.len);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to initialize lexer into memory");	

	token_t t = {0};
	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 149, 152, 5, 1, TOKEN_TYPE_INT, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 153, 157, 5, 4, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 157, 157, 5, 9, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 158, 162, 5, 10, TOKEN_TYPE_VOID, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 162, 162, 5, 14, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 163, 164, 5, 16, TOKEN_TYPE_LEFT_BRACE, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 166, 172, 5, 17, TOKEN_TYPE_RETURN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 173, 174, 5, 24, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 174, 174, 5, 26, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "02-comments.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("02-comments.c", 175, 176, 5, 28, TOKEN_TYPE_RIGHT_BRACE, t);

	free(str.buff);

	SUCCESS("02-comments.c")
	return 0;
}


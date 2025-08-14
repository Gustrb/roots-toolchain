#include <stdio.h>

#include "lib.h"
#include "../lib/iris.h"

int main(void)
{
	START_CASE("01-helloworld.c")

	io_string_t str = {0};

	int err = file_into_memory("./test/examples/01-helloworld.c", &str);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to load file into memory");	

	lexer_t l = {0};
	err = lexer_init(&l, str.buff, str.len);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to initialize lexer into memory");	

	token_t t = {0};

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 20, 23, 2, 1, TOKEN_TYPE_INT, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 24, 28, 2, 4, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 28, 28, 2, 9, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 29, 33, 2, 10, TOKEN_TYPE_VOID, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 33, 33, 2, 14, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 34, 35, 3, 1, TOKEN_TYPE_LEFT_BRACE, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 38, 44, 4, 2, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 44, 44, 4, 11, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 45, 62, 4, 12, TOKEN_TYPE_STRING_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 62, 62, 4, 29, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 63, 63, 4, 30, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 66, 72, 5, 31, TOKEN_TYPE_RETURN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 73, 74, 5, 11, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 74, 74, 5, 13, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "01-helloworld.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("01-helloworld.c", 75, 76, 6, 1, TOKEN_TYPE_RIGHT_BRACE, t);

	SUCCESS("01-helloworld.c")
}


#include <stdio.h>

#include "lib.h"
#include "../lib/iris.h"

int main(void)
{
	START_CASE("00-fib.c")

	io_string_t str = {0};

	int err = file_into_memory("./test/examples/00-fib.c", &str);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to load file into memory");	

	lexer_t l = {0};
	err = lexer_init(&l, str.buff, str.len);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to initialize lexer into memory");	

	token_t t = {0};
	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 0, 3, 0, 1, TOKEN_TYPE_INT, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 4, 7, 0, 4, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 7, 7, 0, 8, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 8, 11, 0, 9, TOKEN_TYPE_INT, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 12, 13, 0, 12, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 13, 13, 0, 14, TOKEN_TYPE_RIGHT_PAREN, t);
	
	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 14, 15, 1, 1, TOKEN_TYPE_LEFT_BRACE, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 18, 20, 2, 2, TOKEN_TYPE_IF, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 20, 21, 2, 8, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 22, 23, 2, 9, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 23, 24, 2, 11, TOKEN_TYPE_EQUALS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 25, 25, 2, 12, TOKEN_TYPE_EQUALS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 27, 28, 2, 13, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 28, 28, 2, 15, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 30, 36, 2, 16, TOKEN_TYPE_RETURN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 37, 38, 2, 23, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 38, 38, 2, 25, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 41, 43, 3, 26, TOKEN_TYPE_IF, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 43, 44, 3, 8, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 45, 46, 3, 9, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 46, 47, 3, 11, TOKEN_TYPE_EQUALS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 48, 48, 3, 12, TOKEN_TYPE_EQUALS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 50, 51, 3, 13, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 51, 51, 3, 15, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 53, 59, 3, 16, TOKEN_TYPE_RETURN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 60, 61, 3, 23, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 61, 61, 3, 25, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 64, 70, 4, 26, TOKEN_TYPE_RETURN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 71, 74, 4, 11, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 74, 74, 4, 15, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 75, 76, 4, 16, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 76, 76, 4, 17, TOKEN_TYPE_MINUS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 77, 78, 4, 18, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 78, 78, 4, 19, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 79, 79, 4, 20, TOKEN_TYPE_PLUS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 80, 83, 4, 21, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 83, 83, 4, 24, TOKEN_TYPE_LEFT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 84, 85, 4, 25, TOKEN_TYPE_IDENTIFIER, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 85, 85, 4, 26, TOKEN_TYPE_MINUS, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 86, 87, 4, 27, TOKEN_TYPE_NUMERIC_LITERAL, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 87, 87, 4, 28, TOKEN_TYPE_RIGHT_PAREN, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 88, 88, 4, 29, TOKEN_TYPE_SEMICOLON, t);

	err = lexer_next_token(&l, &t);
	ASSERT_EQ(0, err, "00-fib.c", "should not fail to lex valid program");	
	ASSERT_TOKEN_EQ("00-fib.c", 89, 91, 5, 1, TOKEN_TYPE_RIGHT_BRACE, t);

	free(str.buff);
	SUCCESS("00-fib.c")
	return 0;
}


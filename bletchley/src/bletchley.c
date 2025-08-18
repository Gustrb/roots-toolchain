#include <stdio.h>

#include "../../iris/lib/iris.h"

#define E_INVALIDUSAGE 999

#define E_FAILTOOPENFILE 1
#define E_FAILTOSEEKFILE 2
#define E_FAILTOTELLFILESIZE 3
#define E_FAILTOREWINDFILE 4
#define E_OUTOFMEM 5
#define E_FAILTOREAD 6

typedef struct
{
	char *buff;
	size_t len;
} io_string_t;

void __usage(char **argv);
int __file_into_memory(const char *filepath, io_string_t *iostr);
void __print_token(token_t *t);

#define ITEM(x) [x] = #x

static const char *__tok_type_table[] = {
	ITEM(TOKEN_TYPE_COMMA), ITEM(TOKEN_TYPE_DOT),

	ITEM(TOKEN_TYPE_PLUS), ITEM(TOKEN_TYPE_MINUS),
	ITEM(TOKEN_TYPE_STAR), ITEM(TOKEN_TYPE_SLASH),
	ITEM(TOKEN_TYPE_GREATER), ITEM(TOKEN_TYPE_SMALLER),
	ITEM(TOKEN_TYPE_BANG), ITEM(TOKEN_TYPE_EQUALS),
	ITEM(TOKEN_TYPE_AMPERSAND), ITEM(TOKEN_TYPE_PIPE),
	ITEM(TOKEN_TYPE_XOR), ITEM(TOKEN_TYPE_TILDE),

	ITEM(TOKEN_TYPE_AUTO), ITEM(TOKEN_TYPE_BREAK),
	ITEM(TOKEN_TYPE_CASE), ITEM(TOKEN_TYPE_CHAR),
	ITEM(TOKEN_TYPE_CONST), ITEM(TOKEN_TYPE_CONTINUE),
	ITEM(TOKEN_TYPE_DEFAULT), ITEM(TOKEN_TYPE_DO),
	ITEM(TOKEN_TYPE_DOUBLE), ITEM(TOKEN_TYPE_ELSE),
	ITEM(TOKEN_TYPE_ENUM), ITEM(TOKEN_TYPE_EXTERN),
	ITEM(TOKEN_TYPE_FLOAT), ITEM(TOKEN_TYPE_FOR),
	ITEM(TOKEN_TYPE_GOTO), ITEM(TOKEN_TYPE_IF),
	ITEM(TOKEN_TYPE_INT), ITEM(TOKEN_TYPE_LONG),
	ITEM(TOKEN_TYPE_REGISTER), ITEM(TOKEN_TYPE_RETURN),
	ITEM(TOKEN_TYPE_SHORT), ITEM(TOKEN_TYPE_SIGNED),
	ITEM(TOKEN_TYPE_SIZEOF), ITEM(TOKEN_TYPE_STATIC),
	ITEM(TOKEN_TYPE_STRUCT), ITEM(TOKEN_TYPE_SWITCH),
	ITEM(TOKEN_TYPE_TYPEDEF), ITEM(TOKEN_TYPE_UNION),
	ITEM(TOKEN_TYPE_UNSIGNED), ITEM(TOKEN_TYPE_VOID),
	ITEM(TOKEN_TYPE_VOLATILE), ITEM(TOKEN_TYPE_WHILE),
	
	ITEM(TOKEN_TYPE_LEFT_PAREN), ITEM(TOKEN_TYPE_RIGHT_PAREN),
	ITEM(TOKEN_TYPE_LEFT_BRACE), ITEM(TOKEN_TYPE_RIGHT_BRACE),
	ITEM(TOKEN_TYPE_LEFT_BRACKET), ITEM(TOKEN_TYPE_RIGHT_BRACKET),
	ITEM(TOKEN_TYPE_SEMICOLON), ITEM(TOKEN_TYPE_COLON),

	ITEM(TOKEN_TYPE_IDENTIFIER), ITEM(TOKEN_TYPE_EOF),

	ITEM(TOKEN_TYPE_STRING_LITERAL), ITEM(TOKEN_TYPE_CHAR_LITERAL),
	ITEM(TOKEN_TYPE_NUMERIC_LITERAL), ITEM(TOKEN_TYPE_FLOAT_LITERAL),
};

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		__usage(argv);
		return E_INVALIDUSAGE;
	}

	// TODO: Support format args
	char *filepath = argv[1];

	io_string_t str = {0};
	int err = __file_into_memory(filepath, &str);
	if (err)
	{
		switch (err)
		{
			case E_FAILTOOPENFILE:
			{
				fprintf(stderr, "[ERROR]: Failed to open file, is it at: `%s`?\n", filepath);
			}; break;
			case E_FAILTOSEEKFILE:
			{
				fprintf(stderr, "[ERROR]: Failed to seek file, unix does not like you :(\n");
			}; break;
			case E_FAILTOTELLFILESIZE:
			{
				fprintf(stderr, "[ERROR]: Failed to tell file size, unix does not like you :(\n");
			}; break;
			case E_OUTOFMEM:
			{
				fprintf(stderr, "[ERROR]: Failed to load the file into memory, buy a new computer :(\n");
			}; break;
			case E_FAILTOREAD:
			{
				fprintf(stderr, "[ERROR]: Failed to read the file into memory, fuck!\n");
			}; break;
		}
		return err;
	}

	lexer_t l;
	err = lexer_init(&l, str.buff, str.len);
	if (err)
	{
		fprintf(stderr, "[ERROR]: For some reason I did something really dumb in the software, contact me! My bad\n");
		return err;
	}

	token_t t;
	while (t.t != TOKEN_TYPE_EOF)
	{
		err = lexer_next_token(&l, &t);
		if (err)
		{
			// TODO: Actually handle this
			fprintf(stderr, "[ERROR]: Failed to lex next token, error code: %d\n", err);
			break;
		}

		// TODO: Display the token in screen
		__print_token(&t);
	}

	free(str.buff);
	return 0;
}

int __file_into_memory(const char *filepath, io_string_t *iostr)
{
	FILE *f = fopen(filepath, "r");
	if (!f)
	{
		return E_FAILTOOPENFILE;
	}

	if (fseek(f, 0, SEEK_END) == -1)
	{
		fclose(f);
		return E_FAILTOSEEKFILE;
	}
	
	long fsize = ftell(f);
	if (fsize == -1)
	{
		fclose(f);
		return E_FAILTOTELLFILESIZE;
	}

	rewind(f);

	iostr->buff = malloc(sizeof(char) * fsize);
	if (iostr->buff == NULL)
	{
		fclose(f);
		return E_OUTOFMEM;
	}
	iostr->len = (size_t)fsize;

	if (fread(iostr->buff, iostr->len, sizeof(char), f) == 0)
	{
		free(iostr->buff);
		fclose(f);
		return E_FAILTOREAD;
	}

	fclose(f);

	return 0;
}

void __usage(char **argv)
{
	char *execname = argv[0];
	fprintf(stdout, "Invalid usage!!! Usage: `%s <filename>`\n", execname);
}

void __print_token(token_t *t)
{

	fprintf(stdout, "{");
	fprintf(stdout, "type: '%s', start: '%zu', end: '%zu'", __tok_type_table[t->t], t->start, t->end);
	fprintf(stdout, "}\n");
}


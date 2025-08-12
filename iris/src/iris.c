#include <string.h>

#include "../lib/iris.h"
#include "../lib/log.h"

#define IS_ALPHABETIC(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')
#define IS_NUMERIC(c) (c) >= '0' && (c) <= '9'
#define IS_ALPHANUMERIC(c) IS_ALPHABETIC(c) || IS_NUMERIC(c)

#define __LEXER_NKEYWORDS 32

static char __lexer_advance(lexer_t *, char);
static token_type_t __lexer_figure_out_if_it_is_keyword_or_identifier(const char *, token_t *);
static unsigned long long __hash(const char *, size_t, size_t);

typedef struct 
{
	token_type_t t;
	unsigned long long v;
} keyword_pair_t;

// sorted by value (hash)
static const keyword_pair_t __keywords[__LEXER_NKEYWORDS] = {
	{ .t = TOKEN_TYPE_EXTERN,       .v = 417800907370030071ull   },
	{ .t = TOKEN_TYPE_DO,           .v = 617372513566700692ull   },
	{ .t = TOKEN_TYPE_IF,           .v = 628023482707099174ull   },
	{ .t = TOKEN_TYPE_REGISTER,     .v = 1390995973168426656ull  },
	{ .t = TOKEN_TYPE_INT,          .v = 3143511548502526014ull  },
	{ .t = TOKEN_TYPE_VOID,         .v = 3563412735833858527ull  },
	{ .t = TOKEN_TYPE_UNION,        .v = 5416238596490301492ull  },
	{ .t = TOKEN_TYPE_SIGNED,       .v = 5788750116080139093ull  },
	{ .t = TOKEN_TYPE_AUTO,         .v = 6757501175350325814ull  },
	{ .t = TOKEN_TYPE_CONST,        .v = 7334518323283222324ull  },
	{ .t = TOKEN_TYPE_ELSE,         .v = 9163537027783908656ull  },
	{ .t = TOKEN_TYPE_SIZEOF,       .v = 9964885110000857501ull  },
	{ .t = TOKEN_TYPE_ENUM,         .v = 10474992716130377088ull },
	{ .t = TOKEN_TYPE_BREAK,        .v = 10644074229358120504ull },
	{ .t = TOKEN_TYPE_VOLATILE,     .v = 10769778583944859405ull },
	{ .t = TOKEN_TYPE_GOTO,         .v = 11331286950270920518ull },
	{ .t = TOKEN_TYPE_FLOAT,        .v = 11532138274943533413ull },
	{ .t = TOKEN_TYPE_DOUBLE,       .v = 11567507311810436776ull },
	{ .t = TOKEN_TYPE_SWITCH,       .v = 11936925801837197745ull },
	{ .t = TOKEN_TYPE_CASE,         .v = 13068990443679199185ull },
	{ .t = TOKEN_TYPE_UNSIGNED,     .v = 13236544317732455142ull },
	{ .t = TOKEN_TYPE_STRUCT,       .v = 14058172984575203104ull },
	{ .t = TOKEN_TYPE_STATIC,       .v = 14210125031242066299ull },
	{ .t = TOKEN_TYPE_RETURN,       .v = 14251563519059995999ull },
	{ .t = TOKEN_TYPE_LONG,         .v = 14837330719131395891ull },
	{ .t = TOKEN_TYPE_WHILE,        .v = 14882043299657492846ull },
	{ .t = TOKEN_TYPE_CONTINUE,     .v = 15186091406668687012ull },
	{ .t = TOKEN_TYPE_FOR,          .v = 15902905282948881040ull },
	{ .t = TOKEN_TYPE_TYPEDEF,      .v = 16509985740318510052ull },
	{ .t = TOKEN_TYPE_DEFAULT,      .v = 16982411286042166782ull },
	{ .t = TOKEN_TYPE_CHAR,         .v = 17483980429552467645ull },
	{ .t = TOKEN_TYPE_SHORT,        .v = 17767075776831802709ull },
};

static const char *__keywords_str[__LEXER_NKEYWORDS] = {
	"extern", "do", "if", "register", "int", "void",
	"union", "signed", "auto", "const", "else", "sizeof",
	"enum", "break", "volatile", "goto", "float",
	"double", "switch", "case", "unsigned", "struct",
	"static", "return", "long", "while", "continue",
	"for", "typedef", "default", "char", "sort",
};

// Here we don't really care if str is null-terminated or not.
// we read from 0..len (non-inclusive) 
int lexer_init(lexer_t *lexer, const char *str, size_t len)
{
	if (lexer == NULL) return E_INVALIDPARAM;
	if (str == NULL)   return E_INVALIDPARAM;

	lexer->data     = str;
	lexer->data_len = len;

	lexer->line = 0;
	lexer->pos  = 0;
	lexer->col  = 1;

	return 0;
}

typedef enum
{
	STATE_INITIAL,
	STATE_STRING_IDENTIFIER,
	STATE_NUMBER_IDENTIFIER,
	STATE_STRING_LITERAL,
	STATE_CHAR_LITERAL,
} state_t;

int lexer_next_token(lexer_t *l, token_t *t)
{
	if (l == NULL) return E_INVALIDPARAM;
	if (t == NULL) return E_INVALIDPARAM;

	state_t state = STATE_INITIAL;


	size_t start = l->pos;
	size_t colstart = l->col;

	while (l->pos < l->data_len)
	{
		char curr = l->data[l->pos];
		switch (state)
		{
			case STATE_INITIAL:
			{
				switch (curr)
				{
					case '(':
					{
						t->t = TOKEN_TYPE_LEFT_PAREN;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;
					case ')':
					{
						t->t = TOKEN_TYPE_RIGHT_PAREN;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;
					case '{':
					{
						t->t = TOKEN_TYPE_LEFT_BRACE;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);
						return 0;
					}; break;
					case '}':
					{
						t->t = TOKEN_TYPE_RIGHT_BRACE;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;

					case '=':
					{
						t->t = TOKEN_TYPE_EQUALS;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;

					case '*':
					{
						t->t = TOKEN_TYPE_STAR;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;

					case ';':
					{
						t->t = TOKEN_TYPE_SEMICOLON;

						t->line = l->line;
						t->col = l->col;
						t->start = start;
						t->end = l->pos;

						__lexer_advance(l, curr);

						return 0;
					}; break;
					
					case '"':
					{
						state = STATE_STRING_LITERAL;
						start = l->pos;
						__lexer_advance(l, curr);
					}; break;

					case '\'':
					{
						state = STATE_CHAR_LITERAL;
						start = l->pos;
						__lexer_advance(l, curr);
					}; break;

					case ' ': case '\n': case '\t':
					{

						__lexer_advance(l, curr);
					}; break;

					default:
					{
						if (IS_ALPHABETIC(curr))
						{
							state = STATE_STRING_IDENTIFIER;
							start = l->pos;
							__lexer_advance(l, curr);
						}
						else if (IS_NUMERIC(curr))
						{
							state = STATE_NUMBER_IDENTIFIER;
							start = l->pos;
							__lexer_advance(l, curr);
						}
						else
						{
							l->pos++;
							return E_INVALIDTOKEN;
						}
					}; break;
				}
			}; break;
			
			case STATE_STRING_LITERAL:
			{
				t->line = l->line;
				t->col = colstart;
				t->start = start;

				while (curr != '"' && l->pos < l->data_len)
				{
					curr = __lexer_advance(l, curr);
					if (curr == '\n')
					{
						curr = __lexer_advance(l, curr);
						return E_UNTERMINATEDSTRINGLITERAL;
					}
				}

				if (l->pos == l->data_len)
				{
					return E_UNTERMINATEDSTRINGLITERAL;
				}

				curr = __lexer_advance(l, curr);

				t->end = l->pos;
				t->t = TOKEN_TYPE_STRING_LITERAL;
				return 0;
			}; break;

			case STATE_CHAR_LITERAL:
			{
				t->line = l->line;
				t->col = colstart;
				t->start = start;

				if (curr == '\n' || l->pos >= l->data_len) return E_UNTERMINATEDCHARLITERAL;

				curr = __lexer_advance(l, curr);

				if (curr != '\'') return E_CHARLITERALTOOBIG;

				curr = __lexer_advance(l, curr);

				t->end = l->pos;
				t->t = TOKEN_TYPE_CHAR_LITERAL;
				return 0;
			}; break;

			case STATE_STRING_IDENTIFIER:
			{
				t->line = l->line;
				t->col = colstart;
				t->start = start;

				while (IS_ALPHANUMERIC(curr) && l->pos < l->data_len)
				{
					curr = __lexer_advance(l, curr);
				}

				t->end = l->pos;

				t->t = __lexer_figure_out_if_it_is_keyword_or_identifier(l->data, t);
				return 0;
			}; break;

			case STATE_NUMBER_IDENTIFIER:
			{
				t->line = l->line;
				t->col = colstart;
				t->start = start;

				while (IS_NUMERIC(curr) && l->pos < l->data_len)
				{
					curr = __lexer_advance(l, curr);
				}

				t->end = l->pos;

				t->t = TOKEN_TYPE_NUMERIC_LITERAL;
				return 0;
			}; break;

			default:
			{
				return E_INVALIDSTATE;
			}; break;
		}
	}

	t->t = TOKEN_TYPE_EOF;
	t->line = l->line;
	t->col = l->col;
	t->start = l->pos;
	t->end = t->start;

	return 0;
}

static char __lexer_advance(lexer_t *l, char c)
{
	char o = l->data[++l->pos];
	if (c == '\n')
	{
		l->col = 1;
		l->line++;
		return o;
	}
	if (c == '\t')
	{
		l->col +=4;
		return o;
	}

	l->col++;
	return o;
}

// I guess SIMD would be super cool for keyword matching (?)
static token_type_t __lexer_figure_out_if_it_is_keyword_or_identifier(const char *stream, token_t *t)
{
	unsigned long long h = __hash(stream, t->start, t->end);

	size_t low = 0;
	size_t high = __LEXER_NKEYWORDS-1;

	while (low <= high)
	{
		size_t i = (low+high)/2;
		if (__keywords[i].v == h)
		{
			size_t l = t->end-t->start;
			if (l == strlen(__keywords_str[i]))
			{
				return __keywords[i].t;
			}
			return TOKEN_TYPE_IDENTIFIER;
		}
		else if (__keywords[i].v < h)
		{
			low = i+1;
		}
		else
		{
			if (i == 0) break;
			high = i-1;
		}
	}

	return TOKEN_TYPE_IDENTIFIER;
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


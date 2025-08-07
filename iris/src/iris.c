#include "../lib/iris.h"

#define IS_ALPHABETIC(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')

void __lexer_advance(lexer_t *, char);

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
} state_t;

int lexer_next_token(lexer_t *l, token_t *t)
{
	if (l == NULL) return E_INVALIDPARAM;
	if (t == NULL) return E_INVALIDPARAM;

	state_t state = STATE_INITIAL;

	while (l->pos < l->data_len)
	{
		char curr = l->data[l->pos];
		size_t start = l->pos;

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

					case ' ': case '\n': case '\t':
					{

						__lexer_advance(l, curr);
					}; break;

					default:
					{
						if (IS_ALPHABETIC(curr))
						{
							state = STATE_STRING_IDENTIFIER;
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

			case STATE_STRING_IDENTIFIER:
			{
				// TODO: Implement this shit
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

void __lexer_advance(lexer_t *l, char c)
{
	l->pos++;
	if (c == '\n')
	{
		l->col = 1;
		l->line++;
		return;
	}
	if (c == '\t')
	{
		l->col +=4;
		return;
	}

	l->col++;
}


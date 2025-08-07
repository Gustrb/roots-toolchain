#include "../lib/iris.h"

#define IS_ALPHABETIC(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')
#define IS_NUMERIC(c) (c) >= '0' && (c) <= '9'
#define IS_ALPHANUMERIC(c) IS_ALPHABETIC(c) || IS_NUMERIC(c)

static char __lexer_advance(lexer_t *, char);
static token_type_t __lexer_figure_out_if_it_is_keyword_or_identifier(const char *, token_t *);

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

typedef enum
{
	KL_STATE_INITIAL,	

	KL_STATE_I,
	KL_STATE_IN,

	KL_STATE_V,
	KL_STATE_VO,
	KL_STATE_VOI,

	KL_STATE_F,
	KL_STATE_FL,
	KL_STATE_FLO,
	KL_STATE_FLOA,

	KL_STATE_D,
	KL_STATE_DO,
	KL_STATE_DOU,
	KL_STATE_DOUB,
	KL_STATE_DOUBL,

	KL_STATE_R,
	KL_STATE_RE,
	KL_STATE_RET,
	KL_STATE_RETU,
	KL_STATE_RETUR,
} keyword_lex_state_t;

#include <stdio.h>

static token_type_t __lexer_figure_out_if_it_is_keyword_or_identifier(const char *stream, token_t *t)
{
	size_t pos = t->start;

	keyword_lex_state_t state = KL_STATE_INITIAL; 

	while (pos < t->end)
	{
		char c = stream[pos];

		switch (state)
		{
			case KL_STATE_INITIAL:
			{
				switch (c)
				{
					case 'i':
					{
						state = KL_STATE_I;
						pos++;
					}; break;

					case 'v':
					{
						state = KL_STATE_V;
						pos++;
					}; break;
					
					case 'f':
					{
						state = KL_STATE_F;
						pos++;
					}; break;

					case 'd':
					{
						state = KL_STATE_D;
						pos++;
					}; break;
					
					case 'r':
					{
						state = KL_STATE_R;
						pos++;
					}; break;
					
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					}; break;

				}
			}; break;

			case KL_STATE_I:
			{
				switch (c)
				{
					case 'n':
					{
						state = KL_STATE_IN;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_IN:
			{
				switch (c)
				{
					case 't':
					{
						if (pos == t->end-1)
							return TOKEN_TYPE_INT;
						state = KL_STATE_INITIAL;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_V:
			{
				switch (c)
				{
					case 'o':
					{
						state = KL_STATE_VO;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_VO:
			{
				switch (c)
				{
					case 'i':
					{
						state = KL_STATE_VOI;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_VOI:
			{
				switch (c)
				{
					case 'd':
					{
						if (pos == t->end-1)
							return TOKEN_TYPE_VOID;
						state = KL_STATE_INITIAL;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_F:
			{
				switch (c)
				{
					case 'l':
					{
						state = KL_STATE_FL;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_FL:
			{
				switch (c)
				{
					case 'o':
					{
						state = KL_STATE_FLO;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_FLO:
			{
				switch (c)
				{
					case 'a':
					{
						state = KL_STATE_FLOA;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_FLOA:
			{
				switch (c)
				{
					case 't':
					{
						if (pos == t->end-1)
							return TOKEN_TYPE_FLOAT;
						state = KL_STATE_INITIAL;
						pos++;
					}; break;
					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_D:
			{
				switch (c)
				{
					case 'o':
					{
						state = KL_STATE_DO;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};

				}
			}; break;

			case KL_STATE_DO:
			{
				switch (c)
				{
					case 'u':
					{
						state = KL_STATE_DOU;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};

				}
			}; break;

			case KL_STATE_DOU:
			{
				switch (c)
				{
					case 'b':
					{
						state = KL_STATE_DOUB;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_DOUB:
			{
				switch (c)
				{
					case 'l':
					{
						state = KL_STATE_DOUBL;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};

				}
			}; break;

			case KL_STATE_DOUBL:
			{
				switch (c)
				{
					case 'e':
					{
						if (pos == t->end-1)
							return TOKEN_TYPE_DOUBLE;
						state = KL_STATE_INITIAL;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};

				}
			}; break;

			case KL_STATE_R:
			{
				switch (c)
				{
					case 'e':
					{
						state = KL_STATE_RE;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_RE:
			{
				switch (c)
				{
					case 't':
					{
						state = KL_STATE_RET;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_RET:
			{
				switch (c)
				{
					case 'u':
					{
						state = KL_STATE_RETU;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_RETU:
			{
				switch (c)
				{
					case 'r':
					{
						state = KL_STATE_RETUR;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;

			case KL_STATE_RETUR:
			{
				switch (c)
				{
					case 'n':
					{
						if (pos == t->end-1)
							return TOKEN_TYPE_RETURN;
						state = KL_STATE_INITIAL;
						pos++;
					}; break;

					default:
					{
						return TOKEN_TYPE_IDENTIFIER;
					};
				}
			}; break;











			default:
			{
				// Unreachable, I guess
				return TOKEN_TYPE_IDENTIFIER;
			}; break;
		}
	}

	return TOKEN_TYPE_IDENTIFIER;
}


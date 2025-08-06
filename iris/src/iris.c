#include "../lib/iris.h"

// Here we don't really care if str is null-terminated or not, we assume the size does not point
// to a null-byte and that is it.
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

int lexer_next_token(lexer_t *l, token_t *t)
{
	if (l == NULL) return E_INVALIDPARAM;
	if (t == NULL) return E_INVALIDPARAM;

	if (l->pos >= l->data_len)
	{
		t->t = TOKEN_TYPE_EOF;
		t->line = l->line;
		t->col = l->col;
		t->start = l->pos;
		t->end = t->start;

		return 0;
	}

	return 0;
}


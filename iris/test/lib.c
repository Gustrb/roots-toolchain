#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

char tok_equal(const char *suitename, token_t *e, token_t *g)
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


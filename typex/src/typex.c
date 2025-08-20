#include "../lib/typex.h"

#include <string.h>

#define ISBLANK(c) ( (c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r' )

#define __N_DIRECTIVES 1

typedef enum
{
	DIRECTIVE_ERROR,
	DIRECTIVE_DEFINE,
} directive_type_t;

typedef struct
{
	directive_type_t t;
	unsigned long long v;
} directive_pair_t;

static const directive_pair_t __directives[__N_DIRECTIVES] = {
	{ .t = DIRECTIVE_DEFINE, .v = 5500604938257788306ULL },
};

static const char *__directives_str[__N_DIRECTIVES] = {
	"define",
};

static unsigned long long __hash(const char *, size_t, size_t);
static directive_type_t __figure_out_directive(const char *, size_t linebegin, size_t lineend);
int __preprocess_line(const char *in, size_t linebegin, size_t lineend);

int typex_preprocess(const char *in, size_t in_len, owned_str_t *out)
{
	int err = 0;
	size_t out_len = in_len;

	for (size_t i = 0; i < in_len; ++i)
	{
		size_t linebegin = i;
		while (i < in_len && in[i] != '\n')
		{
			i++;		
		}

		if (i >= in_len) break;

		err = __preprocess_line(in, linebegin, i);
		if (err)
		{
			return err;
		}

		// the macro is going to be removed from the rest, so we can discount it from the size to allocate
		size_t linesize = i - linebegin;
		out_len -= linesize;
	}

	out->buff = malloc(sizeof(char) * out_len);
	if (out->buff == NULL)
	{
		return E_TYPEX_OUTOFMEM;
	}

	for (size_t i = 0; i < in_len; ++i)
	{
		out->buff[i] = in[i];
	}

	return 0;
}

#include <stdio.h>

int __preprocess_line(const char *in, size_t linebegin, size_t lineend)
{
	size_t i = linebegin;

	while (i < lineend && ISBLANK(in[i])) i++;

	// Skip empty lines
	if (i == lineend) return 0;
	// Skip non-macro lines
	if (in[i++] != '#') return 0;
	// If the only char in the line was a # we need to error out
	if (i == lineend) return E_INVALIDMACRODIRECTIVE;

	// Now we need to figure out what directive it is
	// so we need to search for the next space in the string, in[i..spaceidx] is going to be the directive
	size_t spaceidx = i;
	while (spaceidx < lineend && in[spaceidx++] != ' ');

	directive_type_t t = __figure_out_directive(in, i, spaceidx-1);

	switch (t)
	{
		case DIRECTIVE_ERROR:
		{
			return E_INVALIDMACRODIRECTIVE;
		}; break;
		case DIRECTIVE_DEFINE:
		{
			// TODO: Implement this.
			// I think we should add it to a hashmap of directives
			printf("define\n");
		}; break;
	}

	return 0;
}

static directive_type_t __figure_out_directive(const char *stream, size_t linebegin, size_t lineend)
{
	unsigned long long h = __hash(stream, linebegin, lineend);

	size_t low = 0;
	size_t high = __N_DIRECTIVES-1;

	if (low == high)
	{
		if (__directives[low].v == h) return __directives[low].t;
	}

	while (low <= high)
	{
		size_t i = (low+high)/2;
		if (__directives[i].v == h)
		{
			size_t l = lineend-linebegin;
			if (l == strlen(__directives_str[i]))
			{
				return __directives[i].t;
			}
			return DIRECTIVE_ERROR;
		}
		else if (__directives[i].v < h)
		{
			low = i+1;
		}
		else
		{
			if (i == 0) break;
			high = i-1;
		}
	}

	return DIRECTIVE_ERROR;
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


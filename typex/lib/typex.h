#ifndef TYPEX_H
#define TYPEX_H

#include <stdlib.h>

typedef struct
{
	char *buff;
	size_t len;
} owned_str_t;

#define E_TYPEX_OUTOFMEM 1
#define E_INVALIDMACRODIRECTIVE 2

int typex_preprocess(const char *in, size_t in_len, owned_str_t *out);

#endif


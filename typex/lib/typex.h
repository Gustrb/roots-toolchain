#ifndef TYPEX_H
#define TYPEX_H

#include <stdlib.h>

#define __TYPEX_MAX_DEFINITIONS 1024
#define __TYPEX_TABLE_SIZE 100

typedef struct
{
	char *buff;
	size_t len;
} owned_str_t;

typedef struct
{
	int next;
	int prev;
	
	size_t name_begin;
	size_t name_end;
	
	size_t replacement_begin;
	size_t replacement_end;
} typex_directive_define_t;

typedef struct
{
	const char *stream;

	typex_directive_define_t definitions[__TYPEX_MAX_DEFINITIONS];
	size_t definitions_len;

	// Indexes into the definitions table
	int buckets[__TYPEX_TABLE_SIZE];
} typex_context_t;

#define E_TYPEX_OUTOFMEM 1
#define E_INVALIDMACRODIRECTIVE 2

int typex_preprocess(const char *in, size_t in_len, owned_str_t *out);
int typex_new_ctx(typex_context_t *ctx, const char *stream);

#define E_TYPEXTOOMANYDEFINES 3
int typex_define_replacement(typex_context_t *ctx, typex_directive_define_t *d);
int typex_define_replacement_lookup(typex_context_t *ctx, size_t name_begin, size_t name_end, typex_directive_define_t *d);

#endif


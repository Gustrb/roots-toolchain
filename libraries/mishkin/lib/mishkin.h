#ifndef MISHKIN_H
#define MISHKIN_H

#include <stdlib.h>

typedef struct mishkin_arena_t {
    short next;

    char *bytes;
    size_t s;
} mishkin_arena_t;

#define MISHKIN_MAX_CONTEXT_ARENAS 1024

typedef struct {
    mishkin_arena_t arenas[MISHKIN_MAX_CONTEXT_ARENAS];
    size_t num_arenas;
} mishkin_allocation_context_t;


#define E_MISHKIN_OUTOFMEM 998
#define E_INVALID_ARGUMENT 999

int mishkin_init_context(mishkin_allocation_context_t *context);
void *mishkin_arena_alloc(mishkin_allocation_context_t *context, size_t size);
void mishkin_context_free(mishkin_allocation_context_t *context);

#endif

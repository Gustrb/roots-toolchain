#include <stdio.h>
#include <string.h>

#include "../lib/mishkin.h"

#define __MISHKIN_ARENA_LEN 4096

int __mishkin_allocate_arena(mishkin_arena_t *arena);

void *mishkin_arena_alloc(mishkin_allocation_context_t *context, size_t size)
{
    if (!context || !size) return NULL;

    int err = 0;

    mishkin_arena_t *head = &context->arenas[0];
    if (head->bytes == NULL)
    {
        err = __mishkin_allocate_arena(head);
        if (err)
        {
            return NULL;
        }

        context->num_arenas++;
    }

    for (size_t curr = 0; curr < context->num_arenas; curr++)
    {
        mishkin_arena_t *arena = &context->arenas[curr];
        if (arena->s + size <= __MISHKIN_ARENA_LEN)
        {
            void *ptr = arena->bytes + arena->s;
            arena->s += size;
            return ptr;
        }
    }

    // If we got here, it means no arena has enough space, so lets check if
    // we can allocate a new one
    if (context->num_arenas < MISHKIN_MAX_CONTEXT_ARENAS)
    {
        mishkin_arena_t *arena = &context->arenas[context->num_arenas];
        err = __mishkin_allocate_arena(arena);
        if (err)
        {
            return NULL;
        }

        context->num_arenas++;

        if (arena->s + size <= __MISHKIN_ARENA_LEN)
        {
            void *ptr = arena->bytes + arena->s;
            arena->s += size;
            return ptr;
        }
    }

    return NULL;
}

void mishkin_context_free(mishkin_allocation_context_t *context)
{
    if (!context)
    {
        return;
    }

    for (size_t curr = 0; curr < context->num_arenas; curr++)
    {
        mishkin_arena_t *arena = &context->arenas[curr];

        // Should always be different then NULL, but just in case
        if (arena->bytes)
        {
            free(arena->bytes);
            arena->bytes = NULL;
        }

        arena->s = 0;
    }

    context->num_arenas = 0;
}

int mishkin_init_context(mishkin_allocation_context_t *context)
{
    if (!context)
    {
        return E_INVALID_ARGUMENT;
    }

    context->num_arenas = 0;

    for (size_t i = 0; i < MISHKIN_MAX_CONTEXT_ARENAS; i++)
    {
        mishkin_arena_t *new_arena = &context->arenas[i];
        new_arena->s = 0;
        new_arena->bytes = NULL;
    }

    return 0;
}

int __mishkin_allocate_arena(mishkin_arena_t *arena)
{
    arena->bytes = malloc(__MISHKIN_ARENA_LEN);
    if (!arena->bytes)
    {
        return E_MISHKIN_OUTOFMEM;
    }

    arena->s = 0;

    return 0;
}

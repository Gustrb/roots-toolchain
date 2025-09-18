#include <stdio.h>
#include <string.h>

#include "lib.h"
#include "../lib/mishkin.h"

int __should_be_able_to_initialize_a_context(void);
int __should_be_able_to_allocate_stuff_inside_a_context(void);
int __should_not_fail_allocating_a_shitload_of_stuff(void);
int __should_fail_to_allocate_more_than_the_maximum_number_of_arenas(void);

int main(void)
{
    #define casename "mishkin"
    START_CASE;

    int err = 0;

    err = err || __should_be_able_to_initialize_a_context();
    err = err || __should_be_able_to_allocate_stuff_inside_a_context();
    err = err || __should_not_fail_allocating_a_shitload_of_stuff();
    err = err || __should_fail_to_allocate_more_than_the_maximum_number_of_arenas();

	if (!err)
	{
		SUCCESS;
	}

	#undef casename
	return err;
}

int __should_be_able_to_initialize_a_context(void)
{
    #define casename "should_be_able_to_initialize_a_context"
    START_CASE;

    mishkin_allocation_context_t ctx;
    int err = mishkin_init_context(&ctx);
    ASSERT_EQ(err, 0, "should error out when initializing a valid context");

    err = mishkin_init_context(NULL);
    ASSERT_EQ(err, E_INVALID_ARGUMENT, "should error out when initializing a valid context");

    SUCCESS;
    #undef casename
}

int __should_be_able_to_allocate_stuff_inside_a_context(void)
{
    #define casename "should_be_able_to_initialize_a_context"
    START_CASE;

    mishkin_allocation_context_t ctx;
    int err = mishkin_init_context(&ctx);
    ASSERT_EQ(err, 0, "should error out when initializing a valid context");

    char *ptr = mishkin_arena_alloc(&ctx, 1024);
    ASSERT_NEQ(ptr, NULL, "should be able to allocate stuff inside a context");

    mishkin_context_free(&ctx);

    SUCCESS;
    #undef casename
}

int __should_not_fail_allocating_a_shitload_of_stuff(void)
{
    #define casename "should_not_fail_allocating_a_shitload_of_stuff"
    START_CASE;

    mishkin_allocation_context_t ctx;
    int err = mishkin_init_context(&ctx);
    ASSERT_EQ(err, 0, "should error out when initializing a valid context");

    for (int i = 0; i < 100; ++i)
    {
        char *ptr = mishkin_arena_alloc(&ctx, 1000);
        ASSERT_NEQ(ptr, NULL, "should be able to allocate stuff inside a context");
    }

    ASSERT_EQ(ctx.num_arenas, 25, "should allocate 25 arenas");

    mishkin_context_free(&ctx);

    SUCCESS;
    #undef casename
}

int __should_fail_to_allocate_more_than_the_maximum_number_of_arenas(void)
{
    #define casename "should_fail_to_allocate_more_than_the_maximum_number_of_arenas"
    START_CASE;

    mishkin_allocation_context_t ctx;
    int err = mishkin_init_context(&ctx);
    ASSERT_EQ(err, 0, "should error out when initializing a valid context");

    for (int i = 0; i < 1024; ++i)
    {
        char *ptr = mishkin_arena_alloc(&ctx, 4096);
        ASSERT_NEQ(ptr, NULL, "should be able to allocate stuff inside a context");
    }

    char *ptr = mishkin_arena_alloc(&ctx, 4096);
    ASSERT_EQ(ptr, NULL, "should fail to allocate more than the maximum number of arenas");

    mishkin_context_free(&ctx);

    SUCCESS;
    #undef casename
}

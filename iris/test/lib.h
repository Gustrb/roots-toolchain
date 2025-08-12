#ifndef TEST_LIB
#define TEST_LIB

#include "../lib/iris.h"

#define START_CASE(name) printf("[TEST " name "]: Running tests...\n");
#define SUCCESS(name) \
	do \
	{ \
		printf("[TEST " name "]: All tests passes!!\n"); \
		return 0; \
	} while(0); 

#define FAIL(name, message) \
	do \
	{ \
		fprintf(stderr, "[FAIL " name "]: " message "\n");	\
		return 1; \
	} while (0);

#define ASSERT_EQ(v, err, name, message) \
	do \
	{ \
		if (v != err) FAIL(name, message); \
	} while (0);

#define ASSERT_TOKEN_EQ(name, s, e, l, c, type, tok) \
	do \
	{ \
		token_t t1; \
		t1.start = s; \
		t1.end = e; \
		t1.line = l; \
		t1.col = c; \
		t1.t = type; \
		if (tok_equal(name, &t1, &tok)) \
		{ \
			FAIL(name, "tokens didn't match"); \
		} \
	} while (0);

char tok_equal(const char *, token_t *e, token_t *g);

#endif


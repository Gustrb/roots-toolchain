#ifndef TEST_LIB
#define TEST_LIB

#include <stdio.h>
#include <stdlib.h>

#define START_CASE printf("[TEST " casename "]: Running tests...\n");
#define SUCCESS \
	do \
	{ \
		printf("[TEST " casename "]: All tests passes!!\n"); \
		return 0; \
	} while(0); 

#define FAIL(message) \
	do \
	{ \
		fprintf(stderr, "[FAIL " casename "]: " message "\n");	\
		return 1; \
	} while (0);

#define ASSERT_EQ(v, err, message) \
	do \
	{ \
		if (v != err) FAIL(message); \
	} while (0);

#define ASSERT_TOKEN_EQ(s, e, l, c, type, tok) \
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
			FAIL("tokens didn't match"); \
		} \
	} while (0);

typedef struct
{
	char *buff;
	size_t len;
} io_string_t;

#define E_FAILTOOPENFILE 1
#define E_FAILTOSEEKFILE 2
#define E_FAILTOTELLFILESIZE 3
#define E_FAILTOREWINDFILE 4
#define E_OUTOFMEM 5
#define E_FAILTOREAD 6

int file_into_memory(const char *, io_string_t *);

#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *__keywords[] = {
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",
};

static const size_t __nkeywords = 32;

static unsigned long long __hash(const char *, size_t);

int main(void)
{
	for (size_t i = 0; i < __nkeywords; ++i)
	{
		unsigned long long o = __hash(__keywords[i], strlen(__keywords[i]));
		printf("%s %llu\n", __keywords[i], o);
	}

	return 0;
}

static unsigned long long __hash(const char *d, size_t l)
{
	unsigned long long o = 0xcbf29ce484222325ULL;
	for (size_t i = 0; i < l; ++i)
	{
		o = (o ^ d[i]) * 0x100000001b3ULL;
	}

	return o;
}


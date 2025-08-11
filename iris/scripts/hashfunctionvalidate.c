// Validates that there is no __hash of same length that can collide with any keyword. 
// This is necessary since we check two things in the binary search
// we check for hash equallity and that the number of chars is the same.
// Find a permutation p having size p_s of the set of all alphanumeric characters such that:
// there is at least one element(e) in the set of keywords such that __hash(p, p_s) == __hash(e, e_s) and e_s = e_s, and e != p
// Since I don't really want to bother writing code that generates *ALL* the possible permutation, I am going to use some
// random sampling (check __SAMPLING_SIZE__ to see how many rounds). And I am going to assume this distribution is correct.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/iris.h"

#define __NKEYWORDS 32
#define __SAMPLING_SIZE 1000

static char alphanumeric_chars_set[63] = {0};

// sorted by value (hash)
static const unsigned long long __keywords[__NKEYWORDS] = {
	417800907370030071ull,
	617372513566700692ull,
	628023482707099174ull,
	1390995973168426656ull,
	3143511548502526014ull,
	3563412735833858527ull,
	5416238596490301492ull,
	5788750116080139093ull,
	6757501175350325814ull,
	7334518323283222324ull,
	9163537027783908656ull,
	9964885110000857501ull,
	10474992716130377088ull,
	10644074229358120504ull,
	10769778583944859405ull,
	11331286950270920518ull,
	11532138274943533413ull,
	11567507311810436776ull,
	11936925801837197745ull,
	13068990443679199185ull,
	13236544317732455142ull,
	14058172984575203104ull,
	14210125031242066299ull,
	14251563519059995999ull,
	14837330719131395891ull,
	14882043299657492846ull,
	15186091406668687012ull,
	15902905282948881040ull,
	16509985740318510052ull,
	16982411286042166782ull,
	17483980429552467645ull,
	17767075776831802709ull,
};

static const char *__keywords_str[__NKEYWORDS] = {
	"extern", "do", "if", "register", "int", "void",
	"union", "signed", "auto", "const", "else", "sizeof",
	"enum", "break", "volatile", "goto", "float",
	"double", "switch", "case", "unsigned", "struct",
	"static", "return", "long", "while", "continue",
	"for", "typedef", "default", "char", "sort",
};

static unsigned long long __hash(const char *d, size_t l);

int main(void)
{
	srand(time(NULL));
	size_t i = 0;
	for (char c = 'a'; c <= 'z'; ++c)
	{
		alphanumeric_chars_set[i++] = c;	
	}

	for (char c = 'A'; c <= 'Z'; ++c)
	{
		alphanumeric_chars_set[i++] = c;	
	}

	for (char c = '0'; c <= '9'; ++c)
	{
		alphanumeric_chars_set[i++] = c;	
	}

	alphanumeric_chars_set[i++] = '_';	
	
	if (i != 63)
	{
		fprintf(stderr, "[ERROR]: There are 63 possible alphanumeric digits, got %zu\n", i);
		return 1;
	}

	for (size_t i = 0; i < __NKEYWORDS; ++i)
	{
		const char *e = __keywords_str[i];
		int e_s = strlen(e);
		char buff[10];

		// Generate __SAMPLING_SIZE random strings	
		for (size_t s = 0; s < __SAMPLING_SIZE; ++s)
		{
			for (size_t j = 0; j < e_s; j++)
			{
				size_t ri = rand() % 63;
				buff[j] = ri;

				unsigned long long h = __hash(buff, e_s);	
				if (h == __keywords[i])
				{
					fprintf(stdout, "[INFO]: Found hash collision with: %s and %.*s\n", e, e_s, buff);
					return 1;
				}
			}
		}
	}


	// Ran 10 times and got no collisions, so we are good to go :P
	fprintf(stdout, "[INFO]: No hash collisions\n");

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


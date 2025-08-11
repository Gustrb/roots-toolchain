#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long nums[1024];

void __str_to_ull(const char *in, size_t, int i);
void __qsort(int, int);

int main(int argc, const char **argv)
{
	if (argc == 1) return 0;

	for (int i = 0; i < argc-1; ++i)
	{
		__str_to_ull(argv[i+1], strlen(argv[i+1]), i);
	}
	
	__qsort(0, argc-2);

	for (int i = 0; i < argc-1; i++)
	{
		fprintf(stdout, "%llu\n", nums[i]);
	}


	return 0;
}

void __str_to_ull(const char *in, size_t inlen, int ni)
{
	unsigned long long acc = 0;
	nums[ni] = 0;

	for (int i = 0; i < (int)inlen; ++i)
	{
		acc = acc * 10 + (in[i] - '0');
	}
	nums[ni] = acc;
}

int __partition(int low, int high)
{
	unsigned long long p = nums[low];
	int i = low;
	int j = high;
	while (i < j)
	{
		while (nums[i] <= p && i <= high-1)
		{
			i++;
		}

		while (nums[j] >= p && j >= low + 1) {
			j--;
		}
		if (i < j)
		{
			unsigned long long tmp = nums[i];
			nums[i] = nums[j];
			nums[j] = tmp;
		}
	}

	unsigned long long tmp = nums[low];
	nums[low] = nums[j];
	nums[j] = tmp;
	return j;
}

// TODO: Debug this, it is not sorting correctly
void __qsort(int low, int high)
{
	if (low < high)
	{
		int p = __partition(low, high);
		__qsort(low, p-1);
		__qsort(p+1, high);
	}
}


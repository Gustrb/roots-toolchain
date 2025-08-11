#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define E_INVALIDCAST 2

unsigned long long nums[1024];

int __str_to_ull(const char *in, size_t, int i);
void __qsort(int, int);

int main(int argc, const char **argv)
{
	if (argc == 1) return 0;

	for (int i = 0; i < argc-1; ++i)
	{
		int err = __str_to_ull(argv[i+1], strlen(argv[i+1]), i);
		if (err)
		{
			fprintf(stderr, "Failed converting: %s to unsigned long long\n", argv[i+1]);
			return err;
		}
	}
	
	__qsort(0, argc-2);

	for (int i = 0; i < argc-1; i++)
	{
		fprintf(stdout, "%llu\n", nums[i]);
	}


	return 0;
}

int __str_to_ull(const char *in, size_t inlen, int ni)
{
	unsigned long long acc = 0;
	nums[ni] = 0;

	for (int i = 0; i < (int)inlen; ++i)
	{
		char b = in[i];
		if (!((b >= '0') && (b <= '9'))) return E_INVALIDCAST;
		acc = acc * 10 + (b - '0');
	}
	nums[ni] = acc;

	return 0;
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

void __qsort(int low, int high)
{
	if (low < high)
	{
		int p = __partition(low, high);
		__qsort(low, p-1);
		__qsort(p+1, high);
	}
}


#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

int file_into_memory(const char *filepath, io_string_t *iostr)
{
	FILE *f = fopen(filepath, "r");
	if (!f)
	{
		return E_FAILTOOPENFILE;
	}

	if (fseek(f, 0, SEEK_END) == -1)
	{
		fclose(f);
		return E_FAILTOSEEKFILE;
	}
	
	long fsize = ftell(f);
	if (fsize == -1)
	{
		fclose(f);
		return E_FAILTOTELLFILESIZE;
	}

	rewind(f);

	iostr->buff = malloc(sizeof(char) * fsize);
	if (iostr->buff == NULL)
	{
		fclose(f);
		return E_OUTOFMEM;
	}
	iostr->len = (size_t)fsize;

	if (fread(iostr->buff, iostr->len, sizeof(char), f) == 0)
	{
		free(iostr->buff);
		fclose(f);
		return E_FAILTOREAD;
	}

	fclose(f);

	return 0;
}


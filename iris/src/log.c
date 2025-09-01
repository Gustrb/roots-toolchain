#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../lib/log.h"

#define E_FAILSTRFTIME 9999

static int __refresh_date(char *datebuff, size_t l);

int iris_log(log_level_t level, const char *message, ...)
{
#ifndef DEBUG
	(void) message;
	if (level == LOG_LEVEL_DEBUG) return 0;
#endif

	char datebuff[64];
	int err = __refresh_date(datebuff, sizeof(datebuff));
	if (err) return err;

	va_list args;
	va_start(args, message);

	const char *msg = "error";
	FILE *p = stdout;

	switch (level)
	{
		case LOG_LEVEL_INFO:  { p = stdout; msg = "INFO";  }; break;
		case LOG_LEVEL_DEBUG: { p = stdout; msg = "DEBUG"; }; break;
		case LOG_LEVEL_ERROR: { p = stderr; msg = "ERROR"; }; break;
	}

	fprintf(p, "[%s %s]: ", datebuff, msg);
	vfprintf(p, message, args);
	fprintf(p, "\n");
	va_end(args);

	return 0;
}

static int __refresh_date(char *datebuff, size_t l)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	size_t ret = strftime(datebuff, l, "%c", tm);
	if (!ret) return E_FAILSTRFTIME;
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define E_FAILSTRTIME    1
#define E_FAILDISTCREATE 2
#define E_OUTOFMEM       3

#define LOG_LEVEL_INFO  0
#define LOG_LEVEL_ERROR 1

static char datebuf[64] = {0};

#define LOG(level, ...) \
        do { \
                int err; \
                if ((err = __refresh_date())) return err; \
                switch (level) \
                { \
                        case LOG_LEVEL_INFO:  { fprintf(stdout, "[%s INFO]: ", datebuf); fprintf(stdout, __VA_ARGS__); }; break;  \
                        case LOG_LEVEL_ERROR: { fprintf(stderr, "[%s ERROR]: ", datebuf); fprintf(stderr, __VA_ARGS__); }; break; \
                } \
        } while(0); \

static const char *distfile = "dist";
static const char *executable_path = "./src/iris.c";

static char *build_loglib_args =
	"gcc "
	"-c "
        "./src/log.c "
        "-o "
        "./dist/log.o "
        "-Wall "
        "-Wextra "
	"-Werror "
        "-pedantic";

static char *build_command_args =
        "gcc "
	"-c "
        "./src/iris.c "
        "-o "
        "./dist/iris.o "
        "-Wall "
        "-Wextra "
	"-Werror "
        "-pedantic";

static char *build_test_lib_command_args =
        "gcc "
	"-c "
        "./test/lib.c "
        "-o "
        "./dist/testlib.o "
        "-Wall "
        "-Wextra "
	"-Werror "
        "-pedantic";

static char *build_tests_command_args =
        "gcc "
        "./test/unittest.c "
        "-o "
        "./dist/unittest "
	"./dist/iris.o "
	"./dist/log.o "
	"./dist/testlib.o "
        "-Wall "
        "-Wextra "
	"-Werror "
        "-pedantic";

static const char *testarg = "--test";
static const char *debugarg = "--debug";
static const char *debugflag = " -DDEBUG ";

static int __refresh_date(void);
static int __build_executable(char debug);
static int __build_loglib(char debug);
static int __build_test(char debug);

int main(int argc, char **argv)
{
	char test  = 0;
	char debug = 0;

	// parsing args
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strncmp(argv[i], testarg, strlen(testarg)) == 0)
			{
				test = 1;
			}
			else if (strncmp(argv[i], debugarg, strlen(debugarg)) == 0)
			{
				debug = 1;
			}

		}
	}

        struct stat st = {0};
        LOG(LOG_LEVEL_INFO, "Checking if \"%s\" folder exists...\n", distfile);
        if (stat(distfile, &st) == -1)
        {
                LOG(LOG_LEVEL_INFO, "\"%s\" folder does not exist... Creating it\n", distfile);
                if (mkdir(distfile, 0700))
                {
                        LOG(LOG_LEVEL_ERROR, "Failed to create \"%s\" file, exiting out...\n", distfile);
                        return E_FAILDISTCREATE;
                }

        }
        else
        {
                LOG(LOG_LEVEL_INFO, "\"%s\" folder exists. Skipping creation\n", distfile);
        }

        int err;
        if ((err = __build_loglib(debug)))
        {
                LOG(LOG_LEVEL_ERROR, "Failed to build lob library :( exiting out...\n");
                return err;
        }

        if ((err = __build_executable(debug)))
        {
                LOG(LOG_LEVEL_ERROR, "Failed to build executable at: \"%s\" exiting out...\n", executable_path);
                return err;
        }

        LOG(LOG_LEVEL_INFO, "Finished building the app\n");
	if (test)
	{
		if ((err = __build_test(debug)))
		{
			LOG(LOG_LEVEL_ERROR, "Failed to build executable at: \"%s\" exiting out...\n", build_tests_command_args);
			return err;
		}

		LOG(LOG_LEVEL_INFO, "Finished building the tests\n");
	}

        return 0;
}

static int __refresh_date(void)
{
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        size_t ret = strftime(datebuf, sizeof(datebuf), "%c", tm);
        if (!ret)
        {
                return E_FAILSTRTIME;
        }

        return 0;
}

static int __build_executable(char debug)
{
	size_t l = strlen(build_command_args);
	size_t stoalloc = l + 1;
	if (debug)
	{
		stoalloc += strlen(debugflag);
	}

	char *buff = malloc(stoalloc);
	if (buff == NULL)
	{
		return E_OUTOFMEM;
	}

	size_t i = 0;
	for (i = 0; i < l; ++i)
	{
		buff[i] = build_command_args[i];
	}

	if (debug)
	{
		for (size_t j = 0; j < strlen(debugflag); ++j)
		{
			buff[i++] = debugflag[j];
		}
	}

	buff[i] = '\0';

        LOG(LOG_LEVEL_INFO, "Running %s to build the project\n", buff);
        if (system(buff)) return 1;
        return 0;
}

static int __build_loglib(char debug)
{
	size_t l = strlen(build_loglib_args);
	size_t stoalloc = l + 1;
	if (debug)
	{
		stoalloc += strlen(debugflag);
	}

	char *buff = malloc(stoalloc);
	if (buff == NULL)
	{
		return E_OUTOFMEM;
	}

	size_t i = 0;
	for (i = 0; i < l; ++i)
	{
		buff[i] = build_loglib_args[i];
	}

	if (debug)
	{
		for (size_t j = 0; j < strlen(debugflag); ++j)
		{
			buff[i++] = debugflag[j];
		}
	}

	buff[i] = '\0';

        LOG(LOG_LEVEL_INFO, "Running %s to build the log library\n", buff);
        if (system(buff)) return 1;
        return 0;
}

static int __build_test_lib(char debug)
{
	size_t l = strlen(build_test_lib_command_args);
	size_t stoalloc = l + 1;
	if (debug)
	{
		stoalloc += strlen(debugflag);
	}

	char *buff = malloc(stoalloc);
	if (buff == NULL)
	{
		return E_OUTOFMEM;
	}

	size_t i = 0;
	for (i = 0; i < l; ++i)
	{
		buff[i] = build_test_lib_command_args[i];
	}

	if (debug)
	{
		for (size_t j = 0; j < strlen(debugflag); ++j)
		{
			buff[i++] = debugflag[j];
		}
	}

	buff[i] = '\0';

        LOG(LOG_LEVEL_INFO, "Running %s to build the test lib\n", buff);
        if (system(buff)) return 1;

	free(buff);
        return 0;
}

static int __build_test(char debug)
{
	int err;
	if ((err = __build_test_lib(debug)))
	{
		return err;
	}

	size_t l = strlen(build_tests_command_args);
	size_t stoalloc = l + 1;
	if (debug)
	{
		stoalloc += strlen(debugflag);
	}

	char *buff = malloc(stoalloc);
	if (buff == NULL)
	{
		return E_OUTOFMEM;
	}

	size_t i = 0;
	for (i = 0; i < l; ++i)
	{
		buff[i] = build_tests_command_args[i];
	}

	if (debug)
	{
		for (size_t j = 0; j < strlen(debugflag); ++j)
		{
			buff[i++] = debugflag[j];
		}
	}

	buff[i] = '\0';

        LOG(LOG_LEVEL_INFO, "Running %s to build the tests\n", buff);
        if (system(buff)) return 1;

	free(buff);
        return 0;
}


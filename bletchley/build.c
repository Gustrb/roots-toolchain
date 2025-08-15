#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define E_FAILSTRTIME    1
#define E_FAILDISTCREATE 2
#define E_BUFFEROVERFLOW 999

#define LOG_LEVEL_INFO  0
#define LOG_LEVEL_ERROR 1

#define __COMMAND_BUFFER_SIZE 4096

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

#define __C_APPEND(c, str) \
	do \
		if ((err = command_append_arg(&c, str))) return err;\
	while (0);

typedef struct
{
	char buff[__COMMAND_BUFFER_SIZE];
	size_t last;
} command_t;

static const char *distfile = "dist";
static const char *executable_path = "./src/bletchley.c";
static const char *debugarg = "--debug";
static const char *debugflag = " -DDEBUG ";

static char datebuf[64] = {0};

static int __refresh_date(void);
static int __build_executable(char debug);
static int __build_iris(char debug);
static int command_append_arg(command_t *c, const char *arg);
static int command_run(command_t *c); 

int main(int argc, char **argv)
{
	char debug = 0;

	// parsing args
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strncmp(argv[i], debugarg, strlen(debugarg)) == 0)
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
        if ((err = __build_iris(debug)))
        {
                LOG(LOG_LEVEL_ERROR, "Failed to build lexer :( exiting out...\n");
                return err;
        }


        if ((err = __build_executable(debug)))
        {
                LOG(LOG_LEVEL_ERROR, "Failed to build executable at: \"%s\" exiting out...\n", executable_path);
                return err;
        }

        LOG(LOG_LEVEL_INFO, "Finished building the app\n");

	return 0;
}

static int __build_executable(char debug)
{
	int err;
	command_t c = {0};

	__C_APPEND(c, "gcc ");
	__C_APPEND(c, "./src/bletchley.c ");
	__C_APPEND(c, "-o ");
	__C_APPEND(c, "./dist/bletchley ");
	__C_APPEND(c, "./dist/iris.o ");
	__C_APPEND(c, "-Wall ");
	__C_APPEND(c, "-Wextra ");
	__C_APPEND(c, "-Werror ");
	__C_APPEND(c, "-pedantic ");

	if (debug)
	{
		__C_APPEND(c, debugflag);
	}

	if ((err = command_run(&c)))
	{
		return err;
	}

        LOG(LOG_LEVEL_INFO, "Running %s to build the project\n", c.buff);
        return 0;
}

static int __build_iris(char debug)
{
	int err;
	command_t c = {0};

	__C_APPEND(c, "gcc ");
	__C_APPEND(c, "-c ");
	__C_APPEND(c, "../iris/src/iris.c ");
	__C_APPEND(c, "-o ");
	__C_APPEND(c, "./dist/iris.o ");
	__C_APPEND(c, "-Wall ");
	__C_APPEND(c, "-Wextra ");
	__C_APPEND(c, "-Werror ");
	__C_APPEND(c, "-pedantic ");

	if (debug)
	{
		__C_APPEND(c, debugflag);
	}

	if ((err = command_run(&c)))
	{
		return err;
	}

        LOG(LOG_LEVEL_INFO, "Running %s to build the lexer\n", c.buff);
        return 0;
}

static int command_append_arg(command_t *c, const char *arg)
{
	size_t len = strlen(arg);
	// We want to keep the invariant of
	// c->last < __COMMAND_BUFFER_SIZE - 1
	// Because that is where we are going to put the null byte
	if (c->last + len > __COMMAND_BUFFER_SIZE-1)
	{
		return E_BUFFEROVERFLOW;
	}

	for (size_t i = 0; i < len; ++i)
	{
		c->buff[c->last++] = arg[i];
	}

	return 0;
}


static int command_run(command_t *c)
{
	if (c->last > __COMMAND_BUFFER_SIZE)
	{
		return E_BUFFEROVERFLOW;
	}

	c->buff[c->last] = '\0';
	return system(c->buff);
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


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define E_FAILSTRTIME    1
#define E_FAILDISTCREATE 2

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

static char *build_command_args =
        "gcc "
	"-c "
        "./src/iris.c "
        "-o "
        "./dist/iris.o "
        "-Wall "
        "-Wextra "
        "-pedantic";

static int __refresh_date(void);
static int __build_executable(void);

int main(void)
{
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
        if ((err = __build_executable()))
        {
                LOG(LOG_LEVEL_ERROR, "Failed to build executable at: \"%s\" exiting out...\n", executable_path);
                return err;
        }


        LOG(LOG_LEVEL_INFO, "Finished building the app\n");

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

static int __build_executable(void)
{
        LOG(LOG_LEVEL_INFO, "Running %s to build the project\n", build_command_args);
        if (system(build_command_args)) return 1;
        return 0;
}


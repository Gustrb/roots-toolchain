#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define __PATH_BUFF_MAX 4096
#define __MAX_HEADERS_COUNT 1024

#define E_NOTENOUGHARGS    1
#define E_PATHTOOLONG      2
#define E_FAILTOFORMATPATH 3
#define E_FILENOTFOUND     4
#define E_FAILTOOPENFILE  5
#define E_FAILTOSEEKFILE  6
#define E_FAILTOTELLFILESIZE 7
#define E_OUTOFMEM 8
#define E_FAILTOREAD 9
#define E_INVALIDBUILDTYPE 10

typedef struct {
    char *buff;
    size_t len;
} io_string_t;

typedef struct {
    const char *inputfolder;
    size_t inputfolder_len;
} bobthebuilder_cli_args_t;

typedef struct {
    const char *buildfile;
    size_t buildfile_len;

    io_string_t contents;
} bobthebuilder_buildfile_t;

typedef enum {
    BOBTHEBUILDER_BUILD_TYPE_LIBRARY,
} bobthebuilder_build_type_t;

typedef struct {
    bobthebuilder_build_type_t t;

    char outputpath[__PATH_BUFF_MAX];
    size_t outputpath_len;

    char *headers[__MAX_HEADERS_COUNT];
    size_t headers_count;

    union {
        struct {
            char *library_path;
            size_t library_path_len;
        } library;

        // TODO: Add support for executable
    } as;
} bobthebuilder_buildfile_context_t;

static const char *__error_messages[] = {
    [E_NOTENOUGHARGS] = "Not enough arguments provided",
    [E_PATHTOOLONG] = "Path too long",
    [E_FAILTOFORMATPATH] = "Failed to format path",
    [E_FILENOTFOUND] = "buildfile not found at the root of the project",
    [E_FAILTOOPENFILE] = "Failed to open buildfile",
    [E_FAILTOSEEKFILE] = "Failed to seek buildfile",
    [E_FAILTOTELLFILESIZE] = "Failed to tell buildfile size",
    [E_OUTOFMEM] = "Out of memory",
    [E_FAILTOREAD] = "Failed to read buildfile",
    [E_INVALIDBUILDTYPE] = "Invalid build type",
};

int cli_parse(int argc, char **argv, bobthebuilder_cli_args_t *args);
int __figure_out_buildfile(bobthebuilder_cli_args_t *args, bobthebuilder_buildfile_t *buildfile);
int __parse_buildfile(bobthebuilder_buildfile_context_t *context, bobthebuilder_buildfile_t *buildfile);
int __file_into_memory(const char *filepath, io_string_t *iostr);
int bobthebuilder_cleanup(bobthebuilder_buildfile_context_t *context, bobthebuilder_buildfile_t *buildfile);

int main(int argc, char **argv)
{
    bobthebuilder_cli_args_t cli;
    int err;

    if ((err = cli_parse(argc, argv, &cli)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    bobthebuilder_buildfile_t buildfile;
    if ((err = __figure_out_buildfile(&cli, &buildfile)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    bobthebuilder_buildfile_context_t context = {0};
    if ((err = __parse_buildfile(&context, &buildfile)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

#ifdef DEBUG
    printf("Build type: %d\n", context.t);
    printf("Output path: %s\n", context.outputpath);
    printf("Headers count: %zu\n", context.headers_count);
    for (size_t i = 0; i < context.headers_count; i++)
    {
        printf("Header %zu: %s\n", i, context.headers[i]);
    }
    printf("Library path: %s\n", context.as.library.library_path);
#endif

    bobthebuilder_cleanup(&context, &buildfile);
    return 0;
}

#define ADVANCE_ARG(argc, argv) \
    argc--; argv++;

int cli_parse(int argc, char **argv, bobthebuilder_cli_args_t *args)
{
    (void) args;
    
    if (argc == 1)
    {
        return E_NOTENOUGHARGS;
    }

    // consume program name
    ADVANCE_ARG(argc, argv);

    if (argc == 1)
    {
        // We got only one argument, interpret it as the path to the project 
        args->inputfolder = *argv;
        args->inputfolder_len = strlen(args->inputfolder);
        return 0;
    }

    fprintf(stderr, "[ERROR]: Not implemented yet, currently only one argument is supported\n");
 
    return 0;
}

const char *__buildfile_name = "buildfile";

int __figure_out_buildfile(bobthebuilder_cli_args_t *args, bobthebuilder_buildfile_t *buildfile)
{
    char buildfile_path_buff[__PATH_BUFF_MAX];

    // If the new file does not fit into the buffer, fuck it, let's error out, who tf need a path larger than 1 fucking OS page.
    if (args->inputfolder_len + strlen(__buildfile_name) + 1 > __PATH_BUFF_MAX)
    {
        return E_PATHTOOLONG;
    }

    // TODO: this is retarded, if it ends with a slash it won't work. Fix it plis.
    if (snprintf(buildfile_path_buff, __PATH_BUFF_MAX, "%s/%s", args->inputfolder, __buildfile_name) < 0)
    {
        // how tf can this happen?
        return E_FAILTOFORMATPATH;
    }

    // So now we need to check if the file exists.
    if (access(buildfile_path_buff, F_OK) == -1)
    {
        return E_FILENOTFOUND;
    }

    buildfile->buildfile = buildfile_path_buff;
    buildfile->buildfile_len = strlen(buildfile_path_buff);

    return 0;
}

int __parse_buildfile(bobthebuilder_buildfile_context_t *context, bobthebuilder_buildfile_t *buildfile)
{
    int err;
    if ((err = __file_into_memory(buildfile->buildfile, &buildfile->contents)))
    {
        return err;
    }

    // The file is a list with the following format:
    // key=value
    // And we have a few specific keys that we care about (we can ignore the rest):
    // - type
    // - output
    // - headers
    // - lib
    char *rest_of_buildfile = buildfile->contents.buff;
    char *rest_of_line;
    char *line = strtok_r(rest_of_buildfile, "\n", &rest_of_buildfile);
    while ((line = strtok_r(rest_of_buildfile, "\n", &rest_of_buildfile)) != NULL)
    {
        char *line_copy = strdup(line);

        char *key = strtok_r(line_copy, "=", &rest_of_line);
        char *value = strtok_r(NULL, "=", &rest_of_line);

        if (key == NULL || value == NULL)
        {
            free(line_copy);
            continue;
        }

        if (strcmp(key, "type") == 0)
        {
            if (strcmp(value, "library") == 0)
            {
                context->t = BOBTHEBUILDER_BUILD_TYPE_LIBRARY;
                free(line_copy);
                continue;
            }
            else
            {
                free(line_copy);
                return E_INVALIDBUILDTYPE;
            }
        }

        if (strcmp(key, "output") == 0)
        {
            memcpy(context->outputpath, value, __PATH_BUFF_MAX);
            context->outputpath_len = strlen(value);
            free(line_copy);
            continue;
        }

        if (strcmp(key, "headers") == 0)
        {
            if (context->headers_count >= __MAX_HEADERS_COUNT)
            {
                return E_OUTOFMEM;
            }

            context->headers_count++;
            context->headers[context->headers_count - 1] = strdup(value);
            free(line_copy);
            continue;
        }

        if (strcmp(key, "lib") == 0)
        {
            context->as.library.library_path = strdup(value);
            context->as.library.library_path_len = strlen(value);
            free(line_copy);
            continue;
        }

        free(line_copy);
    }

    return 0;
}

int __file_into_memory(const char *filepath, io_string_t *iostr)
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

int bobthebuilder_cleanup(bobthebuilder_buildfile_context_t *context, bobthebuilder_buildfile_t *buildfile)
{
    free(buildfile->contents.buff);

    if (context->t == BOBTHEBUILDER_BUILD_TYPE_LIBRARY)
    {
        for (size_t i = 0; i < context->headers_count; i++)
        {
            free(context->headers[i]);
        }
        free(context->as.library.library_path);
    }

    return 0;
}

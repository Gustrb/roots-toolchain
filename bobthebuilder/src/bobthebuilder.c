#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define __PATH_BUFF_MAX 4096
#define __MAX_HEADERS_COUNT 1024
#define __COMMAND_BUFFER_SIZE 4096
#define __MAX_LINKWITH_COUNT 1024

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
#define E_BUFFEROVERFLOW 11 
#define E_FAILTORUNCOMMAND 12
#define E_FAILTOCREATEFOLDER 13

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

    char *dependencies[__MAX_LINKWITH_COUNT];
    size_t dependencies_count;

    char *linkwith[__MAX_LINKWITH_COUNT];
    size_t linkwith_count;

    union {
        struct {
            char *library_path;
            size_t library_path_len;
        } library;

        // TODO: Add support for executable
    } as;
} bobthebuilder_buildfile_context_t;

typedef struct
{
	char buff[__COMMAND_BUFFER_SIZE];
	size_t last;
} command_t;

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
    [E_BUFFEROVERFLOW] = "Buffer overflow",
    [E_FAILTORUNCOMMAND] = "Failed to run command",
    [E_FAILTOCREATEFOLDER] = "Failed to create folder",
};

int cli_parse(int argc, char **argv, bobthebuilder_cli_args_t *args);
int __figure_out_buildfile(bobthebuilder_cli_args_t *args, bobthebuilder_buildfile_t *buildfile);
int __parse_buildfile(bobthebuilder_buildfile_context_t *context, bobthebuilder_buildfile_t *buildfile);
int __file_into_memory(const char *filepath, io_string_t *iostr);

int __bobthebuilder_cleanup_buildfile(bobthebuilder_buildfile_t *buildfile);
int __bobthebuilder_cleanup_context(bobthebuilder_buildfile_context_t *context);

int bobthebuilder_build(bobthebuilder_cli_args_t *cli_args, bobthebuilder_buildfile_context_t *context);
int bobthebuilder_build_library(bobthebuilder_cli_args_t *cli_args, bobthebuilder_buildfile_context_t *context);

int bobthebuilder_run(bobthebuilder_cli_args_t *cli_args, bobthebuilder_buildfile_context_t *context);

int __copy_file(const char *src, const char *dst);

static int command_append_arg(command_t *c, const char *arg);
static int command_run(command_t *c); 

int main(int argc, char **argv)
{
    bobthebuilder_cli_args_t cli;
    int err;

    if ((err = cli_parse(argc, argv, &cli)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    bobthebuilder_buildfile_context_t context = {0};
    if ((err = bobthebuilder_run(&cli, &context)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    __bobthebuilder_cleanup_context(&context);

    return 0;
}

int bobthebuilder_run(bobthebuilder_cli_args_t *cli, bobthebuilder_buildfile_context_t *context)
{
    int err;
    bobthebuilder_buildfile_t buildfile;
    if ((err = __figure_out_buildfile(cli, &buildfile)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    if ((err = __parse_buildfile(context, &buildfile)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

#ifdef DEBUG
    printf("Build type: %d\n", context->t);
    printf("Output path: %s\n", context->outputpath);
    printf("Headers count: %zu\n", context->headers_count);
    for (size_t i = 0; i < context->headers_count; i++)
    {
        printf("Header %zu: %s\n", i, context->headers[i]);
    }

    printf("Dependencies count: %zu\n", context->dependencies_count);
    for (size_t i = 0; i < context->dependencies_count; i++)
    {
        printf("Dependency %zu: %s\n", i, context->dependencies[i]);
    }

    printf("Linkwith count: %zu\n", context->linkwith_count);
    for (size_t i = 0; i < context->linkwith_count; i++)
    {
        printf("Linkwith %zu: %s\n", i, context->linkwith[i]);
    }

    printf("Library path: %s\n", context->as.library.library_path);
#endif

    if ((err = bobthebuilder_build(cli, context)))
    {
        fprintf(stderr, "[ERROR]: %s\n", __error_messages[err]);
        return err;
    }

    __bobthebuilder_cleanup_buildfile(&buildfile);

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

        if (strcmp(key, "dependencies") == 0)
        {
            if (context->dependencies_count >= __MAX_LINKWITH_COUNT)
            {
                return E_OUTOFMEM;
            }

            context->dependencies_count++;
            context->dependencies[context->dependencies_count - 1] = strdup(value);
            free(line_copy);
            continue;
        }

        if (strcmp(key, "linkwith") == 0)
        {
            if (context->linkwith_count >= __MAX_LINKWITH_COUNT)
            {
                return E_OUTOFMEM;
            }

            context->linkwith_count++;
            context->linkwith[context->linkwith_count - 1] = strdup(value);
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

int __bobthebuilder_cleanup_buildfile(bobthebuilder_buildfile_t *buildfile)
{
    free(buildfile->contents.buff);

    return 0;
}

int __bobthebuilder_cleanup_context(bobthebuilder_buildfile_context_t *context)
{
    if (context->t == BOBTHEBUILDER_BUILD_TYPE_LIBRARY)
    {
        for (size_t i = 0; i < context->headers_count; i++)
        {
            free(context->headers[i]);
        }
        free(context->as.library.library_path);
    }

    for (size_t i = 0; i < context->dependencies_count; i++)
    {
        free(context->dependencies[i]);
    }

    for (size_t i = 0; i < context->linkwith_count; i++)
    {
        free(context->linkwith[i]);
    }

    return 0;
}

int bobthebuilder_build(bobthebuilder_cli_args_t *cli_args, bobthebuilder_buildfile_context_t *context)
{
    switch (context->t)
    {
        case BOBTHEBUILDER_BUILD_TYPE_LIBRARY: return bobthebuilder_build_library(cli_args, context);

        default:
            fprintf(stderr, "[ERROR]: Not implemented yet\n");
            return E_INVALIDBUILDTYPE;
    }
}

static const char *compiler = "cc ";

#define __C_APPEND(c, str) \
	do \
		if ((err = command_append_arg(&c, str))) return err;\
	while (0);


int bobthebuilder_build_library(bobthebuilder_cli_args_t *cli_args, bobthebuilder_buildfile_context_t *context)
{
    // Here we are hardcoding the output folder to be dist/
    // that probably should not be the case, but it is what it is for now.
    char output_folder[__PATH_BUFF_MAX];
    if (snprintf(output_folder, __PATH_BUFF_MAX, "%s/dist", cli_args->inputfolder) < 0)
    {
        return E_FAILTOFORMATPATH;
    }

    struct stat st = {0};
    if (stat(output_folder, &st) == -1)
    {
        if (mkdir(output_folder, 0700))
        {
            return E_FAILTOCREATEFOLDER;
        }
    }

    int err;
    for (size_t i = 0; i < context->dependencies_count; i++)
    {
        bobthebuilder_cli_args_t libcliargs = {0};
        bobthebuilder_buildfile_context_t libcontext = {0};

        libcliargs.inputfolder = context->dependencies[i];
        libcliargs.inputfolder_len = strlen(libcliargs.inputfolder);

        if ((err = bobthebuilder_run(&libcliargs, &libcontext)))
        {
            return err;
        }

        // We need to copy the headers from the library to the lib folder
        for (size_t j = 0; j < libcontext.headers_count; j++)
        {
            // We need to get the headers list, copy them from the lib's folder into
            // the current project's folder
            char header_path[__PATH_BUFF_MAX];
            snprintf(header_path, __PATH_BUFF_MAX, "%s/%s", libcliargs.inputfolder, libcontext.headers[j]);

            char dst_header_path[__PATH_BUFF_MAX];
            snprintf(dst_header_path, __PATH_BUFF_MAX, "%s/%s", cli_args->inputfolder, libcontext.headers[j]);

            if ((err = __copy_file(header_path, dst_header_path)))
            {
                return err;
            }
        }

        __bobthebuilder_cleanup_context(&libcontext);
    }

	command_t c = {0};

	__C_APPEND(c, compiler);
    __C_APPEND(c, " -c ");
    __C_APPEND(c, " ");

	__C_APPEND(c, cli_args->inputfolder);
    __C_APPEND(c, "/");
	__C_APPEND(c, context->as.library.library_path);

	__C_APPEND(c, " -o ");

	__C_APPEND(c, cli_args->inputfolder);
    __C_APPEND(c, "/");
	__C_APPEND(c, context->outputpath);

    for (size_t i = 0; i < context->linkwith_count; i++)
    {
        __C_APPEND(c, " ");
        __C_APPEND(c, context->linkwith[i]);
        __C_APPEND(c, " ");
    }

    __C_APPEND(c, " -Wall ");
	__C_APPEND(c, "-Wextra ");
	__C_APPEND(c, "-Werror ");
	__C_APPEND(c, "-pedantic ");
	
    if ((err = command_run(&c)))
    {
        return err;
    }

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
#ifdef DEBUG
    printf("Running command: %s\n", c->buff);
#endif

	int err = system(c->buff);
	if (err != 0)
	{
		return E_FAILTORUNCOMMAND;
	}

	return 0;
} 

#define __BUFFER_SIZE 4096

int __copy_file(const char *src, const char *dst)
{
#ifdef DEBUG
    printf("Copying file from %s to %s\n", src, dst);
#endif

    FILE *src_file = fopen(src, "r");
    if (!src_file)
    {
        return E_FAILTOOPENFILE;
    }

    FILE *dst_file = fopen(dst, "w");
    if (!dst_file)
    {
        return E_FAILTOOPENFILE;
    }

    char buffer[__BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, __BUFFER_SIZE, src_file)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dst_file);
    }

    fclose(src_file);
    fclose(dst_file);
    return 0;
}

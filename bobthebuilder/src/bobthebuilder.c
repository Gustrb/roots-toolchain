#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define __PATH_BUFF_MAX 4096


#define E_NOTENOUGHARGS    1
#define E_PATHTOOLONG      2
#define E_FAILTOFORMATPATH 3
#define E_FILENOTFOUND     4

typedef struct {
    const char *inputfolder;
    size_t inputfolder_len;
} bobthebuilder_cli_args_t;

typedef struct {
    const char *buildfile;
    size_t buildfile_len;
} bobthebuilder_buildfile_t;

static const char *__error_messages[] = {
    [E_NOTENOUGHARGS] = "Not enough arguments provided",
    [E_PATHTOOLONG] = "Path too long",
    [E_FAILTOFORMATPATH] = "Failed to format path",
    [E_FILENOTFOUND] = "buildfile not found at the root of the project",
};

int cli_parse(int argc, char **argv, bobthebuilder_cli_args_t *args);
int __figure_out_buildfile(bobthebuilder_cli_args_t *args, bobthebuilder_buildfile_t *buildfile);

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

#include "arguments.h"
#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_filename_argument(int argc, char **argv, char **filename)
{
    *filename = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") != 0)
            continue;

        if (i + 1 >= argc)
        {
            puts("Error: -f requires a filename argument");
            return -1;
        }

        *filename = argv[i + 1];
        return 0;
    }

    return 0;
}

int get_process_count(int argc, char **argv)
{
    int process_count = 4;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-N") != 0)
            continue;

        if (i + 1 >= argc)
        {
            fprintf(stderr, "Error: -N requires a numeric argument\n");
            return -1;
        }

        {
            char *end;
            long parsed_count;

            errno = 0;
            parsed_count = strtol(argv[i + 1], &end, 10);
            if (errno != 0 || end == argv[i + 1] || *end != '\0' ||
                parsed_count <= 0 || parsed_count > INT_MAX)
            {
                fprintf(stderr, "Error: Invalid number of processes '%s'\n", argv[i + 1]);
                return -1;
            }
            process_count = (int)parsed_count;
        }

    }

    if (process_count > MAX_CHILDREN)
    {
        process_count = MAX_CHILDREN;
        puts("Max process count exceeded, defaulting to max");
    }

    return process_count;
}

#include "url_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UrlList *get_urls_list(const char *urls_file_name)
{
    FILE *file = fopen(urls_file_name, "r");
    char **lines = NULL;
    char *current_line = NULL;
    size_t line_capacity = 0;
    int count = 0;
    UrlList *urls;

    if (file == NULL)
    {
        perror("Error opening URLs file");
        return NULL;
    }

    while (getline(&current_line, &line_capacity, file) != -1)
    {
        char **new_lines;
        int repeated = 0;

        current_line[strcspn(current_line, "\r\n")] = '\0';
        if (current_line[0] == '\0')
            continue;

        for (int i = 0; i < count; i++)
        {
            if (strcmp(current_line, lines[i]) == 0)
            {
                repeated = 1;
                break;
            }
        }

        if (repeated)
            continue;

        new_lines = realloc(lines, sizeof(*lines) * (size_t)(count + 1));
        if (new_lines == NULL)
        {
            perror("Realloc failed");
            break;
        }

        lines = new_lines;
        lines[count] = strdup(current_line);
        if (lines[count] == NULL)
        {
            perror("Not enough memory");
            break;
        }
        count++;
    }

    free(current_line);
    fclose(file);

    urls = malloc(sizeof(*urls));
    if (urls == NULL)
    {
        for (int i = 0; i < count; i++)
            free(lines[i]);
        free(lines);
        return NULL;
    }

    urls->list = lines;
    urls->list_size = count;
    return urls;
}

void delete_urls_list(UrlList *urls)
{
    if (urls == NULL)
        return;

    for (int i = 0; i < urls->list_size; i++)
        free(urls->list[i]);
    free(urls->list);
    free(urls);
}

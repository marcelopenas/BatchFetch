#pragma once

typedef struct
{
    char **list;
    int list_size;
} UrlList;

UrlList *get_urls_list(const char *urls_file_name);
void delete_urls_list(UrlList *urls);

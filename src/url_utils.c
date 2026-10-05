#include "url_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *get_filename_from_url(const char *url)
{
    const char *start;
    const char *last_dot;
    const char *last_slash;
    char extension[16] = "";
    size_t slug_source_len;
    size_t extension_len;
    char *slug;
    size_t j = 0;
    int last_was_underscore = 0;

    if (url == NULL)
        return NULL;

    start = strstr(url, "://");
    start = (start != NULL) ? start + 3 : url;
    last_dot = strrchr(start, '.');
    last_slash = strrchr(start, '/');

    if (last_dot != NULL && (last_slash == NULL || last_dot > last_slash) &&
        strlen(last_dot) < sizeof(extension))
    {
        memcpy(extension, last_dot, strlen(last_dot) + 1);
    }

    extension_len = strlen(extension);
    slug_source_len = (extension_len > 0) ? (size_t)(last_dot - start) : strlen(start);
    slug = malloc(slug_source_len + extension_len + 1);
    if (slug == NULL)
        return NULL;

    for (size_t i = 0; i < slug_source_len; i++)
    {
        unsigned char character = (unsigned char)start[i];

        if (isalnum(character))
        {
            slug[j++] = (char)tolower(character);
            last_was_underscore = 0;
        }
        else if (!last_was_underscore && j > 0)
        {
            slug[j++] = '_';
            last_was_underscore = 1;
        }
    }

    if (j > 0 && slug[j - 1] == '_')
        j--;

    memcpy(slug + j, extension, extension_len + 1);
    return slug;
}

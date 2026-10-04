#include "downloader.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *memory;
    size_t size;
} MemoryBuffer;

static size_t write_memory_callback(void *contents, size_t size, size_t count, void *user_data)
{
    MemoryBuffer *buffer = user_data;
    size_t bytes = size * count;
    char *new_memory = realloc(buffer->memory, buffer->size + bytes + 1);

    if (new_memory == NULL)
    {
        perror("not enough memory (realloc returned NULL)");
        return 0;
    }

    buffer->memory = new_memory;
    memcpy(buffer->memory + buffer->size, contents, bytes);
    buffer->size += bytes;
    buffer->memory[buffer->size] = '\0';
    return bytes;
}

int curl_download(const char *download_url, const char *output_file_name)
{
    CURL *curl = curl_easy_init();
    MemoryBuffer buffer = {malloc(1), 0};
    CURLcode result;

    if (curl == NULL || buffer.memory == NULL)
    {
        free(buffer.memory);
        fprintf(stderr, "Error: curl or download buffer not initialized\n");
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_USERAGENT,
                    "Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/81.0");
    curl_easy_setopt(curl, CURLOPT_URL, download_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (result == CURLE_OK)
    {
        FILE *file = fopen(output_file_name, "wb");
        if (file == NULL)
        {
            perror("Error opening file");
            free(buffer.memory);
            return CURLE_WRITE_ERROR;
        }

        if (fwrite(buffer.memory, 1, buffer.size, file) != buffer.size)
            result = CURLE_WRITE_ERROR;
        fclose(file);
    }

    free(buffer.memory);
    return result;
}

#include "arguments.h"
#include "config.h"
#include "downloader.h"
#include "signals.h"
#include "url_list.h"
#include "url_utils.h"

#include <curl/curl.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <bits/sigaction.h>

static int download_single_url(const char *url)
{
    char *output_file_name = get_filename_from_url(url);
    int result;

    if (output_file_name == NULL)
    {
        fprintf(stderr, "Error: could not determine output filename\n");
        return 1;
    }

    result = curl_download(url, output_file_name);
    free(output_file_name);
    if (result == 0)
        puts("Successfully downloaded");
    else
        printf("\033[31m[Failed]\033[0m %s (Error: %s)\n", url, curl_easy_strerror(result));
    return 0;
}

static int download_url_list(UrlList *urls, int process_count)
{
    bool all_ok = true;
    int downloaded_urls = 0;

    puts("Urls to download:");
    fputs("\033[34m", stdout);
    for (int i = 0; i < urls->list_size; i++)
        puts(urls->list[i]);
    fputs("\033[0m", stdout);
    printf("Starting download of %d urls with %d processes\n", urls->list_size, process_count);

    while (downloaded_urls < urls->list_size)
    {
        int remaining = urls->list_size - downloaded_urls;
        int to_launch = (remaining < process_count) ? remaining : process_count;
        int batch_size = 0;
        pid_t *pids = malloc(sizeof(*pids) * (size_t)to_launch);
        sigset_t mask;
        sigset_t old_mask;

        if (pids == NULL)
        {
            perror("Not enough memory");
            return 1;
        }

        printf("\033[35mLaunching batch of %d processes...\033[0m\n", to_launch);
        reset_child_tracking();
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigprocmask(SIG_BLOCK, &mask, &old_mask);

        for (int i = 0; i < to_launch; i++)
        {
            pid_t child_pid = fork();
            if (child_pid < 0)
            {
                perror("Fork failed");
                sigprocmask(SIG_SETMASK, &old_mask, NULL);
                free(pids);
                return 1;
            }

            if (child_pid == 0)
            {
                char *output_file_name;
                int result;

                register_sigint_handler(SIG_IGN);
                output_file_name = get_filename_from_url(urls->list[downloaded_urls + i]);
                if (output_file_name == NULL)
                    _exit(1);
                result = curl_download(urls->list[downloaded_urls + i], output_file_name);
                free(output_file_name);
                delete_urls_list(urls);
                free(pids);
                _exit(result);
            }

            child_pids[i] = child_pid;
            child_count++;
            pids[i] = child_pid;
            batch_size++;
            sigprocmask(SIG_SETMASK, &old_mask, NULL);
        }

        for (int i = 0; i < batch_size; i++)
        {
            int status;
            int result;
            int url_index = downloaded_urls + i;

            while (waitpid(pids[i], &status, 0) == -1 && errno == EINTR)
                ;

            result = WIFEXITED(status) ? WEXITSTATUS(status) : CURLE_ABORTED_BY_CALLBACK;
            if (WIFEXITED(status) && result == 0)
                printf("\033[32m[Success]\033[0m %s\n", urls->list[url_index]);
            else
            {
                all_ok = false;
                printf("\033[31m[Failed]\033[0m %s (Error: %s)\n",
                       urls->list[url_index], curl_easy_strerror(result));
            }
        }

        free(pids);
        downloaded_urls += batch_size;
    }

    puts(all_ok ? "All downloads finished successfully" : "Some downloads failed");
    return 0;
}

int main(int argc, char **argv)
{
    char *urls_file_name;

    if (argc < 2)
    {
        puts("Error: invalid usage, correct is:\n./web_downloader PARAMETERS");
        return 1;
    }

    if (get_filename_argument(argc, argv, &urls_file_name) == -1)
        return 1;

    register_sigint_handler(handle_sigint_parent);
    if (urls_file_name == NULL)
    {
        fputs("Downloading: \033[34m", stdout);
        puts(argv[1]);
        fputs("\033[0m", stdout);
        return download_single_url(argv[1]);
    }

    {
        int process_count = get_process_count(argc, argv);
        UrlList *urls;
        int result;

        if (process_count == -1)
            return 1;
        urls = get_urls_list(urls_file_name);
        if (urls == NULL)
            return 1;
        result = download_url_list(urls, process_count);
        delete_urls_list(urls);
        return result;
    }
}

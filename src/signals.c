#define _POSIX_C_SOURCE 200809L

#include "signals.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t child_count = 0;
volatile pid_t child_pids[MAX_CHILDREN];

void register_sigint_handler(void (*handler)(int))
{
    struct sigaction action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void reset_child_tracking(void)
{
    child_count = 0;
    for (int i = 0; i < MAX_CHILDREN; i++)
        child_pids[i] = 0;
}

void handle_sigint_parent(int signal_number)
{
    int saved_errno = errno;
    char choice = 0;
    (void)signal_number;

    for (int i = 0; i < child_count; i++)
        if (child_pids[i] != 0)
            kill(child_pids[i], SIGSTOP);

    {
        const char message[] = " Are you sure you want to exit? (y/n): ";
        if (write(STDOUT_FILENO, message, sizeof(message) - 1) < 0)
        {
            /* Continue handling the signal even if the prompt cannot be written. */
        }
    }

    while (read(STDIN_FILENO, &choice, 1) < 0 && errno == EINTR)
        ;

    if (choice == 'Y' || choice == 'y')
    {
        const char message[] = "Ctr+C confirmed, exiting!.\n";
        if (write(STDOUT_FILENO, message, sizeof(message) - 1) < 0)
            _exit(EXIT_FAILURE);
        for (int i = 0; i < child_count; i++)
            if (child_pids[i] != 0)
                kill(child_pids[i], SIGKILL);
        _exit(EXIT_SUCCESS);
    }

    {
        const char message[] = "Ctr+C cancelled, resuming.\n";
        if (write(STDOUT_FILENO, message, sizeof(message) - 1) < 0)
        {
            /* There is no async-signal-safe recovery action available here. */
        }
    }
    for (int i = 0; i < child_count; i++)
        if (child_pids[i] != 0)
            kill(child_pids[i], SIGCONT);

    errno = saved_errno;
}

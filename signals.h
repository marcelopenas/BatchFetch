#pragma once

#include <signal.h>
#include <sys/types.h>

#include "config.h"

extern volatile sig_atomic_t child_count;
extern volatile pid_t child_pids[MAX_CHILDREN];

void register_sigint_handler(void (*handler)(int));
void handle_sigint_parent(int signal_number);
void reset_child_tracking(void);

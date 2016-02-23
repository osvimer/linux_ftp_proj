#ifndef __PROCESS_POLL__
#define __PROCESS_POLL__
#include "head.h"

typedef struct process_poll{
    pid_t pid;
    int pfd;
    int busy;
}child_t, * pchild_t;

int make_child(pchild_t, int);

#endif

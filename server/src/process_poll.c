#include "process_poll.h"
#include "service.h"

int make_child(pchild_t cptr, int num){
    int i, ret, pid, fds[2];
    for(i = 0; i < num; i++){
        ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
        if(-1 == ret){
            perror("socketpair");
            return -1;
        }
        pid = fork();
        if(0 == pid){
            close(fds[1]);
            service_handle(fds[0]);
        }else{
            close(fds[0]);
            cptr[i].pid = pid;
            cptr[i].pfd = fds[1];
            cptr[i].busy = 0;
        }
    }
    return 0;
}

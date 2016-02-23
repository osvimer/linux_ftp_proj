#include "init.h"
#include "trans_file.h"

extern int sfd;

void signal_handle(int signum){
    data_t buf;
    strcpy(buf.data, "ABORT");
    send_complete(sfd, &buf);
    exit(0);
}

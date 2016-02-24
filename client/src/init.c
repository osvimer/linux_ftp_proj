#include "init.h"
#include "trans_file.h"

extern int sfd;
extern int connect_status;

void signal_handle(int signum){
    data_t buf;
    strcpy(buf.data, "ABORT");
    buf.len = strlen(buf.data);
    send_complete(sfd, &buf);
    connect_status = 0;
    close(STDIN_FILENO);
}

void log_time(char * logtime){
    time_t rtime;
    struct tm * ptm;
    time(&rtime);
    ptm = gmtime(&rtime);
    sprintf(logtime, "%d年%d月%d日 %02d:%02d:%02d  ", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour + 8, ptm->tm_min, ptm->tm_sec);
}

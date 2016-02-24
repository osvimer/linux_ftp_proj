#ifndef __INIT__
#define __INIT__
#include "head.h"

#define FILE_NEME_LEN 128

struct server_config{
    char ip[16];
    int port;
    int peer_max;
    int log_enable;
    int file_upload_max;
    int timeout;
    char root_dir[128];
    char log_file[128];
};

void set_daemon();
void log_time();
void signal_handle(int);
#endif

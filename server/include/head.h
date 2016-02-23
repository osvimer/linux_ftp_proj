#ifndef __HEAD__
#define __HEAD__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <crypt.h>
#include <shadow.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

typedef struct transfer_data{
    int len;
    char data[1000];
}data_t, * pdata_t;

#endif

/*
typedef struct transfer_status{
    int len;
    int status;
}status_t, * pstatus_t;

typedef struct transfer_cmd{
    int len;
    char cmd[8];
    int args_num;
    char args[128];
}cmd_t, * pcmd_t;

typedef union transfer_union{
    data_t data_mode;
    status_t status_mode;
    cmd_t cmd_mode;
}trans_t, * ptrans_t;
*/

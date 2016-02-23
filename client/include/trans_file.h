#ifndef __TRANS_FILE__
#define __TRANS_FILE__
#include "head.h"
//#include "thread_poll.h"

void send_status(int, int);
void recv_status(int, int *);
void send_complete(int, pdata_t);
void recv_complete(int, pdata_t);
void send_file_len(int, char *);
void recv_file_len(int, long *);
void send_file_by_fd(int, int);
void recv_file_by_fd(int, int, long);
void send_file_by_name(int, char *);
void recv_file_by_name(int, char *);
void multi_thread_send_file();
void muiti_thread_recv_file();
#endif

#include "cmd.h"
#include "trans_file.h"

void recv_list(int sfd){
    long file_len;
    long send_size = 0;
    data_t buf;
    //获取文件信息大小
    recv_file_len(sfd, &file_len);
    //接收文件信息
    while(fflush(stdout), send_size < file_len){
        recv_complete(sfd, &buf);
        printf("%s", buf.data);
        send_size += buf.len;
    }
}

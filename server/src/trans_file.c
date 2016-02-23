#include "trans_file.h"

void send_status(int sfd, int status){
    int msg = htonl(status);
    send(sfd, &msg, sizeof(int), 0);
}

void recv_status(int sfd, int * status){
    recv(sfd, status, sizeof(int), 0);
    * status = ntohl(* status);
}

void send_complete(int sfd, pdata_t buf){
    int size = 0;
    int total = 0;
    buf->len = strlen(buf->data);
    int len = buf->len + sizeof(int);
    buf->len = htonl(buf->len);
    while(total < len){
        size = send(sfd, buf + size, len - total, 0);
        total += size;
    }
}

void recv_complete(int sfd, pdata_t buf){
    int size = 0;
    int total = 0;
    bzero(buf, sizeof(data_t));
    recv(sfd, &buf->len, sizeof(int), 0);
    buf->len = ntohl(buf->len);
    int len = buf->len;
    while(total < len){
        size = recv(sfd, buf->data + total, buf->len - total, 0);
        total += size;
    }
}

void send_file_len(int sfd, char * file_name){
    struct stat file_stat;
    stat(file_name, &file_stat);
    data_t buf;
    //发送文件大小
    long file_len = file_stat.st_size;
    bzero(&buf, sizeof(buf));
    memcpy(&buf.data, &file_len, sizeof(long));
    send_complete(sfd, &buf);
}

void recv_file_len(int sfd, long * file_len){
    //获取文件大小
    data_t buf;
    recv_complete(sfd, &buf);
    memcpy(file_len, buf.data, sizeof(long));
}

void send_file_by_fd(int sfd, int fd){
    data_t buf;
    //发送文件给客户端
    while(bzero(&buf, sizeof(buf)), (buf.len = read(fd, buf.data, sizeof(buf.data))) > 0){
        send_complete(sfd, &buf);
    }
}

void recv_file_by_fd(int sfd, int fd, long file_len){
    data_t buf;
    long file_size = 0;
    //从客户端接收文件
    while(file_size < file_len){
        bzero(&buf, sizeof(buf));
        recv_complete(sfd, &buf);
        write(fd, buf.data, buf.len);
        file_size += buf.len;
    }
}

void send_file_by_name(int sfd, char * file_name){
    struct stat file_stat;
    int status = stat(file_name, &file_stat);
    int fd = open(file_name, O_RDONLY);
    long file_len;
    data_t buf;
    if(-1 == status || -1 == fd){
        send_status(sfd, -1);
        return;
    }
    send_status(sfd, 0);
    //发送文件大小
    file_len = file_stat.st_size;
    bzero(&buf, sizeof(buf));
    memcpy(&buf.data, &file_len, sizeof(long));
    send_complete(sfd, &buf);
    //发送文件内容
    while(bzero(&buf, sizeof(buf)), (buf.len = read(fd, buf.data, sizeof(buf.data))) > 0){
        send_complete(sfd, &buf);
    }
    close(fd);
}

void recv_file_by_name(int sfd, char * file_name){
    int fd;
    long int file_len, file_size;
    data_t buf;
    fd = open(file_name, O_WRONLY|O_CREAT, 0644);
    if(-1 == fd){
        send_status(sfd, -1);
        return;
    }
    send_status(sfd, 0);
    //获取文件大小
    bzero(&buf, sizeof(buf));
    recv_complete(sfd, &buf);
    memcpy(&file_len, buf.data, sizeof(long));
    file_size = 0;
    //接收文件内容
    while(file_size < file_len){
        bzero(&buf, sizeof(buf));
        recv_complete(sfd, &buf);
        write(fd, buf.data, buf.len);
        file_size += buf.len;
    }
    close(fd);
}

void recv_print(int sfd, long file_len){
    ;
}

void multi_thread_send_file(){
    ;
}

void multi_thread_recv_file(){
    ;
}

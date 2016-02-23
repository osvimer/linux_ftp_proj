#include "trans_fd.h"

int send_fd(int sfd, int task_fd){
    struct msghdr msg;
    bzero(&msg, sizeof(struct msghdr));
    char buf[2][10] = {"hello", "world"};
    struct iovec iov[2];
    iov[0].iov_base = buf[0];
    iov[0].iov_len = 5;
    iov[1].iov_base = buf[1];
    iov[1].iov_len = 5;
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    struct cmsghdr * cmsg;
    int len = CMSG_LEN(sizeof(int));
    cmsg = (struct cmsghdr *)calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    int * fdptr = (int *)CMSG_DATA(cmsg);
    * fdptr = task_fd;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;
    int ret = sendmsg(sfd, &msg, 0);
    if(-1 == ret){
        perror("sendmsg");
        return -1;
    }
    return 0;
}

int recv_fd(int sfd, int * task_fd){
    struct msghdr msg;
    bzero(&msg, sizeof(struct msghdr));
    char buf[2][10] = {0};
    struct iovec iov[2];
    iov[0].iov_base = buf[0];
    iov[0].iov_len = 5;
    iov[1].iov_base = buf[1];
    iov[1].iov_len = 5;
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    struct cmsghdr * cmsg;
    int len = CMSG_LEN(sizeof(int));
    cmsg = (struct cmsghdr *)calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;
    int ret = recvmsg(sfd, &msg, 0);
    if(-1 == ret){
        perror("recvmsg");
        return -1;
    }
    * task_fd = * (int *)CMSG_DATA(cmsg);
    return 0;
}

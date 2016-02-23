#include "list.h"

//遍历指定目录
int dir_scan(char * dir_name, int fd){
    char file_path[512];
    DIR * dir;
    dir = opendir(dir_name);
    if(NULL == dir){
        perror("opendir");
        return -1;
    }
    struct dirent * pdir;
    while(NULL != (pdir = readdir(dir))){
        if(strcmp(".", pdir->d_name) && strcmp("..", pdir->d_name)){
            bzero(file_path, sizeof(file_path));
            sprintf(file_path, "%s%s%s", dir_name, "/", pdir->d_name);
            file_ls(file_path, fd);
        }
    }
    closedir(dir);
    return 0;
}
//文件状态显示
int file_ls(char * file_path, int fd){
    int i;
    struct stat buf;
    char file_mode[11];
    char file_time[25];
    char file_name[256];
    char file_info[512];
    bzero(file_mode, sizeof(file_mode));
    bzero(file_time, sizeof(file_time));
    bzero(file_name, sizeof(file_name));
    bzero(file_info, sizeof(file_info));
    if(stat(file_path, &buf) == -1){
        perror("stat");
        return -1;
    }
    sscanf(file_path, "%*[^/]/%s", file_name);
    unsigned short pfm = buf.st_mode;
    //文件类型
    if (S_ISREG(buf.st_mode)){
        file_mode[0] = '-';
    }else if (S_ISDIR(buf.st_mode)){
        file_mode[0] = 'd';
    }else if (S_ISCHR(buf.st_mode)){
        file_mode[0] = 'c';
    }else if(S_ISBLK(buf.st_mode)){
        file_mode[0] = 'b';
    }else if(S_ISFIFO(buf.st_mode)){
        file_mode[0] = 'p';
    }else if(S_ISLNK(buf.st_mode)){
        file_mode[0] = 'l';
    }else if(S_ISSOCK(buf.st_mode)){
        file_mode[0] = 's';
    }
    //文件权限
    for(i = 0; i < 9; i++){
        if(pfm % 2 == 0){
            file_mode[9 - i] = '-';
        }else if(i % 3 == 0){
            file_mode[9 - i] = 'x';
        }else if(i % 3 == 1){
            file_mode[9 - i] = 'w';
        }else if(i % 3 == 2){
            file_mode[9 - i] = 'r';
        }
        pfm >>= 1;
    }
    //时间格式
    strcpy(file_time, ctime(&buf.st_mtime));
    char * pt = file_time + 4;
    *(pt + 12) = 0;
    sprintf(file_info, "%10s%2lu%8s%8s%6ld%13s %-20s\n", file_mode, buf.st_nlink, getpwuid(buf.st_uid)->pw_name, getgrgid(buf.st_gid)->gr_name, buf.st_size, pt, file_name);
    write(fd, file_info, strlen(file_info));
    return 0;
}

/*
int dir_scan_pro(char * dir_name){
    struct dirent **namelist;
    int n;
    n = scandir(dir_name, &namelist, NULL, alphasort);
    if(n < 0){
        perror("scandir");
    }else{
        while(n--){
            printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
}
*/

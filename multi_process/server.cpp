#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include "warp.h"
using namespace std;

void catch_child(int signum){
    while(waitpid(0, nullptr, WNOHANG) > 0){};
    return;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    int listenfd, clientfd;
    pid_t pid;

    //第1步: 创建服务端的socket
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /*
    服务端程序的端口释放后可能会处于TIME_WAIT状态，等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以再次使用。
    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    */
    //第2步: 把服务端用于通信的地址和端口绑定到socket上
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //任意ip地址
    //servaddr.sin_addr.s_addr = inet_addr("10.0.4.11"); // 指定ip地址。
    servaddr.sin_port = htons(atoi(argv[1]));
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //第3步: 把socket设置为监听模式
    Listen(listenfd, 5);

    //第4步: 接收客户端的连接
    while(true){
        int socklen = sizeof(struct sockaddr_in);
        struct sockaddr_in clientaddr;
        clientfd = Accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
        printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));
        pid = fork();
        if(pid < 0){
            perr_exit("accept error");
        }
        else if(pid == 0){
            close(listenfd);
            break;
        }
        else{
            //注册新号捕捉函数
            struct sigaction act;
            act.sa_handler = catch_child;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            int ret = sigaction(SIGCHLD, &act, nullptr);
            if(ret != 0){
                perr_exit("sigaction error");
            }
            close(clientfd);
            continue;
        }
    }

    //第5步: 与客户端通信, 接收客户端发过来的报文后, 回复
    char buffer[1024];
    if(pid == 0){
        while(true){
            memset(buffer, 0, sizeof(buffer));
            int iret = Recv(clientfd, buffer, sizeof(buffer), 0);
            if(iret == 0){
                cout << "End!" << endl;
                break;
            }
            cout << "Received: " << buffer << endl;
            
            for(int i=0; i<iret; i++)
                buffer[i] = toupper(buffer[i]);
            
            iret = Send(clientfd, buffer, strlen(buffer), 0);
            cout << "Send: " << buffer << endl;
        }
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
    close(clientfd);
}
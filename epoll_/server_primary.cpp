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
#include <poll.h>
#include <sys/epoll.h>
#include "warp.h"
using namespace std;

#define OPEN_MAX 1024

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    int i, nready, res;
    int listenfd, clientfd, sockfd;
    char buffer[1024], str[INET_ADDRSTRLEN];
    struct sockaddr_in servaddr, clientaddr;
    struct epoll_event tmp, ep[OPEN_MAX];
    int socklen = sizeof(struct sockaddr_in);

    //第1步: 创建服务端的socket
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    //设置端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //第2步: 把服务端用于通信的地址和端口绑定到socket上
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //任意ip地址
    servaddr.sin_port = htons(atoi(argv[1]));
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //第3步: 把socket设置为监听模式
    Listen(listenfd, 5);

    //第4步: 接收客户端的连接
    int epfd = epoll_create(OPEN_MAX);
    if(epfd == -1)
        perr_exit("epoll_create error");
    
    tmp.events = EPOLLIN;
    tmp.data.fd = listenfd;
    res = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &tmp);
    if(res == -1)
        perr_exit("eppol_ctl error");
    
    while(true){
        nready = epoll_wait(epfd, ep, OPEN_MAX, -1);
        if(nready < 0)
            perr_exit("epoll_wait error");

        for(i=0; i<nready; i++){
            if(!(ep[i].events & EPOLLIN))
                continue;
            if(ep[i].data.fd == listenfd){
                clientfd = Accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
                printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));

                tmp.events = EPOLLIN;
                tmp.data.fd = clientfd;
                res = epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &tmp);
                if(res == -1)
                    perr_exit("epoll_ctl error");
            }
            else{
                sockfd = ep[i].data.fd;
                memset(buffer, 0, sizeof(buffer));
                int iret = Recv(sockfd, buffer, sizeof(buffer), 0);
                if(iret == 0){
                    res = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, nullptr);
                    if(res == -1)
                        perr_exit("epoll_ctl error");
                    Close(sockfd);
                    cout << "End!" << endl;
                }
                else{
                    cout << "Received: " << buffer << endl;
                    for(int j=0; j<iret; j++)
                        buffer[j] = toupper(buffer[j]);
                    
                    iret = Send(sockfd, buffer, strlen(buffer), 0);
                    cout << "Send: " << buffer << endl;
                }
            }
        }
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
    Close(epfd);
}
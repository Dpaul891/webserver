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
#include "warp.h"
using namespace std;

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    int nready;
    int maxfd = 0;
    int listenfd, clientfd;
    char buffer[1024];
    struct sockaddr_in servaddr, clientaddr;
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
    fd_set rset, allset;
    maxfd = listenfd;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while(true){
        rset = allset;
        nready = select(maxfd+1, &rset, nullptr, nullptr, nullptr);
        if(nready < 0)
            perr_exit("select error");
        if(FD_ISSET(listenfd, &rset)){
            clientfd = Accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
            printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));
            FD_SET(clientfd, &allset);
            if(clientfd > maxfd)
                maxfd = clientfd;
            if(0 == --nready)
                continue;
        }

        for(int i=listenfd+1; i<=maxfd; i++){
            if(FD_ISSET(i, &rset)){
                memset(buffer, 0, sizeof(buffer));
                int iret = Recv(i, buffer, sizeof(buffer), 0);
                if(iret == 0){
                    Close(i);
                    FD_CLR(i, &allset);
                    cout << "End!" << endl;
                }
                else{
                    cout << "Received: " << buffer << endl;
                    for(int j=0; j<iret; j++)
                        buffer[j] = toupper(buffer[j]);
                    
                    iret = Send(i, buffer, strlen(buffer), 0);
                    cout << "Send: " << buffer << endl;
                }
            }
        } 
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
}
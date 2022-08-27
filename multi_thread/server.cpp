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
#include <pthread.h>
#include "warp.h"
using namespace std;

struct s_info {                     //定义一个结构体, 将地址结构跟cfd捆绑  
    struct sockaddr_in clientaddr;  
    int clientfd;  
}; 

void *do_work(void *arg){
    int n;
    struct s_info *ts = (struct s_info *)arg;
    char buffer[1024];
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int iret = Recv(ts->clientfd, buffer, sizeof(buffer), 0);
        if(iret == 0){
            cout << "End!" << endl;
            break;
        }
        cout << "Received: " << buffer << endl;
        
        for(int i=0; i<iret; i++)
            buffer[i] = toupper(buffer[i]);
        
        iret = Send(ts->clientfd, buffer, strlen(buffer), 0);
        cout << "Send: " << buffer << endl;
    }
    Close(ts->clientfd);
    return nullptr;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    struct sockaddr_in servaddr, clientaddr;
    pthread_t tid;
    struct s_info ts[256];
    int listenfd, clientfd;

    //第1步: 创建服务端的socket
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    //第2步: 把服务端用于通信的地址和端口绑定到socket上
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //任意ip地址
    //servaddr.sin_addr.s_addr = inet_addr("10.0.4.11"); // 指定ip地址。
    servaddr.sin_port = htons(atoi(argv[1]));
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //第3步: 把socket设置为监听模式
    Listen(listenfd, 5);

    //第4步: 接收客户端的连接
    int i = 0;
    while(true){
        int socklen = sizeof(struct sockaddr_in);
        clientfd = Accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
        printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));
        ts[i].clientaddr = clientaddr;
        ts[i].clientfd = clientfd;
        pthread_create(&tid, nullptr, do_work, (void *)&ts[i]);
        pthread_detach(tid);
        i++;
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
    return 0;
}
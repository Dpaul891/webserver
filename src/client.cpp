#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <typeinfo>
#include "warp.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 3){
        cout << "Using:./client ip port" << endl;
        cout << "Example:./client 127.0.0.1 5005" << endl;
        return -1;
    }

    // 第1步: 创建客户端socket
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // 第2步: 向服务器发起连接请求
    struct hostent *h = gethostbyname(argv[1]);
    if(!h){
        cout << "gethostbyname failed." << endl;
        close(sockfd);
        return -1;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char buffer[1024];
    // 第3步: 与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    for(int i=0; i<10; i++){
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "this");
        int iret = Send(sockfd, buffer, strlen(buffer), 0);
        cout << "Send: " << buffer << endl;

        memset(buffer, 0, sizeof(buffer));
        iret = Recv(sockfd, buffer, sizeof(buffer), 0);
        if(iret == 0){
            cout << "End!" << endl;
            break;
        }
        cout << "Received: " << buffer << endl;
    }

    // 第4步: 关闭socket, 释放资源
    close(sockfd);
    return 0;
}


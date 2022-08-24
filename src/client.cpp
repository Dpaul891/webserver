#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 3){
        cout << "Using:./client ip port" << endl;
        cout << "Example:./client 127.0.0.1 5005" << endl;
        return -1;
    }

    // 第1步: 创建客户端socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }

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

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
        perror("connect");
        close(sockfd);
        return -1;
    }

    char buffer[1024];
    // 第3步: 与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    for(int i=0; i<10; i++){
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "这是第%d条消息", i + 1);
        int iret = send(sockfd, buffer, strlen(buffer), 0);
        if(iret <= 0){
            perror("send");
            break;
        }
        cout << "已发送: " << buffer << endl;

        memset(buffer, 0, sizeof(buffer));
        iret = recv(sockfd, buffer, sizeof(buffer), 0);
        if(iret <= 0){
            perror("receive");
            break;
        }
        cout << "已接收: " << buffer << endl;
    }

    // 第4步: 关闭socket, 释放资源
    close(sockfd);
    return 0;
}


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
#define MAXLINE 10

int main(int argc, char *argv[])
{
    if(argc != 3){
        cout << "Using:./client ip port" << endl;
        cout << "Example:./client 127.0.0.1 5005" << endl;
        return -1;
    }

    char ch = 'a';
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    int i;

    // 第1步: 创建客户端socket
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // 第2步: 向服务器发起连接请求
    struct hostent *h = gethostbyname(argv[1]);
    if(!h){
        cout << "gethostbyname failed." << endl;
        close(sockfd);
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // 第3步: 与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    while(true){
        //aaaa\n
        for(i=0; i<MAXLINE/2; i++)
            buffer[i] = ch;
        buffer[i-1] = '\n';
        ch++;
        for(; i<MAXLINE; i++)
            buffer[i] = ch;
        buffer[i-1] = '\n';
        ch++;
        //aaaa\nbbbb\n
        int iret = Send(sockfd, buffer, sizeof(buffer), 0);
        sleep(3);
    }
    return 0;
}


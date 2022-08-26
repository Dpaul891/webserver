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
using namespace std;

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    //第1步: 创建服务端的socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket");
        return -1;
    }

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
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
        perror("bind");
        close(listenfd);
        return -1;
    }

    //第3步: 把socket设置为监听模式
    if(listen(listenfd, 5) != 0){
        perror("listen");
        close(listenfd);
        return -1;
    }

    //第4步: 接收客户端的连接
    int socklen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
    if(clientfd == -1){
        perror("accept");
        close(listenfd);
        return -1;
    }
    printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));

    //第5步: 与客户端通信, 接收客户端发过来的报文后, 回复OK

    char buffer[1024];
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int iret = recv(clientfd, buffer, sizeof(buffer), 0);
        if(iret <= 0){
            cout << "iret=" << iret << endl;
            break;
        }
        cout << "已接收: " << buffer << endl;
        
        for(int i=0; i<iret; i++)
            buffer[i] = toupper(buffer[i]);
        
        iret = send(clientfd, buffer, strlen(buffer), 0);
        if(iret <= 0){
            perror("send");
            break;
        }
        cout << "已发送: " << buffer << endl;
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
    close(clientfd);
}
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
#include "warp.h"
using namespace std;

#define OPEN_MAX 1024

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Using:./server port" << endl;
        cout << "Example:./server 5005" << endl;
        return -1;
    }

    int i, nready, maxi;
    int listenfd, clientfd;
    char buffer[1024], str[INET_ADDRSTRLEN]; /* #define INET_ADDRSTRLEN 16 */
    struct sockaddr_in servaddr, clientaddr;
    struct pollfd client[OPEN_MAX];
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
    client[0].fd = listenfd;
    client[0].events = POLLIN;
    for (i=1; i<OPEN_MAX; i++)
        client[i].fd = -1;
    maxi = 0;
    
    while(true){
        nready = poll(client, maxi+1, -1);
        if(nready < 0)
            perr_exit("poll error");
        if(client[0].revents & POLLIN){
            clientfd = Accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
            printf("客户端(%s)已连接\n", inet_ntoa(clientaddr.sin_addr));
            
            for(i=1; i<OPEN_MAX; i++){
                if (client[i].fd < 0) {                /* 找client[]中没有使用的位置 */
                    client[i].fd = clientfd;           /* 保存accept返回的文件描述符到client[]里 */
                    break;
                }
            }
            if(i == OPEN_MAX){
                cout << "Too many clients" << endl;
                exit(1);
            }
            client[i].events = POLLIN;

            if(i > maxi)
                maxi = i;
            if(0 == --nready)
                continue;
        }

        for(i=1; i<=maxi; i++){
            int sockfd = client[i].fd;
            if(sockfd < 0)
                continue;
            if(client[i].revents & POLLIN){
                memset(buffer, 0, sizeof(buffer));
                int iret = Recv(sockfd, buffer, sizeof(buffer), 0);
                if(iret == 0){
                    Close(sockfd);
                    client[i].fd = -1;
                    cout << "End!" << endl;
                }
                else{
                    cout << "Received: " << buffer << endl;
                    for(int j=0; j<iret; j++)
                        buffer[j] = toupper(buffer[j]);
                    
                    iret = Send(sockfd, buffer, strlen(buffer), 0);
                    cout << "Send: " << buffer << endl;
                }
                if(--nready == 0)
                    break;
            }
        }
    }

    //第6步: 关闭socket, 释放资源
    close(listenfd);
}
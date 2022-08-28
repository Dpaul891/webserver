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

    int i, nready, maxi = -1;
    int maxfd = 0, client[FD_SETSIZE]; /* 自定义数组client, 防止遍历1024个文件描述符  FD_SETSIZE默认为1024 */
    int listenfd, clientfd;
    char buffer[1024], str[INET_ADDRSTRLEN]; /* #define INET_ADDRSTRLEN 16 */
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

    for (i=0; i<FD_SETSIZE; i++)
        client[i] = -1;
    
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
            
            for(i=0; i<FD_SETSIZE; i++){
                if (client[i] < 0) {                /* 找client[]中没有使用的位置 */
                    client[i] = clientfd;           /* 保存accept返回的文件描述符到client[]里 */
                    break;
                }
            }
            if(i == FD_SETSIZE){
                cout << "Too many clients" << endl;
                exit(1);
            }

            FD_SET(clientfd, &allset);
            if(clientfd > maxfd)
                maxfd = clientfd;
            if(i > maxi)
                maxi = i;
            if(0 == --nready)
                continue;
        }

        for(i=0; i<=maxi; i++){
            int sockfd = client[i];
            if(sockfd < 0)
                continue;
            if(FD_ISSET(sockfd, &rset)){
                memset(buffer, 0, sizeof(buffer));
                int iret = Recv(sockfd, buffer, sizeof(buffer), 0);
                if(iret == 0){
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
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
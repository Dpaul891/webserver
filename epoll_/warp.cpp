#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include "warp.h"

void perr_exit(const char *s){
	perror(s);
	exit(-1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr){
	int n = accept(fd, sa, salenptr);
	if(n < 0)
		perr_exit("accept error");
	return n;
}

int Bind(int fd, const struct sockaddr *sa, socklen_t salen){
    int n = bind(fd, sa, salen);
	if(n < 0)
		perr_exit("bind error");
    return n;
}

int Connect(int fd, const struct sockaddr *sa, socklen_t salen){
    int n = connect(fd, sa, salen);
	if(n < 0)
		perr_exit("connect error");
    return n;
}

int Listen(int fd, int backlog){
    int n = listen(fd, backlog);
	if(n  < 0)
		perr_exit("listen error");
    return n;
}

int Socket(int family, int type, int protocol){
	int n = socket(family, type, protocol);
	if (n < 0)
		perr_exit("socket error");
	return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes){
	ssize_t n = read(fd, ptr, nbytes);
	if(n == -1)
		perr_exit("read error");
	return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes){
	ssize_t n = write(fd, ptr, nbytes);
	if(n == -1)
		perr_exit("write error");
	return n;
}

int Close(int fd){
    int n = close(fd);
	if(n == -1)
		perr_exit("close error");
    return n;
}

ssize_t Recv(int sockfd, void *buf, size_t len, int flags){
    int n = recv(sockfd, buf, len, flags);
    if(n == -1)
		perr_exit("recv error");
    return n;
}

ssize_t Send(int sockfd, const void *buf, size_t len, int flags){
    int n = send(sockfd, buf, len, flags);
    if(n == -1)
		perr_exit("send error");
    return n;
}

//socket 4096  readn(cfd, buf, 4096)   nleft = 4096-1500
ssize_t Readn(int fd, void *vptr, size_t n){
	size_t  nleft;              //usigned int 剩余未读取的字节数
	ssize_t nread;              //int 实际读到的字节数
	char   *ptr;

	ptr = (char *)vptr;
	nleft = n;                  //n 未读取字节数
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			return -1;
		} 
        else if (nread == 0)
			break;

		nleft -= nread;   //nleft = nleft - nread 
		ptr += nread;
	}
	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n){
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = (char *) vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

static ssize_t my_read(int fd, char *ptr){
	static int read_cnt;
	static char *read_ptr;
	static char read_buf[100];

	if (read_cnt <= 0) {
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			return -1;
		}
        else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char    c, *ptr;
	ptr = (char *)vptr;

	for (n = 1; n < maxlen; n++) {
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} 
        else if (rc == 0) {
			*ptr = 0;
			return n-1;
		} 
        else
			return -1;
	}
	*ptr = 0;
	return n;
}


## socket 编程记录

### 1. 为什么服务端才需要绑定，客户端不需要

- 如果不进行绑定，操作系统会随机生成一个端口号给服务器。如果操作系统给服务器分配这个端口号的同时，有其他程序也准备使用这个端口号或者说端口号已经被使用，则可能会导致服务器一直启动不起来。

- 客户端需要主动向服务器端发送请求，因此客户端就需要知道服务器端的IP地址和端口号，如果不绑定让系统随机生成，客户端将无法知道服务器端的端口号，即使知道也需要每次都去获取。
- 对于客户端来说，服务器端并不需要主动给客户端发送数据，客户端是主动的而服务器端是被动的。客户端给服务器端发送数据时，会将自己的IP地址和端口号一起发送过去，服务器端可以方便的找到客户端。

- 同时，客户端并不是一直运行的，只需要每次系统随机分配即可。

### 2. 一些函数

- `perror`

    ```c++
    #函数原型
    void perror(const char *s); 
    
    #举例:
    #include <stdio.h>
    int main(){
    	FILE* fp;
      	fp = fopen("testfile", "r+");
      	if ( NULL == fp ){
            perror("testfile");
      	}
      	return 0;
    }
    
    #输出
    testfile: No such file or directory
    testfile为perror函数中的字符串;后面所接的字符串就是调用这个函数出错时，该函数重新设置的errno的值。
    ```

- `socket`

    ```c++
    #头文件
    #include <sys/types.h>
    #include <sys/socket.h>
    
    #函数原型
    int socket(int domain, int type, int protocol);
    
    #举例:
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ```

    - 参数说明：

        - domain：协议域，又称协议族（family）。常用的协议族有AF_INET、AF_INET6、AF_LOCAL（或称AF_UNIX，Unix域Socket）、AF_ROUTE等。协议族决定了socket的地址类型，在通信中必须采用对应的地址，如AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合、AF_UNIX决定了要用一个绝对路径名作为地址。

        - type：指定socket类型。常用的socket类型有SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等。流式socket（SOCK_STREAM）是一种面向连接的socket，针对于面向连接的TCP服务应用。数据报式socket（SOCK_DGRAM）是一种无连接的socket，对应于无连接的UDP服务应用。

        - protocol：指定协议。常用协议有IPPROTO_TCP、IPPROTO_UDP、IPPROTO_STCP、IPPROTO_TIPC等，分别对应TCP传输协议、UDP传输协议、STCP传输协议、TIPC传输协议。**当protocol为0时，会自动选择type类型对应的默认协议。**

    - 返回值：
        - 成功则返回一个socket文件描述符，失败返回-1，错误原因存于errno 中。

- `bind`

    ```c++
    #头文件：
    #include <sys/types.h>
    #include <sys/socket.h>
    
    #函数原型：
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```

    - 参数说明：
        - 参数sockfd，需要绑定的socket。
        - 参数addr，存放了服务端用于通信的地址和端口。
        - 参数addrlen表示addr结构体的大小。
    - 返回值：成功则返回0，失败返回-1，错误原因存于errno 中。

- `listen`

    - listen函数把主动连接socket变为被动连接的socket，使得这个socket可以接受其它socket的连接请求，从而成为一个服务端的socket。

    ```c++
    #头文件：
    #include <sys/socket.h>
    int listen(int sockfd, int backlog);
    ```

    - 参数说明：

        - sockfd是已经被bind过的socket。
        - socket函数返回的socket是一个主动连接的socket，在服务端的编程中，程序员希望这个socket可以接受外来的连接请求，也就是被动等待客户端来连接。由于系统默认时认为一个socket是主动连接的，所以需要通过某种方式来告诉系统，程序员通过调用listen函数来完成这件事。
        -  backlog:指定在请求队列中的最大请求数，进入的连接请求将在队列中等待accept()它们。

        - 当调用listen之后，服务端的socket就可以调用accept来接受客户端的连接请求。

    - 返回值：成功则返回0，失败返回-1，错误原因存于errno 中。

- `accept`

    ```c++
    #头文件：   
    #include <sys/types.h>
    #inlcude <sys/socket.h>
    
     #函数原型：
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    ```

    - 参数说明：
        - sockfd是已经被listen过的socket。
        - addr用于存放客户端的地址信息，用sockaddr结构体表达，如果不需要客户端的地址，可以填0。
        - addrlen用于存放addr参数的长度，如果addr为0，addrlen也填0。

    - accept函数等待客户端的连接，如果没有客户端连上来，它就一直等待，这种方式称之为**阻塞**。从已准备好的连接队列中获取一个请求。

    - accept等待到客户端的连接后，创建一个新的socket，函数返回值就是这个新的socket，服务端使用这个新的socket和客户端进行报文的收发。

    - 返回值：失败返回-1，错误原因存于errno 中。

- `connect`

    ```c++
    #头文件
    #include <sys/types.h>
    #include <sys/socket.h>
    
    #函数原型
    int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
    
    #举例:
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))
    ```

    - 函数说明：connect函数用于将参数sockfd 的socket 连至参数serv_addr 指定的服务端，参数addrlen为sockaddr的结构体长度。

    - 返回值：成功则返回0，失败返回-1，错误原因存于errno 中。

    - connect函数只用于客户端。

- `send`

    ```c++
    #头文件：
    #include <sys/socket.h>
    #函数原型：
    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    ```

    - 参数说明：
        - sockfd为已建立好连接的socket。
        - buf为需要发送的数据的内存地址，可以是C语言基本数据类型变量的地址，也可以数组、结构体、字符串，内存中有什么就发送什么。
        - len需要发送的数据的长度，为buf中有效数据的长度。
        - flags填0, 其他数值意义不大。

    - 返回：函数返回已发送的字符数。出错时返回-1，错误信息errno被标记。

- `recv`

    ```c++
    #头文件：
    #include <sys/types.h>
    #include <sys/socket.h>
    #函数原型:
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    ```

    - 参数说明

        - sockfd为已建立好连接的socket。

        - buf为用于接收数据的内存地址，可以是C语言基本数据类型变量的地址，也可以数组、结构体、字符串，只要是一块内存就行了。
        - len缓冲的长度，不能超过buf的大小，否则内存溢出。

        - flags填0, 其他数值意义不大。

    - 返回：函数返回已接收的字符数。出错时返回-1，失败时不会设置errno的值。

    - 如果socket的对端没有发送数据，recv函数就会等待，如果对端发送了数据，函数返回接收到的字符数。出错时返回-1。如果socket被对端关闭，返回值为0。

- `gethostbyname`

    - 把**ip地址或域名**转换为hostent 结构体表达的地址。

    ```c++
    #头文件
    #include<netdb.h>
    #include<sys/socket.h>
    
    struct hostent *gethostbyname(const char *name);
    
    struct  hostent {
            char    FAR * h_name;           /* official name of host */
            char    FAR * FAR * h_aliases;  /* alias list */
            short   h_addrtype;             /* host address type: IPV4 AF_INET*/
            short   h_length;               /* length of address */
            char    FAR * FAR * h_addr_list; /* list of addresses */
    #define h_addr  h_addr_list[0]          /* address, for backward compat */
    };
    
    #近指针和远指针：near指针的长度是16位的，所以可指向的地址范围是64K字节，通常说near指针的寻址范围是64K。far指针的长度是32位，含有一个16位的基地址和16位的偏移量
    ```

- 关于`htons`和`htonl`

    - https://www.cnblogs.com/sddai/p/5790479.html

    - 在Linux和Windows网络编程时需要用到htons和htonl函数，用来将主机字节顺序转换为网络字节顺序。

    -   数字16的16进制表示为0x0010，数字4096的16进制表示为0x1000。 由于Intel机器是小尾端，存储数字16时实际顺序为1000，存储4096时实际顺序为0010。因此在发送网络包时为了报文中数据为0010，需要经过htons进行字节转换。如果用IBM等大尾端机器，则没有这种字节顺序转换，但为了程序的可移植性，也最好用这个函数。

        **另外用注意，数字所占位数小于或等于一个字节（8 bits）时，不要用htons转换。这是因为对于主机来说，大小尾端的最小单位为字节(byte)。**

    ```c++
    #头文件：
    #include <arpa/inet.h>
    uint32_t htonl(uint32_t hostlong);
    #htons的：将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)
    uint16_t htons(uint16_t hostshort);
    
    #htonl: 将主机的无符号长整形数转换成网络字节顺序
    uint32_t htonl(uint32_t hostlong);
    ```

### 3. 其他

- `sockaddr_in`和`sockaddr`

    ```c++
    #头文件
    #include <sys/socket.h>
    
    struct sockaddr {
        unsigned short    sa_family;    // 2 bytes address family, AF_xxx
        char              sa_data[14];     // 14 bytes of protocol address
    };
     
     
    struct sockaddr_in {
        short            sin_family;       // 2 bytes e.g. AF_INET, AF_INET6
        unsigned short   sin_port;    // 2 bytes e.g. htons(3490)
        struct in_addr   sin_addr;     // 4 bytes see struct in_addr, below
        char             sin_zero[8];     // 8 bytes zero this if you want to
    };
     
    struct in_addr {
        unsigned long s_addr;          // 4 bytes load with inet_pton()
    };
    ```

    - sockaddr和sockaddr_in包含的数据都是一样的，但他们在使用上有区别：
        - 程序员不应操作sockaddr，sockaddr是给操作系统用的-
        - 程序员应使用sockaddr_in来表示地址，sockaddr_in区分了地址和端口，使用更方便。
        - 程序员把类型、ip地址、端口填充sockaddr_in结构体，然后强制转换成sockaddr，作为参数传递给系统调用函数

- `inet_ntoa`和`inet_aton`; `inet_ntop`和`inet_pton`
    - inet_aton、inet_ntoa、inet_pton和inet_ntop都是用于IP地址与大端网络字节序二进制数字相互转换的函数,不同的是inet_aton和inet_ntoa只支持ipv4类型的地址转换，而inet_pton和inet_ntop支持ipv4和ipv6类型的地址转换

    - `inet_ntoa`将一个用in参数所表示的Internet地址结构转换成以“.” 间隔的诸如“a.b.c.d”的字符串形式。
    - `inet_aton`将形如“192.168.1.1"类型的点分十进制ip转换成二进制，并存放在struct in_addr中

    ```c++
    #函数原型
    #include <sys/socket.h>	// 这个好像在这里可以省略
    #include <netinet/in.h>	// 提供struct in_addr
    #include <arpa/inet.h>	// 提供inet_ntoa()
    char *inet_ntoa(struct in_addr in);

    int inet_aton(const char *cp, struct in_addr *inp);

    #include <arpa/inet.h>
	int inet_pton(int af, const char *src, void *dst);
	const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
    #af：AF_INET、AF_INET6
	#src：传入，IP地址（点分十进制）
	#dst：传出，转换后的 网络字节序的 IP地址。 

    ```
- `inet_addr`

    ```c++
    #inet_addr的功能是将一个ip地址字符串转换成一个整数值。 和inet_aton功能类似
    ```
- `INADDR_ANY` 

    ```c++
    #原型，即表示0.0.0.0转换成长整型
    #define INADDR_ANY  ((in_addr_t) 0x00000000) 

    80      0.0.0.0          //INADDR_ANY,外部的client ask 从哪个server的地址进来都可以连接到80端口.
    8088    192.168.1.11     //外部的client ask 从server地址192.168.1.11进来才可以连接到8088端口.
    8089    192.168.1.12     //外部的client ask 从server地址192.168.1.12进来才可以连接到8089端口.
    ```
- 在`listen`函数之后，客户端就可以连接了，并不是等到`accept`才可以连接
- listen的socket队列

    - 内核会为listen状态的socket维护两个队列：不完全连接请求队列(SYN_RECV状态)和等待accept建立socket的队列(ESTABLISHED状态)；
    - Linux2.2之后，backlog值为等待accept建立socket的队列长度
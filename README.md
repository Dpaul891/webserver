# C++ socket网络通信编程

## 背景
- socket提供了流（stream）和数据报（datagram）两种通信机制。
- 流socket基于TCP协议，是一个有序、可靠、双向字节流的通道，传输数据不会丢失、不会重复、顺序也不会错乱。
- 数据报socket基于UDP协议，不需要建立和维持连接，可能会丢失或错乱。UDP不是一个可靠的协议，对数据的长度有限制，但是它的速度比较高。
## 客户/服务端模式
- 编程步骤

    |  服务端   | 客户端  |
    |  ----  | ----  |
    | 1. 创建服务端的socket，这个是用来做监听的socket | 1. 创建客户端的socket |
    | 2. 把服务端用于通信的地址和端口绑定到socket上  | 2. 向服务器发起连接请求 |
    | 3. 把socket设置为监听模式  | 3. 与服务端通信，发送一个报文后等待回复，然后再发下一个报文 |
    | 4. 接受客户端的连接，返回一个连接socket  | 4. 不断的重复第3步，直到全部的数据被发送完 |
    | 5. 与客户端通信，接收客户端发过来的报文后，回复处理结果  | 5. 关闭socket，释放资源 |
    | 6. 不断的重复第5步，直到客户端断开连接 |  |
    | 7. 关闭socket，释放资源 |  |

    





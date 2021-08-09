/* $Id: socket.c 1.1 1995/01/01 07:11:14 cthuang Exp $
 *
 * This module has been modified by Radim Kolar for OS/2 emx
 */

/***********************************************************************
  module:       socket.c
  program:      popclient
  SCCS ID:      @(#)socket.c    1.5  4/1/94
  programmer:   Virginia Tech Computing Center
  compiler:     DEC RISC C compiler (Ultrix 4.1)
  environment:  DEC Ultrix 4.3 
  description:  UNIX sockets code.
 ***********************************************************************/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/*建立TCP连接
*host:地址，可以时域名或主机名
*clientPort:客户端端口
*/
int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
    //初始化地址
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    //将主机名（IP）转换为数字，出错返回INADDR_NONE
    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else //若host不是主机名，是域名
    {
        hp = gethostbyname(host); //这里通过域名获取IP地址
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort); //转换为网络字节序模式（大端模式）
    /*
     * int socket(int domain, int type, int protocol);
     * domain：即协议域，又称为协议族（family）。常用的协议族有，AF_INET、AF_INET6、AF_LOCAL（或称AF_UNIX，Unix域socket）、AF_ROUTE等等
     * type：指定socket类型。常用的socket类型有，SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等等
     * protocol：故名思意，就是指定协议。常用的协议有，IPPROTO_TCP、IPPTOTO_UDP、IPPROTO_SCTP、IPPROTO_TIPC等
     */
    sock = socket(AF_INET, SOCK_STREAM, 0); //创建一个socket,使用TCP协议
    if (sock < 0) //创建失败
        return sock;
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0) //进行连接，并判断连接是否成功
        return -1;
    return sock;
}


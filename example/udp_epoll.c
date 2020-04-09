//
//
// Test iperf UDP proxy application 
//
// Acts as a iperf server to a iperf client and as a iperf client to a iperf server
//
// Run iperf client and server on another host (worker2)
//
// iperf-client--->VF-N3--->VPP--->memif0-->proxy0-->BSD    
//                                                    | 
//                                              iperf-server-proxy
//                                                    |
//                                              iperf-client-proxy
//                                                    |         
//                                                    v         
// iperf-server<--VF-N6<---VPP<---memif0<---proxy0<--BSD    // high-bw data tx/rx over memif-0
//    |
//    +------->VF-N6/N3---->VPP-->memif0--->proxy0-->BSD    // low-bw iperf ack tx/rx  over memif-0
//                                                    | 
//                                              iperf-client-proxy
//                                                    |
//                                              iperf-server-proxy
//                                                    |         
//                                                    v         
// iperf-client<---VF-N3<---VPP<---memif0<--proxy0<--BSD
//
//


#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#define    IP_BINDANY              24
// #include "in.h" IP_BINDANY 24
#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"

#define  IP_ADDR_MINE 0x03030303


#define MAX_EVENTS 4

struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
int sockfd;
int sockfd1;

int portno=5201;
int portno1=15201;
//struct linux_sockaddr serveraddr;
//struct linux_sockaddr clientaddr;
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;

unsigned int serverip=0x05050505;
unsigned int clientip=0x02020202;
unsigned int iperfserverip=0x0a0a0a0a;

int loop(void *arg)
{
    /* Wait for events to happen */

    int nevents = ff_epoll_wait(epfd,  events, MAX_EVENTS, 0);
    int i;
    size_t txlen, rxlen;
    int clientlen, serverlen;

    for (i = 0; i < nevents; ++i) {
        // error handling to do
        if (events[i].events & EPOLLIN) {
            char buf[2000];
            if (events[i].data.fd == sockfd) {
                // packets from the client, learn src IP and src port
                // one iperf client only for testing
                rxlen = ff_recvfrom(events[i].data.fd, buf, sizeof(buf), 0, &clientaddr, &clientlen);
                if (rxlen > 0) {
                    serverlen = sizeof(serveraddr);

                    txlen = ff_sendto(sockfd1, buf, rxlen, 0, &serveraddr, serverlen);
                    if (txlen < 0) {
                        printf("ERROR in sendto the server");
                    }
                }
            } 
            // ack from the iperf server
            if (events[i].data.fd == sockfd1) {
                rxlen = ff_recvfrom(events[i].data.fd, buf, sizeof(buf), 0, NULL, NULL);
                if (rxlen > 0) {
                    clientlen = sizeof(clientaddr);
                    
                    txlen = ff_sendto(sockfd, buf, rxlen, 0, &clientaddr, clientlen);
                    if (txlen < 0) {
                        printf("ERROR in sendto the client");
                    }
                }
            }
        }
    }
}

int main(int argc, char * argv[])
{
    ff_init(argc, argv);

    sockfd = ff_socket(AF_INET, SOCK_DGRAM, 0);
    printf("sockfd:%d\n", sockfd);

    sockfd1 = ff_socket(AF_INET, SOCK_DGRAM, 0);
    printf("sockfd1:%d\n", sockfd1);
    if ((sockfd < 0) || (sockfd1 < 0)) {
        printf("ff_socket failed\n");
        exit(1);
    }

    // initialize udp packets from client
    // server address is ANY?
    // proxy the request to another server
    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    //struct linux_sockaddr my_addr;
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portno);
    my_addr.sin_addr.s_addr = htonl(iperfserverip);
    //my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int bindany=1;
    ff_setsockopt(sockfd, IPPROTO_IP, IP_BINDANY, &bindany, sizeof(int));

    int ret = ff_bind(sockfd, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0) {
        printf("ff_bind failed\n");
        exit(1);
    }

    // initialize to server
    on = 1;
    ff_ioctl(sockfd1, FIONBIO, &on);

    bindany=1;
    ff_setsockopt(sockfd1, IPPROTO_IP, IP_BINDANY, &bindany, sizeof(int));

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portno1);
    //my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_addr.s_addr = htonl(iperfserverip);
    ret = ff_bind(sockfd1, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0) {
        printf("ff_bind failed\n");
        exit(1);
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr= serverip;
    serveraddr.sin_port = htons(portno);

    bzero((char *) &clientaddr, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr= clientip;

    assert((epfd = ff_epoll_create(0)) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    ev.data.fd = sockfd1;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd1, &ev);

    ff_run(loop, NULL);
    return 0;
}

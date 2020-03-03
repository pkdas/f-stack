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

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"


#define MAX_EVENTS 512

struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
int sockfd;
char buf[2000];
unsigned int dst_ip_addr;

#if 0
int receiver_loop(void *arg)
{
    int nevents = ff_epoll_wait(epfd,  events, MAX_EVENTS, 0);
    int i;

    for (i = 0; i < nevents; ++i) {
        // we have one sockfd - UDP socket only
        if (events[i].events & EPOLLIN) {
            size_t readlen = ff_read( events[i].data.fd, buf, sizeof(buf));
            ff_write( events[i].data.fd, buf, readlen);
        } else { 
            printf("unknown event: %8.8X\n", events[i].events);
        }
    }
}

int sender(unsigned int dst_ip) 
{
    
    sockfd = ff_socket(AF_INET, SOCK_DGRAM, 0);
    printf("sockfd:%d\n", sockfd);
    if (sockfd < 0) {
        printf("ff_socket failed\n");
        exit(1);
    }

    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(5000);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = ff_bind(sockfd, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0) {
        printf("ff_bind failed\n");
        exit(1);
    }

    assert((epfd = ff_epoll_create(0)) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    ff_run(loop, NULL);
    return 0;
}

int receiver() 
{
    sockfd = ff_socket(AF_INET, SOCK_DGRAM, 0);
    printf("sockfd:%d\n", sockfd);
    if (sockfd < 0) {
        printf("ff_socket failed\n");
        exit(1);
    }

    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(5000);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = ff_bind(sockfd, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0) {
        printf("ff_bind failed\n");
        exit(1);
    }

    assert((epfd = ff_epoll_create(0)) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    ff_run(receiver_loop, NULL);
    return 0;
}

#endif
int main(int argc, char * argv[])
{
    char c;
    char str[INET_ADDRSTRLEN];

    ff_init(argc, argv);

    printf("Enter s to send udp echo or r to receive/reply: ");

    c=getchar();
    if (c == 's') {
        sscanf("Enter receiver IP address: %s", str);
        inet_pton(AF_INET, str, &dst_ip_addr);
        //sender(dst_ip_addr);
    } else  
    if (c == 'r') {
        //receiver();
    } else {
        printf("Unknown input! Enter s to send udp echo or r to receive/reply: ");
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/time.h>

int get_send_buf_size(int fd);
int get_recv_buf_size(int fd);


int main() {
    int tcp, udp, sctp;

    tcp = socket(AF_INET, SOCK_STREAM, 0);
    udp = socket(AF_INET, SOCK_DGRAM, 0);
    sctp = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

    printf("TCP send buffer size: %d\n", get_send_buf_size(tcp));
    printf("TCP recv buffer size: %d\n", get_recv_buf_size(tcp));
    printf("UDP send buffer size: %d\n", get_send_buf_size(udp));
    printf("UDP recv buffer size: %d\n", get_recv_buf_size(udp));
    printf("SCTP send buffer size: %d\n", get_send_buf_size(sctp));
    printf("SCTP recv buffer size: %d\n", get_recv_buf_size(sctp));

    close(tcp);
    close(udp);
    close(sctp);

}

int get_send_buf_size(int fd) {
    int buf_size, len;

    len = sizeof(buf_size);
    if(getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, &len) == -1){
        return -1;
    } 
    
    return buf_size;
}

int get_recv_buf_size(int fd){
    int buf_size, len;

    len = sizeof(buf_size);
    if(getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, &len) == -1){
        return -1;
    } 
    
    return buf_size;
}
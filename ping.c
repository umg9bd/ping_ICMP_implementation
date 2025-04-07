#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <time.h>

#define PACKET_SIZE     64
#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY   0
#define PING_SLEEP_RATE   1

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void build_packet(struct icmphdr *icmp, int pid, int seq) {
    icmp->type = ICMP_ECHO_REQUEST;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = pid;
    icmp->un.echo.sequence = seq;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));
}

void ping(const char *ip_address) {
    struct sockaddr_in dest_addr;
    struct icmphdr icmp_header;
    int sockfd, seq = 1;
    struct timespec start, end;
    long rtt;

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(ip_address);

    int pid = getpid();

    while (1) {
        build_packet(&icmp_header, pid, seq);

        clock_gettime(CLOCK_MONOTONIC, &start);
        if (sendto(sockfd, &icmp_header, sizeof(struct icmphdr), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) <= 0) {
            perror("Send failed");
            close(sockfd);
            return;
        }

        char buffer[1024];
        socklen_t addr_len = sizeof(dest_addr);
        ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, &addr_len);
        if (bytes_received <= 0) {
            perror("Receive failed");
            close(sockfd);
            return;
        }

        struct iphdr *ip_header = (struct iphdr *)buffer;
        struct icmphdr *icmp_reply = (struct icmphdr *)(buffer + (ip_header->ihl * 4));

        if (icmp_reply->type == ICMP_ECHO_REPLY && icmp_reply->un.echo.id == pid) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            rtt = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
            printf("Reply from %s: seq=%d time=%ld ms\n", ip_address, seq, rtt);
        }

        seq++;
        sleep(PING_SLEEP_RATE);
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: sudo ./ping <ip_address>\n");
        return 1;
    }

    printf("Pinging %s with ICMP Echo Request...\n", argv[1]);
    ping(argv[1]);

    return 0;
}
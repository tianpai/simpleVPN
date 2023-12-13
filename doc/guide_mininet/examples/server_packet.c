#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 65536

void print_packet(const unsigned char *buffer, int size) {
    struct iphdr *ip_header = (struct iphdr *)buffer;
    struct tcphdr *tcp_header = (struct tcphdr *)(buffer + ip_header->ihl * 4);

    if (ip_header->protocol == IPPROTO_TCP) {
        printf("Received TCP Packet:\n");
        printf("   Source IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
        printf("   Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));
        printf("   Source Port: %d\n", ntohs(tcp_header->source));
        printf("   Destination Port: %d\n", ntohs(tcp_header->dest));
        printf("\n");
    }
}

int main() {
    int raw_socket;
    ssize_t data_size;
    struct sockaddr_in saddr;
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    // Create a raw socket that captures all packets
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_socket < 0) {
        perror("Socket Error");
        return 1;
    }

    while (1) {
        // Receive packets
        data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (data_size < 0) {
            perror("Recvfrom Error");
            return 1;
        }

        // Process and print the packet
        print_packet(buffer, data_size);
    }

    close(raw_socket);
    free(buffer);
    return 0;
}
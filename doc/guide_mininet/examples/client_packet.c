#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

// Generic checksum calculation function
unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *) &oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}

int main() {
    // Define the payload
    char *data = "Hello, this is the payload!";
    int data_len = strlen(data);

    // Create a buffer for the packet
    char packet[1024];

    // Zero out the packet buffer
    memset(packet, 0, 1024);

    // IP header
    struct iphdr *ip_header = (struct iphdr *) packet;
    // TCP header
    struct tcphdr *tcp_header = (struct tcphdr *) (packet + sizeof(struct iphdr));
    // Pseudo TCP header
    struct pseudo_header psh;

    // Fill in the IP Header
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + data_len;
    ip_header->id = htons(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_TCP;
    ip_header->check = 0; // Set to 0 before calculating checksum
    ip_header->saddr = inet_addr("10.0.0.1"); // Source IP
    ip_header->daddr = inet_addr("10.0.0.2"); // Destination IP
    ip_header->check = calculate_checksum((unsigned short *) packet, ip_header->tot_len);

    // Fill in the TCP Header
    tcp_header->source = htons(1234); // Source Port
    tcp_header->dest = htons(5432); // Destination Port
    tcp_header->seq = htonl(1);
    tcp_header->ack_seq = 0;
    tcp_header->doff = 5; // TCP header size
    tcp_header->fin = 0;
    tcp_header->syn = 1; // Set the SYN flag
    tcp_header->rst = 0;
    tcp_header->psh = 0;
    tcp_header->ack = 0;
    tcp_header->urg = 0;
    tcp_header->window = htons(5840); // Maximum allowed window size
    tcp_header->check = 0; // Set to 0 before calculating checksum
    tcp_header->urg_ptr = 0;

    // Add the payload data
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct tcphdr), data, data_len);

    // Calculate the TCP checksum including the payload
    psh.source_address = inet_addr("10.0.0.1");
    psh.dest_address = inet_addr("10.0.0.2");
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + data_len);

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + data_len;
    char *pseudogram = malloc(psize);
    memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcp_header, sizeof(struct tcphdr) + data_len);

    tcp_header->check = calculate_checksum((unsigned short *) pseudogram, psize);
    free(pseudogram);

    // Now the packet is ready to be sent using raw sockets
    // ...


// Create a raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options, if necessary
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Destination info
    struct sockaddr_in dest_info;
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr.s_addr = inet_addr("10.0.0.2"); // Replace with destination IP

    // Your packet data
    // uint8_t *packet; // Assume this points to your prepared packet
    size_t packet_length; // The length of your packet
    packet_length = 1023;

    // Send the packet
    ssize_t bytes_sent = sendto(sockfd, packet, packet_length, 0, 
                                (struct sockaddr *)&dest_info, sizeof(dest_info));
    if (bytes_sent < 0) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

    // Close the socket
    close(sockfd);

    return 0;
}

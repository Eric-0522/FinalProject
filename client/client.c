// client/client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define PORT 8080

void query_availability(int sockfd);
void book_tickets(int sockfd, int num_tickets, int user_id);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <query|book> [num_tickets]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in serv_addr;

    // 1. Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // 2. Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // 3. Perform action based on arguments
    if (strcmp(argv[1], "query") == 0) {
        query_availability(sockfd);
    } else if (strcmp(argv[1], "book") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s book <num_tickets>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        int num_tickets = atoi(argv[2]);
        if (num_tickets <= 0) {
            fprintf(stderr, "Number of tickets must be a positive integer.\n");
            exit(EXIT_FAILURE);
        }
        // Use a random user ID for simulation
        srand(time(NULL));
        int user_id = rand() % 10000;
        book_tickets(sockfd, num_tickets, user_id);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
    }

    close(sockfd);
    return 0;
}

void query_availability(int sockfd) {
    static uint16_t req_id_counter = 0;

    // 1. Prepare and send request header
    ProtocolHeader req_header = {
        .packet_len = sizeof(ProtocolHeader),
        .opcode = OP_QUERY_AVAILABILITY,
        .req_id = req_id_counter++
    };
    if (write_n_bytes(sockfd, &req_header, sizeof(ProtocolHeader)) <= 0) {
        perror("Failed to send query request");
        return;
    }

    printf("Sent query request (req_id=%u).\n", req_header.req_id);

    // 2. Read response
    ProtocolHeader res_header;
    if (read_n_bytes(sockfd, &res_header, sizeof(ProtocolHeader)) <= 0) {
        perror("Failed to read response header");
        return;
    }

    ServerResponse res_body;
    if (read_n_bytes(sockfd, &res_body, sizeof(ServerResponse)) <= 0) {
        perror("Failed to read response body");
        return;
    }

    // 3. Print result
    printf("----------------------------------------\n");
    printf("Server Response (req_id=%u):\n", res_header.req_id);
    printf("  OpCode: 0x%X\n", res_header.opcode);
    printf("  Remaining Tickets: %u\n", res_body.remaining_tickets);
    printf("  Message: %s\n", res_body.message);
    printf("----------------------------------------\n");
}

void book_tickets(int sockfd, int num_tickets, int user_id) {
    static uint16_t req_id_counter = 0;

    // 1. Prepare request header and body
    ProtocolHeader req_header = {
        .packet_len = sizeof(ProtocolHeader) + sizeof(BookRequest),
        .opcode = OP_BOOK_TICKET,
        .req_id = req_id_counter++
    };
    BookRequest req_body = {
        .num_tickets = num_tickets,
        .user_id = user_id
    };

    // 2. Send request
    if (write_n_bytes(sockfd, &req_header, sizeof(ProtocolHeader)) <= 0) {
        perror("Failed to send booking request header");
        return;
    }
    if (write_n_bytes(sockfd, &req_body, sizeof(BookRequest)) <= 0) {
        perror("Failed to send booking request body");
        return;
    }
    printf("Sent book request for %d tickets (user_id=%d, req_id=%u).\n", num_tickets, user_id, req_header.req_id);

    // 3. Read response
    ProtocolHeader res_header;
    if (read_n_bytes(sockfd, &res_header, sizeof(ProtocolHeader)) <= 0) {
        perror("Failed to read response header");
        return;
    }

    ServerResponse res_body;
    if (read_n_bytes(sockfd, &res_body, sizeof(ServerResponse)) <= 0) {
        perror("Failed to read response body");
        return;
    }

    // 4. Print result
    printf("----------------------------------------\n");
    printf("Server Response (req_id=%u):\n", res_header.req_id);
    if (res_header.opcode == OP_RESPONSE_SUCCESS) {
        printf("  Status: SUCCESS\n");
    } else {
        printf("  Status: FAIL\n");
    }
    printf("  Remaining Tickets: %u\n", res_body.remaining_tickets);
    printf("  Message: %s\n", res_body.message);
    printf("----------------------------------------\n");
}

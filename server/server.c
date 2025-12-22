// server/server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> // For potential future threading

#include "common.h"

#define PORT 8080
#define MAX_PENDING_CONNECTIONS 5

// Global state for simplicity
int total_tickets = 100;
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_connection(int client_socket);

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 1. Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Optional: Allow reusing address/port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for connections
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("Initial tickets: %d\n", total_tickets);

    // 4. Accept connections in a loop
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("accept failed");
            continue; // Continue to next iteration
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Connection accepted from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // Handle the connection (for this simple server, we handle it sequentially)
        handle_connection(client_socket);
    }

    close(server_fd);
    return 0;
}

void handle_connection(int client_socket) {
    ProtocolHeader header;
    int read_ret;

    // Loop to handle multiple requests from the same client
    while ((read_ret = read_n_bytes(client_socket, &header, sizeof(ProtocolHeader))) > 0) {
        printf("Received request: packet_len=%u, opcode=%u, req_id=%u\n",
               header.packet_len, header.opcode, header.req_id);

        ServerResponse response;
        memset(&response, 0, sizeof(ServerResponse)); // Clear response buffer

        switch (header.opcode) {
            case OP_QUERY_AVAILABILITY: {
                pthread_mutex_lock(&ticket_mutex);
                response.remaining_tickets = total_tickets;
                pthread_mutex_unlock(&ticket_mutex);

                strcpy(response.message, "Query successful.");
                
                ProtocolHeader res_header = {
                    .packet_len = sizeof(ProtocolHeader) + sizeof(ServerResponse),
                    .opcode = OP_RESPONSE_SUCCESS,
                    .req_id = header.req_id
                };

                write_n_bytes(client_socket, &res_header, sizeof(ProtocolHeader));
                write_n_bytes(client_socket, &response, sizeof(ServerResponse));
                break;
            }

            case OP_BOOK_TICKET: {
                BookRequest req_body;
                if (read_n_bytes(client_socket, &req_body, sizeof(BookRequest)) <= 0) {
                    perror("Failed to read booking request body");
                    close(client_socket);
                    return;
                }
                
                ProtocolHeader res_header = {
                    .packet_len = sizeof(ProtocolHeader) + sizeof(ServerResponse),
                    .req_id = header.req_id
                };

                pthread_mutex_lock(&ticket_mutex);
                if (total_tickets >= req_body.num_tickets) {
                    total_tickets -= req_body.num_tickets;
                    response.remaining_tickets = total_tickets;
                    sprintf(response.message, "Booking successful for user %u.", req_body.user_id);
                    res_header.opcode = OP_RESPONSE_SUCCESS;
                } else {
                    response.remaining_tickets = total_tickets;
                    sprintf(response.message, "Booking failed: not enough tickets for user %u.", req_body.user_id);
                    res_header.opcode = OP_RESPONSE_FAIL;
                }
                pthread_mutex_unlock(&ticket_mutex);

                write_n_bytes(client_socket, &res_header, sizeof(ProtocolHeader));
                write_n_bytes(client_socket, &response, sizeof(ServerResponse));
                break;
            }

            default: {
                printf("Unknown opcode: %u\n", header.opcode);
                ProtocolHeader res_header = {
                    .packet_len = sizeof(ProtocolHeader) + sizeof(ServerResponse),
                    .opcode = OP_RESPONSE_FAIL,
                    .req_id = header.req_id
                };
                strcpy(response.message, "Unknown operation.");
                response.remaining_tickets = 0; // Or current ticket count

                write_n_bytes(client_socket, &res_header, sizeof(ProtocolHeader));
                write_n_bytes(client_socket, &response, sizeof(ServerResponse));
                break;
            }
        }
    }

    if (read_ret == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("read_n_bytes failed");
    }

    close(client_socket);
}

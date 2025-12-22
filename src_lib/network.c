// src_lib/network.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

/**
 * 建立 Server Socket (socket -> setsockopt -> bind -> listen)
 * * @param port: 要監聽的 Port (例如 8080)
 * @return int: 成功回傳 socket file descriptor，失敗回傳 -1
 */
int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // 1. 建立 Socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 2. 設定 Socket 選項: 允許重用地址 (避免 Server 重啟時 bind 失敗)
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(sockfd);
        return -1;
    }

    // 3. 綁定地址 (Bind)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 監聽所有網卡
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    // 4. 開始監聽 (Listen)
    // backlog 設定為 1024，這對於高併發測試很重要
    if (listen(sockfd, 1024) < 0) { 
        perror("Listen failed");
        close(sockfd);
        return -1;
    }

    log_message(LOG_INFO, "Server socket created on port %d", port);
    return sockfd;
}

/**
 * 建立 Client Socket 並連線到 Server
 * * @param ip: Server 的 IP 位址字串
 * @param port: Server 的 Port
 * @return int: 成功回傳 socket file descriptor，失敗回傳 -1
 */
int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // 1. 建立 Socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // 將字串 IP 轉為二進位格式
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return -1;
    }

    // 2. 連線 (Connect)
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        // 連線失敗由呼叫者決定是否重試，這裡只回傳錯誤
        close(sockfd);
        return -1;
    }

    return sockfd;
}
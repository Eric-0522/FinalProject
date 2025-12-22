// test_logger.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define PROCESS_COUNT 5    // 模擬 5 個 Process
#define LOGS_PER_PROC 100  // 每個 Process 寫 100 行

void worker_task(int id) {
    for (int i = 0; i < LOGS_PER_PROC; i++) {
        log_message(LOG_INFO, "Process %d is writing log line %d", id, i);
        // 隨機延遲，增加競爭機會
        usleep(rand() % 1000);
    }
    printf("Process %d finished.\n", id);
    exit(0);
}

int main() {
    // 移除舊的 log 以便觀察
    remove("test_run.log");
    
    // 初始化寫入 test_run.log
    init_logger("test_run.log");

    printf("Starting Isolated Logger Test...\n");

    for (int i = 0; i < PROCESS_COUNT; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            srand(getpid()); 
            worker_task(i);
        }
    }

    for (int i = 0; i < PROCESS_COUNT; i++) {
        wait(NULL);
    }

    printf("Done. Check 'test_run.log'.\n");
    return 0;
}
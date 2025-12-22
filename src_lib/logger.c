// src_lib/logger.c

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>   // 用於 fcntl 檔案鎖
#include <sys/file.h>
#include <errno.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// 初始化
void init_logger(const char *filename) {
    if (filename) {
        log_file = fopen(filename, "a"); // Append mode
        if (!log_file) {
            perror("Failed to open log file, using stdout");
            log_file = stdout;
        }
    } else {
        log_file = stdout;
    }
}

// 內部 helper: 檔案鎖定
static int lock_file(FILE *fp) {
    int fd = fileno(fp);
    struct flock fl;
    fl.l_type = F_WRLCK;    // 寫入鎖 (獨佔鎖)
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;           // 鎖定整個檔案
    
    // F_SETLKW: 若有鎖則等待 (Wait)
    return fcntl(fd, F_SETLKW, &fl);
}

// 內部 helper: 解除鎖定
static int unlock_file(FILE *fp) {
    int fd = fileno(fp);
    struct flock fl;
    fl.l_type = F_UNLCK;    // 解鎖
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    
    return fcntl(fd, F_SETLKW, &fl);
}

void log_message(LogLevel level, const char *format, ...) {
    if (!log_file) return;

    // 1. 準備時間與層級字串
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);

    const char *level_str = "INFO";
    if (level == LOG_ERROR) level_str = "ERROR";
    else if (level == LOG_DEBUG) level_str = "DEBUG";

    // 2. 格式化訊息內容 (先寫入 buffer 避免多次 I/O)
    char buffer[1024];
    char message_buffer[800]; // 用於存放變數參數處理後的結果

    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);

    // 組合最終字串: [時間] [層級] 訊息\n
    snprintf(buffer, sizeof(buffer), "[%s] [%s] %s\n", time_str, level_str, message_buffer);

    // ==========================================
    // 關鍵區域 (Critical Section)
    // ==========================================
    
    // 第一層保護: 針對同 Process 內的多 Thread
    pthread_mutex_lock(&log_mutex);

    // 第二層保護: 針對不同 Process (使用 OS 檔案鎖)
    lock_file(log_file);

    // 執行寫入
    fputs(buffer, log_file);
    fflush(log_file); // 確保立刻寫入磁碟

    // 解鎖
    unlock_file(log_file);
    pthread_mutex_unlock(&log_mutex);
}
// include/common.h

#ifndef COMMON_H  // 防止重複 include 的保護機制
#define COMMON_H

#include <stdint.h> // 用於 uint32_t, uint16_t 等固定長度型別

// ==========================================
// 1. 操作碼定義 (OpCodes)
// ==========================================
// 用於告訴 Server 這次請求是要做什麼
#define OP_QUERY_AVAILABILITY 0x0001 // 查詢剩餘票數
#define OP_BOOK_TICKET        0x0002 // 訂票請求
#define OP_RESPONSE_SUCCESS   0x1001 // 操作成功
#define OP_RESPONSE_FAIL      0x1002 // 操作失敗

// ==========================================
// 2. 協定標頭 (Header) - 固定 8 bytes
// ==========================================
// __attribute__((packed)) 告訴編譯器不要進行記憶體對齊 (Padding)，確保大小剛好是 8 bytes
typedef struct __attribute__((packed)) {
    uint32_t packet_len;  // 封包總長度 (Header + Body 的 bytes 數)
    uint16_t opcode;      // 操作類型 (如上面的 OP_BOOK_TICKET)
    uint16_t req_id;      // 請求 ID (用於 Client 追蹤是哪一個 Thread 發的)
} ProtocolHeader;

// ==========================================
// 3. 資料內容 (Payload/Body) 結構
// ==========================================

// 訂票請求的 Body (當 OpCode = OP_BOOK_TICKET)
typedef struct __attribute__((packed)) {
    uint32_t num_tickets; // 想買幾張票
    uint32_t user_id;     // 使用者 ID (模擬用)
} BookRequest;

// 伺服器回應的 Body (所有 Response 通用)
typedef struct __attribute__((packed)) {
    uint32_t remaining_tickets; // 剩餘票數
    char message[64];           // 伺服器回傳的訊息 (如 "Booking Success")
} ServerResponse;

// ==========================================
// 4. 函數原型宣告 (Prototypes)
// ==========================================
// 這些函數實作在 src_lib/protocol.c 中

// 基礎網路讀寫 (處理 TCP 黏包/斷包問題)
int read_n_bytes(int sockfd, void *buffer, int n);
int write_n_bytes(int sockfd, void *buffer, int n);

#endif // COMMON_H
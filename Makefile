# ==========================================
# 變數設定 (Variables)
# ==========================================
CC = gcc

# CFLAGS: 基本編譯參數
CFLAGS = -Wall -Wextra -g -Iinclude -fPIC

# LDFLAGS: 連結參數
# -Llib: 連結時去 lib 資料夾找
# -lcommon: 連結 libcommon.so
LDFLAGS = -Llib -lcommon

# [關鍵修改 1] RPATH 設定
# -Wl,-rpath: 告訴 Linker 把路徑寫死在執行檔裡
# '$$ORIGIN/../lib': 代表 "執行檔所在位置/../lib"
# $$ORIGIN 在 Makefile 中需要兩個 $ 才能轉義
LDFLAGS_RPATH = -Wl,-rpath,'$$ORIGIN/../lib'

# LIBS: 特定函式庫
# [關鍵修改 2] Server 移除 -pthread，只保留 -lrt (給 Shared Memory 用)
LIBS_SERVER = -lrt
# Client 仍需要多執行緒模擬壓力測試
LIBS_CLIENT = -pthread

# 目錄路徑定義
SRC_LIB_DIR = src_lib
SERVER_DIR  = server
CLIENT_DIR  = client
INC_DIR     = include

# 輸出目錄定義
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin

# ==========================================
# 檔案清單 (Files)
# ==========================================

SRCS_LIB = $(wildcard $(SRC_LIB_DIR)/*.c)
OBJS_LIB = $(patsubst $(SRC_LIB_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_LIB))

TARGET_LIB    = $(LIB_DIR)/libcommon.so
TARGET_SERVER = $(BIN_DIR)/server
TARGET_CLIENT = $(BIN_DIR)/client

# ==========================================
# 編譯規則 (Build Rules)
# ==========================================

.PHONY: all clean directories

# 預設目標
all: directories $(TARGET_LIB) $(TARGET_SERVER) $(TARGET_CLIENT)
	@echo "=================================================="
	@echo "編譯完成！"
	@echo "現在你可以直接執行 (不需要設定 LD_LIBRARY_PATH):"
	@echo "  Server: ./bin/server"
	@echo "  Client: ./bin/client"
	@echo "=================================================="

# 建立輸出資料夾
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

# --- 1. 編譯動態函式庫 (libcommon.so) ---
$(TARGET_LIB): $(OBJS_LIB)
	@echo "正在連結共用庫: $@"
	$(CC) -shared -o $@ $^

# 編譯 .c -> .o
$(OBJ_DIR)/%.o: $(SRC_LIB_DIR)/%.c
	@echo "正在編譯: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# --- 2. 編譯 Server 執行檔 ---
# [關鍵修改 3] 加入 $(LDFLAGS_RPATH)
$(TARGET_SERVER): $(SERVER_DIR)/server.c $(TARGET_LIB)
	@echo "正在建置 Server..."
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDFLAGS_RPATH) $(LIBS_SERVER)

# --- 3. 編譯 Client 執行檔 ---
# [關鍵修改 3] 加入 $(LDFLAGS_RPATH)
$(TARGET_CLIENT): $(CLIENT_DIR)/client.c $(TARGET_LIB)
	@echo "正在建置 Client..."
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDFLAGS_RPATH) $(LIBS_CLIENT)

# --- 清除規則 ---
clean:
	@echo "正在清除暫存檔與執行檔..."
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
From 483 :

整合步驟：

1. 將 src_lib/network.c 加入 Makefile 的編譯清單（你的 Makefile 使用 wildcard src_lib/*.c ，所以存檔後直接 make 即可自動編譯進 libcommon.so）。

2. 修改 server.c 和 client.c 呼叫這兩個新函式，取代原本冗長的 socket/bind/listen/connect 程式碼。


待修改部份：

目前的 server.c 使用的是：

全域變數 int total_tickets = 100;
pthread_mutex_t ticket_mutex

這在 Multi-Process 架構下會完全失效！ 因為 fork() 出來的子進程會獲得父進程記憶體的複本 (Copy)。

Child Process A 賣了一張票，它的 total_tickets 變 99。
Child Process B 看到的 total_tickets 仍然是 100。
你的 pthread_mutex 也會被複製，導致 A 鎖不住 B。

以上內容完成後請刪除！！！

------------------------------------------------------------------------------------

1. 檢查 logger.c : (僅 logger.c)
- 確保 `bin` 存在 (不存在就 `mkdir -p /bin`)

- 使用 `gcc -o bin/test_logger src_lib/test_logger.c src_lib/logger.c -Iinclude -pthread` 編譯測試檔

- 執行 `./bin/test_logger` 並搭配 `wc -l test_run.log` 查驗結果 (log 檔在專案目錄底下)
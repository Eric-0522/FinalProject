# 購票系統
## Client & Server 
> **負責實作：劉業閎**
- 使用 fork() 為每一個連入的客戶端建立獨立的子行程 (Child Process) 進行處理
- 父行程負責持續監聽 accept()，子行程處理完畢後即結束
- 使用共享記憶體 (shmget, shmat) 來維護一個全域狀態
## Protocol Design
> **負責實作：郭竣安**
- 設計封包 Header、Body 結構
- 實作讀寫封包的功能並確保資料有完整讀寫完
## Security & Reliability
> **負責實作：劉章佑**
- 實作 checksum 功能確認內容沒有被修改，以及 XOR 封包加解密。
- 使用 session_id 驗證 client 端已登入過，才可以進行購票的動作。
- 實作 Timeout 機制
## Logger
> **負責實作：沈柏安**
- 使用 `PTHREAD_MUTEX_INITIALIZER` 初始化一個鎖，其目的是防止同一個 Process 內的不同 Thread 同時寫入
- 使用 `F_WRLCK` (Exclusive Write Lock / 獨佔寫入鎖)。這是 kernel level 的鎖。當一個 Process 鎖住檔案時，OS 會強制其他 Process 等待 (Blocking)，直到鎖被釋放

# 執行結果
- Client.log 的執行結果
![image](https://hackmd.io/_uploads/S1DSJKtQZe.png)
- Server.log 的執行結果
![image](https://hackmd.io/_uploads/BJsskFKQZx.png)


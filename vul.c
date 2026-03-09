#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SESSIONS 5

typedef struct {
    int id;
    char *username;
    void (*log_func)(const char *);
} Session;

Session *sessions[MAX_SESSIONS];

void default_logger(const char *msg) {
    printf("[LOG]: %s\n", msg);
}

// 漏洞 1: 格式化字符串 (Format String Vulnerability)
void custom_logger(const char *msg) {
    // 危险：直接将用户输入作为格式化字符串传递
    printf(msg); 
    printf("\n");
}

void init_session(int index, char *name) {
    if (index < 0 || index >= MAX_SESSIONS) return;
    
    sessions[index] = (Session *)malloc(sizeof(Session));
    sessions[index]->id = index;
    sessions[index]->username = strdup(name);
    sessions[index]->log_func = default_logger;
}

// 漏洞 2: 释放后使用 (Use-After-Free)
void delete_session(int index) {
    if (sessions[index]) {
        printf("Deleting session for %s...\n", sessions[index]->username);
        free(sessions[index]->username);
        free(sessions[index]);
        // 关键错误：没有将 sessions[index] 置为 NULL，导致悬空指针
    }
}

// 漏洞 3: 整数溢出导致的堆溢出 (Integer Overflow -> Heap Overflow)
void process_packet(unsigned short packet_len, char *data) {
    // 如果 packet_len 是 0xFFFF，那么 len + 1 会溢出变成 0
    size_t len = packet_len + 1;
    char *buffer = (char *)malloc(len); 
    
    if (buffer) {
        // 当 len 为 0 时，malloc 仍可能分配极小内存，memcpy 会导致大面积堆溢出
        memcpy(buffer, data, packet_len);
        buffer[packet_len] = '\0';
        printf("Packet processed.\n");
        free(buffer);
    }
}

// 漏洞 4: 栈溢出 (Classic Stack Overflow)
void handle_request(char *input) {
    char local_buf[64];
    // 危险：没有检查 input 的长度
    strcpy(local_buf, input); 
    printf("Request handled: %s\n", local_buf);
}

int main() {
    char attack_input[256];
    
    // --- 场景 1: 触发 UAF ---
    init_session(0, "Admin");
    delete_session(0);
    // 假设攻击者控制了后续的 malloc 分配，覆盖了原有的 Session 结构体
    // 此时再次调用 sessions[0] 就会导致 UAF，甚至执行任意函数指针
    if (sessions[0]) {
        sessions[0]->log_func("This is a UAF trigger!"); 
    }

    // --- 场景 2: 触发格式化字符串 ---
    // 攻击者输入 "%x %x %x %x %p" 可以泄露栈数据
    char *malicious_msg = "%s %s %s %p %p (Exploited!)";
    custom_logger(malicious_msg);

    return 0;
}

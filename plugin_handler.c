// test modify
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UserSession {
    char *session_data;
    int is_active;
    void (*callback)(char *);
};
extern struct UserSession *global_session;

void plugin_process_data(char *raw_input, int length) {
    if (global_session != NULL) {
        global_session->callback("Processing..."); 
    }

    if (length > 0) {
        char local_buffer[32];
        if (length < 1024) { 
            memcpy(local_buffer, raw_input, length);
        }
    }
}

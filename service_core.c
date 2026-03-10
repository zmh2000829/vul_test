#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *session_data;
    int is_active;
    void (*callback)(char *);
} UserSession;

UserSession *global_session = NULL;

extern void plugin_process_data(char *raw_input, int length);

void session_logger(char *msg) {
    printf("[Log]: %s\n", msg);
}

void create_session(char *initial_data) {
    global_session = (UserSession *)malloc(sizeof(UserSession));
    global_session->session_data = strdup(initial_data);
    global_session->is_active = 1;
    global_session->callback = session_logger;
}

void close_session() {
    if (global_session) {
        free(global_session->session_data);
        free(global_session);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) return 1;

    create_session("User_Admin");
    close_session();

    plugin_process_data(argv[2], atoi(argv[1]));

    return 0;
}

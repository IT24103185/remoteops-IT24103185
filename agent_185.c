#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define REG_NUMBER      "IT24103185"
#define PORT            9410
#define AUTH_TOKEN      "OPS-3185"
#define SID_TAG         "SID:5813"
#define LOG_FILE        "remoteops_IT24103185.log"
#define STORAGE_DIR     "./agentfiles/IT24103185/"

// Struct for UDP monitor thread arguments
typedef struct {
    char client_ip[64];
    int udp_port;
    int active;
    pthread_t thread;
} udp_monitor_t;

// Struct for client session
typedef struct {
    int socket_fd;
    char client_ip[64];
    int authenticated;
    udp_monitor_t udp_mon;
} client_session_t;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_event(const char *msg) {
    pthread_mutex_lock(&log_mutex);
    FILE *f = fopen(LOG_FILE, "a");
    if (f) {
        time_t now = time(NULL);
        char *ts = ctime(&now);
        ts[strlen(ts) - 1] = '\0'; // Remove trailing newline
        fprintf(f, "[%s] %s\n", ts, msg);
        fclose(f);
    }
    pthread_mutex_unlock(&log_mutex);
}

ssize_t read_line(int fd, char *buf, size_t maxlen) {
    size_t n = 0;
    while (n < maxlen - 1) {
        char c;
        ssize_t rc = recv(fd, &c, 1, 0);
        if (rc == 1) {
            if (c == '\n') break;
            if (c != '\r') buf[n++] = c;
        } else if (rc == 0) {
            if (n == 0) return 0;
            break;
        } else {
            return -1;
        }
    }
    buf[n] = '\0';
    return n;
}

void *udp_monitor_thread(void *arg) {
    udp_monitor_t *mon = (udp_monitor_t *)arg;
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) return NULL;

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(mon->udp_port);
    inet_pton(AF_INET, mon->client_ip, &dest.sin_addr);

    while (mon->active) {
        char msg[256];
        snprintf(msg, sizeof(msg), "SYSINFO load=0.15 mem=512MB uptime=3600 %s\n", SID_TAG);
        sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr *)&dest, sizeof(dest));
        sleep(3);
    }

    close(udp_sock);
    return NULL;
}

void handle_exec(int client_fd, const char *cmd_name) {
    char response[2048];
    char shell_cmd[256];

    if (strcmp(cmd_name, "DATE") == 0) strcpy(shell_cmd, "date");
    else if (strcmp(cmd_name, "UPTIME") == 0) strcpy(shell_cmd, "uptime");
    else if (strcmp(cmd_name, "DISKFREE") == 0) strcpy(shell_cmd, "df -h /");
    else if (strcmp(cmd_name, "HOSTNAME") == 0) strcpy(shell_cmd, "hostname");
    else if (strcmp(cmd_name, "WHOAMI") == 0) strcpy(shell_cmd, "whoami");
    else {
        snprintf(response, sizeof(response), "ERR 002 COMMAND NOT ALLOWED %s\n", SID_TAG);
        send(client_fd, response, strlen(response), 0);
        return;
    }

    FILE *fp = popen(shell_cmd, "r");
    if (!fp) {
        snprintf(response, sizeof(response), "ERR 003 EXEC FAILED %s\n", SID_TAG);
        send(client_fd, response, strlen(response), 0);
        return;
    }

    char out_buf[1024] = {0};
    fread(out_buf, 1, sizeof(out_buf) - 1, fp);
    pclose(fp);

    for (int i = 0; out_buf[i]; i++) {
        if (out_buf[i] == '\n' || out_buf[i] == '\r') out_buf[i] = ' ';
    }

    snprintf(response, sizeof(response), "OK EXEC_RESULT %s %s\n", out_buf, SID_TAG);
    send(client_fd, response, strlen(response), 0);
}

void handle_put(int client_fd, const char *filename, long filesize) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_DIR, filename);

    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        char err[128];
        snprintf(err, sizeof(err), "ERR 006 CANNOT CREATE FILE %s\n", SID_TAG);
        send(client_fd, err, strlen(err), 0);
        return;
    }

    char buffer[4096];
    long total_received = 0;
    while (total_received < filesize) {
        long to_read = filesize - total_received;
        if (to_read > (long)sizeof(buffer)) to_read = sizeof(buffer);

        ssize_t bytes_read = recv(client_fd, buffer, to_read, 0);
        if (bytes_read <= 0) break;

        fwrite(buffer, 1, bytes_read, fp);
        total_received += bytes_read;
    }
    fclose(fp);

    char ok_msg[256];
    snprintf(ok_msg, sizeof(ok_msg), "OK FILE RECEIVED %s %s\n", filename, SID_TAG);
    send(client_fd, ok_msg, strlen(ok_msg), 0);
}

void handle_get(int client_fd, const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_DIR, filename);

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        char err[128];
        snprintf(err, sizeof(err), "ERR 005 FILE NOT FOUND %s\n", SID_TAG);
        send(client_fd, err, strlen(err), 0);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[256];
    snprintf(header, sizeof(header), "OK FILE SEND %s %ld %s\n", filename, filesize, SID_TAG);
    send(client_fd, header, strlen(header), 0);

    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(client_fd, buffer, bytes_read, 0);
    }
    fclose(fp);
}

void *client_handler(void *arg) {
    client_session_t *session = (client_session_t *)arg;
    int client_fd = session->socket_fd;
    char line[1024];

    char log_buf[1280];
    snprintf(log_buf, sizeof(log_buf), "Connection accepted from %s", session->client_ip);
    log_event(log_buf);

    while (1) {
        ssize_t n = read_line(client_fd, line, sizeof(line));
        if (n <= 0) break;

        snprintf(log_buf, sizeof(log_buf), "Command from %s: %s", session->client_ip, line);
        log_event(log_buf);

        if (strncmp(line, "AUTH ", 5) == 0) {
            char token[64];
            sscanf(line + 5, "%s", token);
            if (strcmp(token, AUTH_TOKEN) == 0) {
                session->authenticated = 1;
                char resp[128];
                snprintf(resp, sizeof(resp), "OK AUTHENTICATED %s\n", SID_TAG);
                send(client_fd, resp, strlen(resp), 0);
            } else {
                char resp[128];
                snprintf(resp, sizeof(resp), "ERR 001 AUTH FAILED %s\n", SID_TAG);
                send(client_fd, resp, strlen(resp), 0);
            }
        } else if (!session->authenticated) {
            char resp[128];
            snprintf(resp, sizeof(resp), "ERR 001 AUTH FAILED %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        } else if (strcmp(line, "SYSINFO") == 0) {
            char resp[256];
            snprintf(resp, sizeof(resp), "OK SYSINFO 0.15 1024 3600 %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        } else if (strcmp(line, "LISTPROC") == 0) {
            char resp[256];
            snprintf(resp, sizeof(resp), "OK PROCS systemd,sshd,bash,agent_185 %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        } else if (strncmp(line, "EXEC ", 5) == 0) {
            handle_exec(client_fd, line + 5);
        } else if (strncmp(line, "PUT ", 4) == 0) {
            char filename[128];
            long filesize = 0;
            if (sscanf(line + 4, "%s %ld", filename, &filesize) == 2) {
                handle_put(client_fd, filename, filesize);
            }
        } else if (strncmp(line, "GET ", 4) == 0) {
            handle_get(client_fd, line + 4);
        } else if (strncmp(line, "MONITOR START ", 14) == 0) {
            int udp_port = atoi(line + 14);
            session->udp_mon.udp_port = udp_port;
            strcpy(session->udp_mon.client_ip, session->client_ip);
            session->udp_mon.active = 1;
            pthread_create(&session->udp_mon.thread, NULL, udp_monitor_thread, &session->udp_mon);

            char resp[128];
            snprintf(resp, sizeof(resp), "OK MONITOR STARTED %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        } else if (strcmp(line, "MONITOR STOP") == 0) {
            if (session->udp_mon.active) {
                session->udp_mon.active = 0;
                pthread_join(session->udp_mon.thread, NULL);
            }
            char resp[128];
            snprintf(resp, sizeof(resp), "OK MONITOR STOPPED %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        } else if (strcmp(line, "QUIT") == 0) {
            char resp[128];
            snprintf(resp, sizeof(resp), "OK BYE %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
            break;
        } else {
            char resp[128];
            snprintf(resp, sizeof(resp), "ERR 000 UNKNOWN COMMAND %s\n", SID_TAG);
            send(client_fd, resp, strlen(resp), 0);
        }
    }

    if (session->udp_mon.active) {
        session->udp_mon.active = 0;
        pthread_join(session->udp_mon.thread, NULL);
    }

    snprintf(log_buf, sizeof(log_buf), "Client disconnected: %s", session->client_ip);
    log_event(log_buf);

    close(client_fd);
    free(session);
    return NULL;
}

int main() {
    mkdir("./agentfiles", 0755);
    mkdir(STORAGE_DIR, 0755);

    log_event("Agent process started.");

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Agent running on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

        if (client_fd >= 0) {
            client_session_t *session = malloc(sizeof(client_session_t));
            session->socket_fd = client_fd;
            session->authenticated = 0;
            session->udp_mon.active = 0;
            inet_ntop(AF_INET, &client_addr.sin_addr, session->client_ip, sizeof(session->client_ip));

            pthread_t thread;
            pthread_create(&thread, NULL, client_handler, session);
            pthread_detach(thread);
        }
    }

    close(server_fd);
    return 0;
}

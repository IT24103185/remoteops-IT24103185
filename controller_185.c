#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9410

int main(int argc, char *argv[]) {
    const char *ip = (argc > 1) ? argv[1] : "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to Agent at %s:%d\n", ip, PORT);

    char buffer[1024];
    while (1) {
        printf("RemoteOps> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "QUIT", 4) == 0) break;

        char response[2048] = {0};
        ssize_t bytes = recv(sock, response, sizeof(response) - 1, 0);
        if (bytes > 0) {
            response[bytes] = '\0';
            printf("Agent: %s", response);
        }
    }

    close(sock);
    return 0;
}

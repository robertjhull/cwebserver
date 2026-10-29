#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <string.h>

#define PORT 8081
#define MAX_QUEUE 128 // max length for queue of pending connections
#define BUFFER_SIZE 2048
#define HTML_FILE "helloworld.html"

char* read_request(int socket, char* buffer, size_t buffer_size) {
    ssize_t n = recv(socket, buffer, buffer_size - 1, 0);
    if (n < 0) {
        perror("error receiving request");
        n = 0;
    }
    buffer[n] = '\0';
    return buffer;
}

void get_method(const char* request, char* method, size_t method_size) {
    const char* sp = strchr(request, ' ');
    if (sp == NULL) {
        method[0] = '\0';
        return;
    }
    size_t len = (size_t)(sp - request);
    if (len >= method_size) {
        len = method_size - 1;
    }
    memcpy(method, request, len);
    method[len] = '\0';
}

int main() {
    // --- prepare html ---
    
    int htmlfd = open(HTML_FILE, O_RDONLY);
    if (htmlfd < 0) {
        printf("error opening %s\n", HTML_FILE);
        return EXIT_FAILURE;
    }

    struct stat st;
    fstat(htmlfd, &st);
    
    // --- headers ---
    
    char status[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server: cwebserver\r\n";
    char content_type[] = "Content-Type: text/html\r\n";
    char content_len[32];
    snprintf(content_len, sizeof(content_len), "Content-Length: %ld\r\n\r\n", (long)st.st_size);
    struct iovec headers[] = {
        { .iov_base = status, .iov_len = strlen(status) },
        { .iov_base = server, .iov_len = strlen(server) },
        { .iov_base = content_type, .iov_len = strlen(content_type) },
        { .iov_base = content_len, .iov_len = strlen(content_len) },
    };

    struct sf_hdtr hdtr = {
        .headers = headers,
        .hdr_cnt = 4,
        .trailers = NULL,
        .trl_cnt = 0,
    };

    // --- socket ---
    
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("error creating socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in host_addr = {
        .sin_family = PF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY),
        }
    };

    if (bind(sock, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0) {
        perror("error binding socket");
        return EXIT_FAILURE;
    }

    if (listen(sock, MAX_QUEUE) < 0) {
        perror("error listening on socket");
        return EXIT_FAILURE;
    } else {
        printf("socket listening on port %d\n", PORT);
    }

    // --- accept connections ---

    for (;;) {
        struct sockaddr client_addr;
        socklen_t client_addrlen = sizeof(client_addr);

        int new_sock = accept(sock, &client_addr, &client_addrlen);
        if (new_sock < 0) {
            perror("error accepting connection");
            continue;
        }

        int opt = 1;
        if (setsockopt(new_sock, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt)) < 0) {
            perror("failed to setsockopt - connection was likely dropped earlier");
            close(new_sock);
            continue;
        }

        char buffer[BUFFER_SIZE];
        char method[5];
        char* req = read_request(new_sock, buffer, sizeof(buffer));
        get_method(req, method, sizeof(method));

        if (strcmp(method, "GET") == 0) {
            off_t sent = 0;
            if (sendfile(htmlfd, new_sock, 0, &sent, &hdtr, 0) < 0) {
                perror("error sending file to client");
            } else {
                printf("%lld bytes sent to client\n", (long long)sent);
            }
        } else if (strcmp(method, "HEAD") == 0) {
            // TODO
        } else if (method[0] != '\0') {
            char resp[] = "HTTP/1.0 405 Method Not Allowed\r\n\r\n";
            write(new_sock, resp, strlen(resp));
        } else {
            char resp[] = "HTTP/1.0 400 Bad Request\r\n\r\n";
            write(new_sock, resp, strlen(resp));
        }

        close(new_sock);
    }

    close(sock);
    close(htmlfd);
    return 0;
}
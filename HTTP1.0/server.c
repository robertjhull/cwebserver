#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
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



char * read_request(int socket, char* buffer, ssize_t length) {
    char buffer[BUFFER_SIZE];
}

int main() {
    char* hdr = "HTTP/1.0 200 OK\r\n";
    struct iovec headers[] = {
        { .iov_base = "HTTP/1.0 200 OK\r\n", .iov_len = sizeof("HTTP/1.0 200 OK\r\n") - 1 },
        { .iov_base = "Server: cwebserver1.1\r\n", .iov_len = sizeof("Server: cwebserver1.1\r\n") - 1 },
        { .iov_base = "Content-type: text/html\r\n\r\n", .iov_len = sizeof("Content-type: text/html\r\n\r\n") - 1 },
    };

    struct sf_hdtr hdtr = {
        .headers = headers,
        .hdr_cnt = 3,
        .trailers = NULL,
        .trl_cnt = 0,
    };

    int htmlfd = open(HTML_FILE, O_RDONLY);
    if (htmlfd < 0) {
        printf("error opening %s\n", HTML_FILE);
        return EXIT_FAILURE;
    }
    
    char buffer[BUFFER_SIZE];
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

        ssize_t read_res = read(new_sock, buffer, BUFFER_SIZE - 1);
        if (read_res < 0) {
            perror("failed to read request");
            close(new_sock);
            continue;
        } else {
            buffer[read_res] = '\0';
            printf("received request: %s", buffer);
        }

        off_t sent = 0;
        if (sendfile(htmlfd, new_sock, 0, &sent, &hdtr, 0) < 0) {
            perror("error sending file to client");
        } else {
            printf("%lld bytes sent to client\n", (long long)sent);
        }
        close(new_sock);
    }

    close(sock);
    close(htmlfd);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>

#define PORT 8080
#define MAX_QUEUE 128 // max length for queue of pending connections
#define BUFFER_SIZE 2048
#define HTML_FILE "helloworld.html"

const char* headers =   "HTTP/1.0 200 OK\r\n"
                        "Server: cwebserver1.1\r\n"
                        "Content-type: text/html\r\n\r\n";

int main() {
    // load the html response
    FILE* fileptr = fopen(HTML_FILE, "r");
    if (fileptr == NULL) {
        perror("error opening file");
        return -1;
    }

    fseek(fileptr, 0, SEEK_END);
    long filesize = ftell(fileptr);
    fseek(fileptr, 0, SEEK_SET);
    
    char* html_buffer = malloc(filesize + 1); // +1 for null terminator
    if (fread(html_buffer, 1, filesize, fileptr) != filesize) {
        printf("something went wrong reading the file or EOF was reached\n");
    }

    printf("HTML loaded: %s\n", html_buffer);
    fclose(fileptr);
    
    char* buffer[BUFFER_SIZE];

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror(NULL);
    }
    printf("successfully created socket\n");

    struct sockaddr_in host_addr;
    host_addr.sin_family = PF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int bind_result = bind(sock, (struct sockaddr *)&host_addr, sizeof(host_addr));
    if (bind_result == -1) {
        perror(NULL);
        return 1;
    }
    printf("socket bound\n");

    int listen_result = listen(sock, MAX_QUEUE);
    if (listen_result == -1) {
        perror(NULL);
        printf("error listening");
        return 1;
    }
    printf("socket listening\n");

    for (;;) {
        struct sockaddr client_addr;
        socklen_t client_addrlen = sizeof(client_addr);

        int fd_newsock = accept(sock, &client_addr, &client_addrlen);
        if (fd_newsock < 0) {
            perror("error accepting connection");
            continue;
        }

        int opt = 1;
        if (setsockopt(fd_newsock, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt)) < 0) {
            perror("failed to setsockopt - connection was likely dropped earlier");
            close(fd_newsock);
            continue;
        }

        printf("connection accepted\n");

        ssize_t read_res = read(fd_newsock, buffer, BUFFER_SIZE - 1);
        if (read_res < 0) {
            perror("failed to read request");
            close(fd_newsock);
            continue;
        }
        buffer[read_res] = '\0';
        printf("received request: %s\n", buffer);

        send(fd_newsock, headers, strlen(headers), 0);
        send(fd_newsock, html_buffer, strlen(html_buffer), 0);

        close(fd_newsock);
    }
    
    return 0;
}
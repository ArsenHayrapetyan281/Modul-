#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define BUFFER_SIZE 256

typedef struct {
    int pipe_fd[2];
} thread_params_t;

void *reader_thread(void *arg) {
    thread_params_t *params = (thread_params_t *)arg;
    int file_fd = open("input.txt", O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(params->pipe_fd[1], buffer, bytes_read) < 0) {
            perror("Failed to write to pipe");
            close(file_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) {
        perror("Failed to read from file");
    }

    close(file_fd);
    close(params->pipe_fd[1]);
    return NULL;
}

void *printer_thread(void *arg) {
    thread_params_t *params = (thread_params_t *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(params->pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
        printf("Read %zd bytes: %.*s\n", bytes_read, (int)bytes_read, buffer);
    }

    if (bytes_read < 0) {
        perror("Failed to read from pipe");
    }

    close(params->pipe_fd[0]);
    return NULL;
}

int main() {
    pthread_t reader_tid, printer_tid;
    thread_params_t params;

    if (pipe(params.pipe_fd) < 0) {
        perror("Failed to create pipe");
        return EXIT_FAILURE;
    }

    if (pthread_create(&reader_tid, NULL, reader_thread, &params) != 0) {
        perror("Failed to create reader thread");
        return EXIT_FAILURE;
    }

    if (pthread_create(&printer_tid, NULL, printer_thread, &params) != 0) {
        perror("Failed to create printer thread");
        return EXIT_FAILURE;
    }

    pthread_join(reader_tid, NULL);
    pthread_join(printer_tid, NULL);

    return 0;
}


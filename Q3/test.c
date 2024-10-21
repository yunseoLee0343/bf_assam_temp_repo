#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE 256
#define MAX_LINES 100

typedef struct {
    char *lines[MAX_LINES];
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int done;
} shared_data_t;

int stat[MAX_STRING_LENGTH];
int stat2[ASCII_SIZE];

void *producer(void *arg) {
    shared_data_t *data = (shared_data_t *)arg;
    FILE *rfile = fopen("input.txt", "r");
    if (rfile == NULL) {
        perror("Error opening file");
        return NULL;
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, rfile) != -1) {
        pthread_mutex_lock(&data->lock);
        while (data->count >= MAX_LINES) {
            pthread_cond_wait(&data->cond, &data->lock);
        }
        data->lines[data->count++] = strdup(line);
        pthread_cond_signal(&data->cond);
        pthread_mutex_unlock(&data->lock);
    }

    pthread_mutex_lock(&data->lock);
    data->done = 1;
    pthread_cond_broadcast(&data->cond);
    pthread_mutex_unlock(&data->lock);

    free(line);
    fclose(rfile);
    return NULL;
}

void *consumer(void *arg) {
    shared_data_t *data = (shared_data_t *)arg;

    while (1) {
        pthread_mutex_lock(&data->lock);
        while (data->count == 0 && !data->done) {
            pthread_cond_wait(&data->cond, &data->lock);
        }

        if (data->count == 0 && data->done) {
            pthread_mutex_unlock(&data->lock);
            break;
        }

        char *line = data->lines[--data->count];
        pthread_cond_signal(&data->cond);
        pthread_mutex_unlock(&data->lock);

        for (char *cptr = line; *cptr != '\0'; cptr++) {
            if (*cptr >= 'A' && *cptr <= 'Z') {
                stat2[*cptr]++;
            } else if (*cptr >= 'a' && *cptr <= 'z') {
                stat2[*cptr]++;
            }
        }

        free(line);
    }

    return NULL;
}

void print_statistics() {
    printf("*** Alphabet Statistics ***\n");
    printf("  A    B    C    D    E    F    G    H    I    J\n");
    for (char c = 'A'; c <= 'Z'; c++) {
        printf("%4d ", stat2[c] + stat2[c + 32]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    pthread_t prod_thread;
    pthread_t cons_threads[4];
    shared_data_t data;
    data.count = 0;
    data.done = 0;

    pthread_mutex_init(&data.lock, NULL);
    pthread_cond_init(&data.cond, NULL);

    pthread_create(&prod_thread, NULL, producer, &data);

    for (int i = 0; i < 4; i++) {
        pthread_create(&cons_threads[i], NULL, consumer, &data);
    }

    pthread_join(prod_thread, NULL);
    for (int i = 0; i < 4; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    print_statistics();

    pthread_mutex_destroy(&data.lock);
    pthread_cond_destroy(&data.cond);

    return 0;
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct sharedobject {
    FILE *rfile;
    int linenum;
    char *line;
    pthread_mutex_t lock;
    pthread_cond_t cond; // 조건 변수 추가
    int full;
} so_t;

void *producer(void *arg) {
    so_t *so = arg;
    int *ret = malloc(sizeof(int));
    FILE *rfile = so->rfile;
    int i = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while (1) {
        read = getdelim(&line, &len, '\n', rfile);
        pthread_mutex_lock(&so->lock);
        while (so->full) {
            pthread_cond_wait(&so->cond, &so->lock);
        }
        if (read == -1) {
            so->line = NULL;
            so->full = 1;
            pthread_cond_signal(&so->cond);
            pthread_mutex_unlock(&so->lock);
            break;
        }
        so->linenum = i;
        so->line = strdup(line);
        so->full = 1;
        pthread_cond_signal(&so->cond);
        pthread_mutex_unlock(&so->lock);
        i++;
    }
    free(line);
    printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
    *ret = i;
    pthread_exit(ret);
}

void *consumer(void *arg) {
    so_t *so = arg;
    int *ret = malloc(sizeof(int));
    int i = 0;
    char *line;

    while (1) {
        pthread_mutex_lock(&so->lock);
        while (!so->full) {
            pthread_cond_wait(&so->cond, &so->lock);
        }
        line = so->line;
        if (line == NULL) {
            pthread_mutex_unlock(&so->lock);
            break;
        }
        printf("Cons_%x: [%02d:%02d] %s", (unsigned int)pthread_self(), i, so->linenum, line);
        free(line);
        so->line = NULL;
        so->full = 0;
        pthread_cond_signal(&so->cond);
        pthread_mutex_unlock(&so->lock);
        i++;
    }
    printf("Cons: %d lines\n", i);
    *ret = i;
    pthread_exit(ret);
}

int main (int argc, char *argv[]) {
    pthread_t prod[100];
    pthread_t cons[100];
    int Nprod, Ncons;
    int rc; long t;
    int *ret;
    int i;
    FILE *rfile;

    if (argc == 1) {
        printf("usage: ./prod_cons <readfile> #Producer #Consumer\n");
        exit (0);
    }
    
    so_t *share = malloc(sizeof(so_t));
    memset(share, 0, sizeof(so_t));
    rfile = fopen((char *) argv[1], "r");
    if (rfile == NULL) {
        perror("rfile");
        exit(0);
    }
    
    if (argv[2] != NULL) {
        Nprod = atoi(argv[2]);
        if (Nprod > 100) Nprod = 100;
        if (Nprod == 0) Nprod = 1;
    } else Nprod = 1;

    if (argv[3] != NULL) {
        Ncons = atoi(argv[3]);
        if (Ncons > 100) Ncons = 100;
        if (Ncons == 0) Ncons = 1;
    } else Ncons = 1;

    share->rfile = rfile;
    share->line = NULL;
    pthread_mutex_init(&share->lock, NULL);
    pthread_cond_init(&share->cond, NULL); // 조건 변수 초기화

    for (i = 0; i < Nprod; i++)
        pthread_create(&prod[i], NULL, producer, share);
    for (i = 0; i < Ncons; i++)
        pthread_create(&cons[i], NULL, consumer, share);
    
    printf("main continuing\n");

    for (i = 0; i < Ncons; i++) {
        rc = pthread_join(cons[i], (void **) &ret);
        printf("main: consumer_%d joined with %d\n", i, *ret);
    }
    for (i = 0; i < Nprod; i++) {
        rc = pthread_join(prod[i], (void **) &ret);
        printf("main: producer_%d joined with %d\n", i, *ret);
    }

    pthread_mutex_destroy(&share->lock); // 뮤텍스 자원 해제
    pthread_cond_destroy(&share->cond);  // 조건 변수 해제
    fclose(rfile); // 파일 포인터 해제
    free(share);   // 공유 객체 해제
    pthread_exit(NULL);
    exit(0);
}

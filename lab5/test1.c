#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

int count = 0;
pthread_t tids[10];

void *consume(void *argv){

    int id = (pthread_t*)argv - tids;
    pthread_mutex_lock(&mutex);
    while (count < 9) {
        ++ count;
        printf("  thread: %d, count: %d\n", id, count);
        pthread_cond_wait(&cond, &mutex);
    }
    printf("thread: %d, count: %d\n", id, count);
    count = 9;
    usleep(1000);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    // singal is just used like it
}



int main(int argc, const char **argcv)
{
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);


    for (int i = 0; i < 10; ++i) {
        if (pthread_create(tids + i, 0, consume, tids + i));
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(tids[i], NULL);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    return 0;
}

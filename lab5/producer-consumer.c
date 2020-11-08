#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
void *ThreadFunc()
{
    static int count = 1;
    printf ("Create thread %d\n", count);
    count++;
}

struct product {
    int product_id;
    int producer_id;
    int comsumer_id;
    struct timeval tv;
    int buf_id;
};



#define POOL_SIZE 3
#define CONSUMER_SIZE 4
#define PRODUCER_SIZE 5

struct product *shared_pool[POOL_SIZE];


const int max_product = 15;


int producted_count = 0;
int producing_count = 0;
int consumed_count = 0;
int consuming_count = 0;



pthread_mutex_t pool_mutex;
pthread_cond_t condc;
pthread_cond_t condp;
pthread_mutex_t count_mutex;

pthread_t consumer_tids[CONSUMER_SIZE];
pthread_t producer_tids[PRODUCER_SIZE];

struct timeval start_tv;

enum pool_state_enum {empty, filled, producing, consuming};

int pool_states [POOL_SIZE];

void init(){
    for (int i = 0; i < POOL_SIZE; ++i) {
        shared_pool[i] = 0;
        pool_states[i] = empty;
    }
    pthread_mutex_init(&pool_mutex, 0);
    pthread_mutex_init(&count_mutex, 0);
    pthread_cond_init(&condc, 0);
    pthread_cond_init(&condp, 0);

    gettimeofday(&start_tv, NULL);
}

void destory(){
    pthread_mutex_destroy(&pool_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&condc);
    pthread_cond_destroy(&condp);

}

void print_pool();

void my_sleep(){
    usleep((rand() % 5 + 1)*1000);
    //sleep((rand() % 5 + 1));
}

int pool_empty_count = POOL_SIZE;
int pool_filled_count = 0;


int get_buf(pool_state_enum test, pool_state_enum set, pthread_cond_t *cnod) {
    pthread_mutex_lock(&pool_mutex);
    while(1){
        if (test == empty && pool_empty_count > 0 ||
            test == filled && pool_filled_count > 0) {
            for (int i = 0; i < POOL_SIZE;++i) {
                if (pool_states[i] == test) {
                    pool_states[i] = set;
                    test == empty ? -- pool_empty_count : -- pool_filled_count;
                    pthread_mutex_unlock(&pool_mutex);
                    return i;
                }
            }
        }
        pthread_cond_wait(cond, &pool_mutex);
    }
}


void print_product(struct product*ppro){
    double t = ppro->tv.tv_sec - start_tv.tv_sec + (ppro->tv.tv_usec - start_tv.tv_usec) / 1.0e6;
    if (ppro->comsumer_id != -1)
        printf("id: %d, producer: %d, comsumer: %d, time: %f, buf: %d\n",
            ppro->product_id, ppro->producer_id, ppro->comsumer_id, t,  ppro->buf_id);
    else
        printf("id: %d, producer: %d, comsumer:  , time: %f, buf: %d\n",
            ppro->product_id, ppro->producer_id, t,  ppro->buf_id);
}


void print_pool(){

    for(int i = 0; i < POOL_SIZE; ++i){
        printf("  %d: ", i);
        if (shared_pool[i])
            print_product(shared_pool[i]);
        else
            printf("null\n");
    }

}

void *produce(void*argv){
    int producer_id = (pthread_t*)argv - producer_tids;
    while(1){
        pthread_mutex_lock(&count_mutex);
        int is_full = producted_count + producing_count == max_product;
        if (is_full) {
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        ++ producing_count;
        // int product_id = producted_count + producing_count;
        pthread_mutex_unlock(&count_mutex);

    
        int buf_id = get_buf(empty, producing, &condp);

        my_sleep();


        struct product* ppro = (struct product*)malloc(sizeof(struct product));
        ppro->buf_id = buf_id;
        ppro->producer_id = producer_id;
        // ppro->product_id = product_id;
        ppro->comsumer_id = -1;
        gettimeofday(&ppro->tv, NULL);


        pthread_mutex_lock(&count_mutex);
        -- producing_count;
        ++ producted_count;
        ppro->product_id = producted_count;
        pthread_mutex_unlock(&count_mutex);

        pthread_mutex_lock(&pool_mutex);
        printf("before %d produce %d in %d\n", producer_id, product_id, buf_id);
        print_pool();
        shared_pool[buf_id] = ppro;

        printf("after %d produce %d in %d\n", producer_id, product_id, buf_id);
        print_pool();
        printf("\n");
        pthread_mutex_unlock(&pool_mutex);


        pthread_mutex_lock(&pool_mutex);
        pool_states[buf_id] = filled;
        ++ pool_filled_count;
        pthread_cond_signal(&condc);
        pthread_mutex_unlock(&pool_mutex);
    }
}

void *consume(void*argv){
    int consumer_id = (pthread_t*)argv - consumer_tids;
    while(1) {

        pthread_mutex_lock(&count_mutex);
        int is_enough = consumed_count + consuming_count == max_product;

        if (is_enough) {
            pthread_mutex_unlock(&count_mutex);
            break;
        }

        ++ consuming_count;
        pthread_mutex_unlock(&count_mutex);

        int buf_id = get_buf(filled, consuming, &condc);

        struct product *ppro = shared_pool[buf_id];
        my_sleep();


        ppro->comsumer_id = consumer_id;

        pthread_mutex_lock(&count_mutex);
        -- consuming_count;
        ++ consumed_count;
        pthread_mutex_unlock(&count_mutex);
        pthread_mutex_lock(&pool_mutex);
        int product_id = ppro->product_id;
        printf("before %d consume %d in %d\n", consumer_id, product_id, buf_id);
        print_pool();
        free(ppro);
        shared_pool[buf_id] = NULL;
        printf("after %d consume %d in %d\n", consumer_id, product_id, buf_id);
        print_pool();
        printf("\n");
        pthread_mutex_unlock(&pool_mutex);


        pthread_mutex_lock(&pool_mutex);
        pool_states[buf_id] = empty;
        ++ pool_empty_count;
        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&pool_mutex);
    }
}




void main(void)
{
    init();

    for (int i = 0; i < CONSUMER_SIZE; ++i) {
        int err = pthread_create(consumer_tids + i, NULL, consume, consumer_tids + i);
        if (err != 0) {
            printf("can't create consumer%d, err: %s\n", i, strerror(err));
        }
    }
    for (int i = 0; i < PRODUCER_SIZE; ++i) {
        int err = pthread_create(producer_tids + i, NULL, produce, producer_tids + i);
        if (err != 0) {
            printf("can't create producer%d, err: %s\n", i, strerror(err));
        }
    }
    for (int i = 0; i < CONSUMER_SIZE; ++i) {
        pthread_join(consumer_tids[i],NULL);
    }

    for (int i = 0; i < PRODUCER_SIZE; ++i) {
        pthread_join(producer_tids[i],NULL);
    }
    printf("\nfinish!!!!!\n");
    destory();
}

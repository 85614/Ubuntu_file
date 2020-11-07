#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
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
    // produce_time
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

pthread_mutex_t pool_ms[POOL_SIZE];
pthread_cond_t pool_cs[POOL_SIZE];

pthread_mutex_t pool_m;
pthread_cond_t pool_c;
pthread_mutex_t count_m;

// pthread_mutex_t print_m;
void init(){
    for (int i = 0; i < POOL_SIZE; ++i) {
        pthread_mutex_init(pool_ms + i, 0);
        pthread_cond_init(pool_cs + i, 0);
        shared_pool[i] = 0;
    }
    pthread_mutex_init(&pool_m, 0);
    pthread_mutex_init(&count_m, 0);
    pthread_cond_init(&pool_c, 0);
    // pthread_mutex_init(&print_m, 0);
}

void destory(){
    for (int i = 0; i < POOL_SIZE; ++i) {
        pthread_mutex_destroy(pool_ms + i);
        pthread_cond_destroy(pool_cs + i);
        shared_pool[i] = 0;
    }

    pthread_mutex_destroy(&pool_m);
    pthread_mutex_destroy(&count_m);
    pthread_cond_destroy(&pool_c);
    // pthread_mutex_destroy(&print_m);
}

void print_pool();
int get_a_buf(){
    pthread_mutex_lock(&pool_m);
    while(1){
        for (int i = 0; i < POOL_SIZE;++i) {
            if (pthread_mutex_trylock(pool_ms + i) == 0){
                // print_pool();
                //printf("%s, %d\n",__FUNCTION__, __LINE__)
                // printf("lock buf %d\n", i );
                pthread_mutex_unlock(&pool_m);
                return i;

            }
        }
        //pthread_cond_wait(&pool_c, &pool_m);

    }
}

void print_product(struct product*ppro){
    // pthread_mutex_lock(&print_m);
    printf("product_id: %d, producer_id: %d, comsumer_id: %d, buf_id: %d\n", ppro->product_id, ppro->producer_id, ppro->comsumer_id, ppro->buf_id);
    // pthread_mutex_unlock(&print_m);
}

void print_pool(){
    // pthread_mutex_lock(&print_m);
    printf("current shared pool is:\n");
    for(int i = 0; i < POOL_SIZE; ++i){
        if (shared_pool[i])
            print_product(shared_pool[i]);
        else
            printf("null\n");
    }
    printf("\n");
    // pthread_mutex_unlock(&print_m);
}

void *produce(void*argv){
    int producer_id = *(int*)(pthread_t*)argv;
    while(1){


        int buf_id = get_a_buf(); //has locked pool_ms[buf_id]
        // pthread_mutex_lock(&print_m);

        pthread_mutex_lock(&count_m);
        int is_full = producted_count + producing_count == max_product;
        if (is_full) {
            pthread_mutex_unlock(&count_m);
            pthread_mutex_unlock(pool_ms + buf_id);
            pthread_cond_signal(&pool_c);
            pthread_cond_signal(pool_cs + buf_id);
            // pthread_mutex_lock(&print_m);
            printf("produce enough\n");
            // pthread_mutex_unlock(&print_m);
            break;
        }
        ++ producing_count;
        int product_id = producted_count + producing_count;
        pthread_mutex_unlock(&count_m);


        printf("lock buf %d producer%d \n", buf_id, producer_id);
        // pthread_mutex_lock(&print_m);

        while(shared_pool[buf_id]) {
            // printf("producer waiting buf %d\n", buf_id);
            pthread_cond_wait(pool_cs + buf_id, pool_ms + buf_id);
            // printf("producer waiting finish buf %d\n", buf_id);
        }



        struct product* ppro = (struct product*)malloc(sizeof(struct product));
        ppro->buf_id = buf_id;
        ppro->producer_id = producer_id;
        ppro->product_id = product_id;
        ppro->comsumer_id = -1;
        shared_pool[buf_id] = ppro;

        sleep(1);

        // pthread_mutex_lock(&print_m);

        // printf("produce, producer: %d, buf: %d, product: %d\n", producer_id, buf_id, ppro->product_id);
        printf("produce: ");
        print_product(ppro);
        // pthread_mutex_unlock(&print_m);

        pthread_mutex_lock(&count_m);
        -- producing_count;
        ++ producted_count;
        pthread_mutex_unlock(&count_m);


        printf("unlock buf %d producer%d \n", buf_id, producer_id);
        pthread_cond_signal(pool_cs + buf_id);
        pthread_cond_signal(&pool_c);
        pthread_mutex_unlock(pool_ms + buf_id);

    }
}

void *consume(void*argv){
    int consumer_id = *(int*)(pthread_t*)argv;
    while(1) {


        int buf_id = get_a_buf(); //has locked pool_ms[buf_id]

        pthread_mutex_lock(&count_m);
        int is_enough = consumed_count + consuming_count == max_product;

        if (is_enough) {
            pthread_mutex_unlock(&count_m);
            pthread_mutex_unlock(pool_ms + buf_id);
            pthread_cond_signal(&pool_c);
            pthread_cond_signal(pool_cs + buf_id);
            // pthread_mutex_lock(&print_m);
            printf("consume enough\n");
            // pthread_mutex_unlock(&print_m);
            break;
        }

        ++ consuming_count;
        pthread_mutex_unlock(&count_m);

        // pthread_mutex_lock(&print_m);
        printf("lock buf %d consumer%d \n", buf_id, consumer_id);
        // pthread_mutex_unlock(&print_m);

        while(!shared_pool[buf_id]) {
            // printf("consumer waiting buf %d\n", buf_id);
            pthread_cond_wait(pool_cs + buf_id, pool_ms + buf_id);
            // printf("consumer waiting finish buf %d\n", buf_id);
        }


        struct product *ppro = shared_pool[buf_id];
        sleep(1);
        // printf("consume, consumer: %d, buf: %d, product: %d\n", consumer_id, buf_id, ppro->product_id);
        // pthread_mutex_lock(&print_m);

        ppro->comsumer_id = consumer_id;
        printf("consume: ");
        print_product(ppro);
        // pthread_mutex_unlock(&print_m);
        shared_pool[buf_id] = NULL;
        pthread_mutex_lock(&count_m);
        -- consuming_count;
        ++ consumed_count;
        pthread_mutex_unlock(&count_m);


        printf("unlock buf %d consumer%d\n", buf_id, consumer_id);
        pthread_cond_signal(pool_cs + buf_id);
        pthread_cond_signal(&pool_c);
        pthread_mutex_unlock(pool_ms + buf_id);

    }
}




void main(void)
{
    init();
    pthread_t consumer_tids[CONSUMER_SIZE];
    pthread_t producer_tids[PRODUCER_SIZE];
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


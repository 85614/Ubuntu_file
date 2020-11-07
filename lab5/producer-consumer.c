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

// pthread_mutex_t pool_ms[POOL_SIZE];
// pthread_cond_t pool_cs[POOL_SIZE];

pthread_mutex_t pool_m;
pthread_cond_t pool_cc;
pthread_cond_t pool_cp;
pthread_mutex_t count_m;

pthread_t consumer_tids[CONSUMER_SIZE];
pthread_t producer_tids[PRODUCER_SIZE];

pthread_mutex_t print_m;

struct timeval start_tv;
enum pool_state_enum {empty, filled, producing, consuming};
int pool_states [POOL_SIZE];
void init(){
    for (int i = 0; i < POOL_SIZE; ++i) {
        pthread_mutex_init(pool_ms + i, 0);
        pthread_cond_init(pool_cs + i, 0);
        shared_pool[i] = 0;
        pool_states[i] = empty;
    }
    pthread_mutex_init(&pool_m, 0);
    pthread_mutex_init(&count_m, 0);
    pthread_cond_init(&pool_cc, 0);
    pthread_cond_init(&pool_cp, 0);
    pthread_mutex_init(&print_m, 0);
    gettimeofday(&start_tv, NULL);
}

void destory(){
    for (int i = 0; i < POOL_SIZE; ++i) {
        pthread_mutex_destroy(pool_ms + i);
        pthread_cond_destroy(pool_cs + i);
        shared_pool[i] = 0;
    }

    pthread_mutex_destroy(&pool_m);
    pthread_mutex_destroy(&count_m);
    pthread_cond_destroy(&pool_cc);
    pthread_cond_destroy(&pool_cp);
    pthread_mutex_destroy(&print_m);
}

void print_pool();

void my_sleep(){
    //usleep((rand() % 5 + 1)*1000);
    sleep((rand() % 5 + 1));
}

//int count = 0;


int pool_TAS(int test, int set, pthread_cond_t *cond){
    pthread_mutex_lock(&pool_m);
    while(1){
        for (int i = 0; i < POOL_SIZE;++i) {
            if (pool_states[i] == test) {
                pool_states[i] = set;
                pthread_mutex_unlock(&pool_m);
                return i;
            }
        }
        pthread_cond_wait(cond, &pool_m);            
    }
}

int pool_set(int i, int state) {
    pthread_mutex_lock(&pool_m);
    shared_pool[i] = state;
    pthread_mutex_unlock(&pool_m);
}


void print_product(struct product*ppro){
    // pthread_mutex_lock(&print_m);

    double t = ppro->tv.tv_sec - start_tv.tv_sec + (ppro->tv.tv_usec - start_tv.tv_usec) / 1.0e6;
    if (ppro->comsumer_id != -1)
        printf("id: %d, producer: %d, comsumer: %d, time: %f, buf: %d\n",
            ppro->product_id, ppro->producer_id, ppro->comsumer_id, t,  ppro->buf_id);
    else
        printf("id: %d, producer: %d, comsumer:  , time: %f, buf: %d\n",
            ppro->product_id, ppro->producer_id, t,  ppro->buf_id);

    // pthread_mutex_unlock(&print_m);
}


void print_pool(){
    pthread_mutex_lock(&print_m);
    for(int i = 0; i < POOL_SIZE; ++i){
        printf("  %d: ", i);
        if (shared_pool[i])
            print_product(shared_pool[i]);
        else
            printf("null\n");
    }
    pthread_mutex_unlock(&print_m);
}

void *produce(void*argv){
    int producer_id = (pthread_t*)argv - producer_tids;
    while(1){
        pthread_mutex_lock(&count_m);
        int is_full = producted_count + producing_count == max_product;
        if (is_full) {
            //pthread_cond_signal(pool_cs + buf_id);
            //pthread_cond_signal(&pool_c);
            //pthread_cond_broadcast(&pool_c);
            pthread_mutex_unlock(&count_m);
            //pthread_mutex_unlock(pool_ms + buf_id);


            // pthread_mutex_lock(&print_m);
            //printf("produce enough with %d producing\n", producing_count);
            // pthread_mutex_unlock(&print_m);
            break;
        }
        ++ producing_count;
        int product_id = producted_count + producing_count;
        pthread_mutex_unlock(&count_m);


        // int buf_id = get_a_buf1(0); //has locked pool_ms[buf_id]
        int buf_id = pool_TAS(empty, producing, &pool_cp);
        // pthread_mutex_lock(&print_m);

        while(shared_pool[buf_id]) {
            // printf("producer waiting buf %d\n", buf_id);
            pthread_cond_wait(pool_cs + buf_id, pool_ms + buf_id);
            // printf("producer waiting finish buf %d\n", buf_id);
        }



        // printf("lock buf %d producer%d \n", buf_id, producer_id);
        // pthread_mutex_lock(&print_m);


        my_sleep();


        struct product* ppro = (struct product*)malloc(sizeof(struct product));
        ppro->buf_id = buf_id;
        ppro->producer_id = producer_id;
        ppro->product_id = product_id;
        ppro->comsumer_id = -1;
        gettimeofday(&ppro->tv, NULL);



        // pthread_mutex_lock(&print_m);

        // printf("produce, producer: %d, buf: %d, product: %d\n", producer_id, buf_id, ppro->product_id);
        // printf("produce: ");
        // print_product(ppro);
        // pthread_mutex_unlock(&print_m);


        pthread_mutex_lock(&count_m);
        -- producing_count;
        ++ producted_count;
        pthread_mutex_unlock(&count_m);

        pthread_mutex_lock(&pool_m);
        printf("before %d produce %d in %d\n", producer_id, product_id, buf_id);
        print_pool();
        shared_pool[buf_id] = ppro;
        // printf("unlock buf %d producer%d \n", buf_id, producer_id);

        //pthread_cond_signal(&pool_c);
        pthread_cond_signal(pool_cs + buf_id);

        //pthread_cond_signal(&pool_cc);
        //pthread_cond_broadcast(&pool_cc);
        printf("after %d produce %d in %d\n", producer_id, product_id, buf_id);
        print_pool();
        printf("\n");
        pthread_mutex_unlock(&pool_m);

        // pthread_mutex_unlock(pool_ms + buf_id);
        pthread_mutex_lock(&pool_m);
        pool_states[buf_id] = filled;
        pthread_cond_signal(&pool_cc);
        pthread_mutex_unlock(&pool_m);
    }
}

void *consume(void*argv){
    int consumer_id = (pthread_t*)argv - consumer_tids;
    while(1) {

        pthread_mutex_lock(&count_m);
        int is_enough = consumed_count + consuming_count == max_product;

        if (is_enough) {
            //pthread_cond_signal(pool_cs + buf_id);
            //pthread_cond_signal(&pool_c);
            //pthread_cond_broadcast(&pool_c);
            pthread_mutex_unlock(&count_m);
            //pthread_mutex_unlock(pool_ms + buf_id);

            // pthread_mutex_lock(&print_m);
            //printf("consume enough with %d consuming\n", consuming_count);
            // pthread_mutex_unlock(&print_m);
            break;
        }

        ++ consuming_count;
        pthread_mutex_unlock(&count_m);

        // int buf_id = get_a_buf1(1);
        int buf_id = pool_TAS(filled, consuming, &pool_cc)
/*
        pthread_mutex_lock(&count_m);
        int is_enough = consumed_count + consuming_count == max_product;

        if (is_enough) {
            pthread_cond_signal(pool_cs + buf_id);
            //pthread_cond_signal(&pool_c);
            pthread_cond_broadcast(&pool_c);
            pthread_mutex_unlock(&count_m);
            pthread_mutex_unlock(pool_ms + buf_id);

            // pthread_mutex_lock(&print_m);
            printf("consume enough\n");
            // pthread_mutex_unlock(&print_m);
            break;
        }

        ++ consuming_count;
        pthread_mutex_unlock(&count_m);
*/
        while(!shared_pool[buf_id]) {
            // printf("consumer waiting buf %d\n", buf_id);
            pthread_cond_wait(pool_cs + buf_id, pool_ms + buf_id);
            // printf("consumer waiting finish buf %d\n", buf_id);
        }




        // pthread_mutex_lock(&print_m);
        // printf("lock buf %d consumer%d \n", buf_id, consumer_id);
        // pthread_mutex_unlock(&print_m);



        struct product *ppro = shared_pool[buf_id];
        my_sleep();
        // printf("consume, consumer: %d, buf: %d, product: %d\n", consumer_id, buf_id, ppro->product_id);
        // pthread_mutex_lock(&print_m);

        ppro->comsumer_id = consumer_id;
        // printf("consume: ");
        // print_product1(ppro);
        // pthread_mutex_unlock(&print_m);


        pthread_mutex_lock(&count_m);
        -- consuming_count;
        ++ consumed_count;
        pthread_mutex_unlock(&count_m);
        pthread_mutex_lock(&pool_m);
        int product_id = ppro->product_id;
        printf("before %d consume %d in %d\n", consumer_id, product_id, buf_id);
        print_pool();
        free(ppro);
        shared_pool[buf_id] = NULL;
        printf("after %d consume %d in %d\n", consumer_id, product_id, buf_id);
        print_pool();
        printf("\n");
        // printf("unlock buf %d consumer%d\n", buf_id, consumer_id);
        pthread_cond_signal(pool_cs + buf_id);
        //pthread_cond_signal(&pool_c);

        //pthread_cond_signal(&pool_cp);
        //pthread_cond_broadcast(&pool_cp);
        pthread_mutex_unlock(&pool_m);
        //pthread_mutex_unlock(pool_ms + buf_id);
        pthread_mutex_lock(&pool_m);
        pool_states[buf_id] = empty;
        pthread_cond_signal(&pool_cp);
        pthread_mutex_unlock(&pool_m);
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

void old_main(){
    int     err;
    pthread_t tid;
    while (1)
    {
           err= pthread_create(&tid, NULL, ThreadFunc, NULL);
           if(err != 0){
               printf("can't create thread: %s\n",strerror(err));
           break;
           }
          usleep(2000);
    }
}

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

struct Product {
    int proNum;  //产品编号
    int producer;  //生产者编号
    time_t tt;  //生产时间
    int poolNum;  //缓冲区编号
    int consumer;  //消费者编号
    Product(int pro, int pool, int pronum) {  //生产商品的构造函数
        producer = pro;
        poolNum = pool;
        proNum = pronum;
        consumer = -1;
        tt = time(NULL);
    }
    Product() {  //初始化
        producer = -1;
        poolNum = -1;
        proNum = -1;
        consumer = -1;
    }
    void print() {  //输出产品信息
        tm* t=localtime(&tt);
        printf("缓冲区编号: %d,  产品编号: %02d,  ", poolNum, proNum);
        printf("生产时间: %d-%02d-%02d %02d:%02d:%02d,  ", t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec);
        printf("生产者编号: %d,  ",producer);
        if (consumer > 0) {
            printf("消费者编号: %d\n", consumer);
        }
        else {
            printf("未被消费\n");
        }
    }
};

struct Product sharedPool[3];  //构建缓冲区
int empty = 3;  //描述空余缓冲区个数
pthread_mutex_t the_mutex;  //互斥量
pthread_cond_t condp, condc;  //条件变量，分别表示生产者和消费者

void printPool() {  //输出缓冲区状态
    for (int i = 0; i < 3; i++) {
        if (sharedPool[i].consumer <= 0 && sharedPool[i].proNum > 0) {
            sharedPool[i].print();
        }
        else {  //商品卖出后当前缓冲区内无商品
            printf("缓冲区编号: %d,  当前无产品\n", i);
        }
    }
}

void *produce(void *pro) { //生产者函数，传入生产者编号
    int producer = *(int*) pro;
    static int count = 1;  //统计已生产的商品个数
    while(count <= 15) {
        sleep(rand() % 4 + 1);
        if (count == 16) {  //生产商品已达上限，退出线程
            pthread_exit(NULL);
            return NULL;
        }
        pthread_mutex_lock(&the_mutex);  //上锁
        while (empty == 0) {  //缓冲区没有商品，则进行等待
            pthread_cond_wait(&condp, &the_mutex);
        }
        if (count <= 15 && empty > 0) {
            printf("生产前：\n");
            printPool();  //输出生产前缓冲区状态
            int flag = -1;
            for (int i = 0; i < 3; i++) {
                if (sharedPool[i].proNum < 0 || sharedPool[i].consumer >= 0 && sharedPool[i].proNum > 0){
                    flag = i;
                    struct Product tmp(producer, flag, count);
                    sharedPool[flag] = tmp;
                    break;
                }
            }
            empty--;
            count++;
            printf("生产后：\n");
            printPool();  //输出生产后缓冲区状态
            printf("\n");
        }
        pthread_cond_broadcast(&condc);  //唤醒消费者线程
        pthread_mutex_unlock(&the_mutex);  //释放锁
    }
    pthread_exit(NULL);
    return NULL;
}

void *consume (void *con) {  //消费者函数，传入消费者变化
    int consumer = *(int*) con;
    static int count = 1;  //统计已消费的商品个数
    while(count <= 15) {
        sleep(rand() % 4 + 1);
        if (count == 16) {
            pthread_exit(NULL);  //消费商品已达上限，退出线程
            return NULL;
        }
        pthread_mutex_lock(&the_mutex);  //上锁
        while (empty == 3) {  //缓冲区没有空余，则进行等待
            pthread_cond_wait(&condc, &the_mutex);
        }
        if (count <= 15 && empty < 3) {
            printf("消费前：\n");
            printPool();  //输出消费前缓冲区状态
            for (int i = 0; i < 3; i++) {
                if (sharedPool[i].consumer < 0 && sharedPool[i].proNum > 0){
                    sharedPool[i].consumer = consumer;
                    printf("被消费商品：\n");
                    sharedPool[i].print();  //输出当前被消费商品状态
                    break;
                }
            }
            empty++;
            count++;
            printf("消费后：\n");
            printPool();  //输出消费后缓冲区状态
            printf("\n");
        }
        pthread_cond_broadcast(&condp);  //唤醒生产者线程
        pthread_mutex_unlock(&the_mutex);  //释放锁
    }
    pthread_exit(NULL);
    return NULL;
}

int main() {
    pthread_t p1, p2, p3, p4, p5, c1, c2, c3, c4;
    int pp1 = 1, pp2 = 2, pp3 = 3, pp4 = 4, pp5 = 5;
    int cc1 = 1, cc2 = 2, cc3 = 3, cc4 = 4;
    pthread_create(&p1, NULL, produce, (void*)&pp1);
    pthread_create(&p2, NULL, produce, (void*)&pp2);
    pthread_create(&p3, NULL, produce, (void*)&pp3);
    pthread_create(&p4, NULL, produce, (void*)&pp4);
    pthread_create(&p5, NULL, produce, (void*)&pp5);
    pthread_create(&c1, NULL, consume, (void*)&cc1);
    pthread_create(&c2, NULL, consume, (void*)&cc2);
    pthread_create(&c3, NULL, consume, (void*)&cc3);
    pthread_create(&c4, NULL, consume, (void*)&cc4);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);
    pthread_join(p5, NULL);
    pthread_join(c1, NULL);
    pthread_join(c2, NULL);
    pthread_join(c3, NULL);
    pthread_join(c4, NULL);
    return 0;
}

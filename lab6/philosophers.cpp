#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PRINT_FONT_BLA  printf("\033[30m"); //黑色
#define PRINT_FONT_RED  printf("\033[31m"); //红色
#define PRINT_FONT_GRE  printf("\033[32m"); //绿色
#define PRINT_FONT_YEL  printf("\033[33m"); //黄色
#define PRINT_FONT_BLU  printf("\033[34m"); //蓝色
#define PRINT_FONT_PUR  printf("\033[35m"); //紫色
#define PRINT_FONT_CYA  printf("\033[36m"); //青色
#define PRINT_FONT_WHI  printf("\033[37m"); //白色



struct philosopher{

    static pthread_mutex_t mutex; // 锁
    const static int PHILOSOPHER_SIZE = 5; // 哲学家数量

    static philosopher ps[PHILOSOPHER_SIZE]; // 哲学家们
    enum state_enum { eating, thinking, hungry }; // 哲学家可能的状态

    pthread_cond_t cond; // 哲学家拥有的条件变量

    state_enum state = thinking; // 哲学家当前的状态
    int id; // 哲学家id
    time_t last_t; // 上次改变状态时的时间

private: // 禁止外部实例化哲学家

    philosopher(){
        pthread_cond_init(&cond, NULL);
        id = this - ps;
        last_t = 0;
        
    }

    ~philosopher(){
        pthread_cond_destroy(&cond);
    }
public:

    philosopher &left()const {
        return ps[(id + PHILOSOPHER_SIZE - 1) % PHILOSOPHER_SIZE];
    }

    philosopher &right()const {
        return ps[(id + 1) % PHILOSOPHER_SIZE];
    }

    void begin_eat() {
        // 打算就餐，条件不满足则等待
        pthread_mutex_lock(&mutex); // 获得锁
        this->state = hungry; // 更新状态为hungry
        print_all(*this); // 打印
        while (left().state == eating || right().state == eating) { 
            // 若左边或右边的哲学家正在就餐
            pthread_cond_wait(&this->cond, &mutex); // 等待
        }
        this->state = eating; // 更新状态为就餐
        print_all(*this); // 打印
        pthread_mutex_unlock(&mutex); // 释放锁
    }

    void begin_think(){
        // 开始思考
        pthread_mutex_lock(&mutex); // 获得锁
        pthread_cond_signal(&left().cond); // 唤醒可能正在等待的左边的哲学家
        pthread_cond_signal(&right().cond); // 唤醒可能正在等待的右边的哲学家
        this->state = thinking; // 更新状态为思考
        print_all(*this); // 打印
        pthread_mutex_unlock(&mutex); // 释放锁
    }

    static void *run(void *pp){
        
        philosopher &p = *(philosopher*)pp;
        
        for (int i = 0; i <  10; ++i) {            

            p.begin_eat(); // 打算就餐
            // 开始就餐
            sleep(rand() % 9 + 2);            
            // 就餐结束
            p.begin_think(); 
            // 开始思考
            sleep(rand() % 6 + 3);
            // 思考结束
        }
        return nullptr;
    }

    static void print_all(philosopher &p){
        // 打印所有的哲学家状态，p是状态刚发生改变的哲学家
        for (int i = 0; i < PHILOSOPHER_SIZE; ++i) {
            if (p.id == i) {
                PRINT_FONT_RED; // 红色字体，给发生改变的哲学家的状态
            }
            else {
                PRINT_FONT_WHI; // 白色字体
            }
            switch (ps[i].state)
            {
            case hungry:
                printf("%10s", "hungry");
                break;
            case eating:
                printf("%10s", "eating");
                break;
            case thinking:
                printf("%10s", "thinking");
                break;
            default:
                break;
            }
            
        }
        time_t tt = time(NULL);
        tm *t = localtime(&tt);
        time_t last_t = p.last_t;
        p.last_t = time(nullptr);
        PRINT_FONT_WHI;
        if (last_t == 0)
            printf(" %10s    ", ""); // 是第一次计时，状态为初始状态，没有上一状态维持的时间
        else  {
            switch (p.state)
            {
            case hungry:
                printf(" %10s %02lds", "thinking", p.last_t - last_t);
                break;
            case eating:
                printf(" %10s %02lds", "hungry", p.last_t - last_t);
                break;
            case thinking:
                printf(" %10s %02lds", "eating", p.last_t - last_t);
                break;
            default:
                break;
            }
        }
        // 打印时的时间
        printf(" %d-%02d-%02d %02d:%02d:%02d\n", 
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);        
    }
};


pthread_mutex_t philosopher::mutex = PTHREAD_MUTEX_INITIALIZER;

philosopher philosopher::ps[philosopher::PHILOSOPHER_SIZE];



int main(int argc, const char **argv){
    
    pthread_t tids[philosopher::PHILOSOPHER_SIZE];
    for (int i = 0; i < philosopher::PHILOSOPHER_SIZE; ++i) {
        int err = pthread_create(tids + i, nullptr, &philosopher::run, philosopher::ps + i);
        if(err != 0){
            printf("can't create thread for philosopher %02d: %s\n", i, strerror(err));
            break;
        }
    }
    for (int i = 0; i < philosopher::PHILOSOPHER_SIZE; ++i) {
        pthread_join(tids[i], NULL);
    }
    return 0;
}

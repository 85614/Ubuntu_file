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

    pthread_mutex_t mutex; // 锁
    const static int PHILOSOPHER_SIZE = 5; // 哲学家数量

    static philosopher ps[PHILOSOPHER_SIZE]; // 哲学家们
    enum state_enum { eating, thinking, hungry }; // 哲学家可能的状态

    pthread_cond_t cond; // 哲学家拥有的条件变量

    state_enum state = thinking; // 哲学家当前的状态
    int id; // 哲学家id
    time_t last_t; // 上次改变状态时的时间

private: // 禁止外部实例化哲学家

    philosopher(){
        pthread_mutex_init(&this->mutex, NULL);
        pthread_cond_init(&this->cond, NULL);
        id = this - ps;
        last_t = 0;
        
    }

    ~philosopher(){
        pthread_mutex_destroy(&this->mutex);
        pthread_cond_destroy(&this->cond);
    }
public:

    philosopher &left()const {
        return ps[(id + PHILOSOPHER_SIZE - 1) % PHILOSOPHER_SIZE];
    }

    philosopher &right()const {
        return ps[(id + 1) % PHILOSOPHER_SIZE];
    }

    void begin_eat() {
        pthread_mutex_lock(&this->mutex);
        this->state = hungry; // 更新状态为hungry
        printf("%*d hungry\n", this->id*12, this->id);
        pthread_mutex_unlock(&this->mutex);

        while(1){
            pthread_mutex_lock(&left().mutex); 
            pthread_mutex_lock(&right().mutex); //先左后右
            if (left().state == eating) {
                pthread_mutex_unlock(&right().mutex);        
                pthread_cond_wait(&this->cond, &left().mutex); // 在左边的锁上等待，等左边唤醒
                pthread_mutex_unlock(&left().mutex);
            }
            else if (right().state == eating) {
                pthread_mutex_unlock(&left().mutex);
                pthread_cond_wait(&this->cond, &right().mutex); // 在右边的锁上等待，等右边唤醒
                pthread_mutex_unlock(&right().mutex);        
            } else {
                pthread_mutex_unlock(&right().mutex);        
                pthread_mutex_unlock(&left().mutex);
                break;
            }
        }
        
        
        pthread_mutex_lock(&this->mutex);
        this->state = eating; // 更新状态为就餐
        printf("%*d eating\n", this->id*12, this->id);
        pthread_mutex_unlock(&this->mutex);
    }

    void begin_think(){
        // 开始思考
        pthread_mutex_lock(&this->mutex); // 获得锁
        pthread_cond_signal(&left().cond); // 唤醒可能正在等待的左边的哲学家
        pthread_cond_signal(&right().cond); // 唤醒可能正在等待的右边的哲学家
        this->state = thinking; // 更新状态为思考
        printf("%*d thinking\n", this->id*12, this->id);
        pthread_mutex_unlock(&this->mutex); // 释放锁
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

};


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

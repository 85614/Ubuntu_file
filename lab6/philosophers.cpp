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
    static pthread_mutex_t mutex;
    const static int PHILOSOPHER_SIZE = 5;

    static philosopher ps[PHILOSOPHER_SIZE];
    enum state_enum { eating, thinking, hungry };

    static constexpr const char *state_str[]{ "eating", "thinking", "hungry" };

    pthread_cond_t cond;

    
    state_enum state = thinking;
    int id;
    time_t last_t;
    
    philosopher(){
        pthread_cond_init(&cond, NULL);
        id = this - ps;
        last_t = 0;
        
    }
    ~philosopher(){
        pthread_cond_destroy(&cond);
    }
    philosopher &left()const {
        return ps[(id + PHILOSOPHER_SIZE - 1) % PHILOSOPHER_SIZE];
    }
    philosopher &right()const {
        return ps[(id + 1) % PHILOSOPHER_SIZE];
    }

    void begin_eat(){
        pthread_mutex_lock(&mutex);
        state = hungry;
        print_all(*this);
        while (left().state == eating || right().state == eating) {
            pthread_cond_wait(&cond, &mutex);
        }
        state = eating;
        print_all(*this);
        pthread_mutex_unlock(&mutex);
    }
    void begin_think(){
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&left().cond);
        pthread_cond_signal(&right().cond);
        state = thinking;
        print_all(*this);
        pthread_mutex_unlock(&mutex);
    }

    static void *run(void *pp){
        
        philosopher &p = *(philosopher*)pp;
        while(true) {            

            p.begin_eat();
            
            sleep(rand() % 9 + 2);            
            
            p.begin_think();

            sleep(rand() % 6 + 3);
        }
        return nullptr;
    }
    static void print_all(philosopher &p){
        
        for (int i = 0; i < PHILOSOPHER_SIZE; ++i) {
            if (p.id == i) {
                PRINT_FONT_RED;
            }
            else {
                PRINT_FONT_WHI;
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
            printf(" %10s    ", "");
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

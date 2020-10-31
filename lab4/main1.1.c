#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>


#define BUF_SIZE 512


enum { success, src_open_fail, dst_open_fail };


int copy_file(const char *src, const char *dst);


pthread_mutex_t c_mutex;


int task_count = 0;
volatile int completed_count = 0;
volatile int failed_count = 0;


void muti_copy_dir(const char *src, const char *dst);


int detail = 0;

void add_completed(){
    while (pthread_mutex_trylock(&c_mutex)){
        // return not zero, lock failed
        // only do a simple thing, use spinlock
        printf("trylock fail\n"); // for debug
    }
    printf("trylock success\n"); // for debug
    ++ completed_count;
    pthread_mutex_unlock(&c_mutex);
}

void add_failed(){
    while (pthread_mutex_trylock(&c_mutex)){
        // return not zero, lock failed
        // only do a simple thing, use spinlock
        printf("trylock fail\n"); // for debug
    }
    printf("trylock success\n"); // for debug
    ++ completed_count;
    pthread_mutex_unlock(&c_mutex);
}


int main(int argc, char **argv){
    for (int i = 3; i < argc; ++i) {
        if (strcmp("--detail", argv[i]) == 0) {
            detail = 1;
        }
    }
    pthread_mutex_init(&c_mutex, NULL);
    muti_copy_dir(argv[1], argv[2]);
    usleep(1000);
    while(task_count > completed_count + failed_count)
    {
        usleep(1000);
    }
    pthread_mutex_destroy(&c_mutex);
    printf("completed %d/%d files\n", completed_count, task_count);
    return 0;

}

int create_thread(void *(*ThreadFunc)(void*),void* arg);



void path_cat(char *buf, const char *base, const char *child);



struct pair {
    char *src;
    char *dst;
};

char *new_str(const char*s){
    int len = strlen(s)+1;
    char*buf = (char*)malloc(len);
    memcpy(buf,s,len);
    return buf;
}

struct pair *new_pair(const char*src, const char *dst){
    struct pair* ans = (struct pair*)malloc(sizeof(struct pair));
    ans->src = new_str(src);
    ans->dst = new_str(dst);
    return ans;
}

void free_pair(struct pair* ppair){
    free(ppair->src);
    free(ppair->dst);
    free(ppair);
}


void *muti_copy_file(void *pvoid){

    struct pair *ppair = (struct pair*)pvoid;
    const char *src = ppair->src;
    const char *dst = ppair->dst;

    printf("start copy file from %s to %s\n", src, dst);
    if (copy_file(src,dst) == success) {
        printf("copy file completed from %s to %s\n", src, dst);
        add_completed();
    }
    else {
        printf("copy file failed from %s to %s\n", src, dst);
        add_failed();
    }



	free_pair(ppair);
}




void muti_copy_dir(const char *src, const char *dst)
{
    DIR * dir;
    struct dirent * ptr;
    //int i;
    dir = opendir(src);
    {
        DIR *dst_dir = opendir(dst);
        if(!dst_dir && mkdir(dst, 0777)){
            printf("create dir fail!\n");
        }
    }
    char *buf = (char*)malloc(100);
    char *buf2 = (char*)malloc(100);

    while((ptr = readdir(dir)) != NULL)
    {
        if (!strcmp("..", ptr->d_name) || !strcmp(".", ptr->d_name))
            continue;
        path_cat(buf, src,ptr->d_name);
        path_cat(buf2, dst,ptr->d_name);

        if (ptr->d_type == DT_DIR){
            muti_copy_dir(buf, buf2);
        }
        else if (ptr->d_type == DT_REG){

            if (!create_thread(muti_copy_file, new_pair(buf,buf2)))
                ++ task_count;
            //muti_copy_file(new_pair(buf,buf2));
            //if(copy_file(buf,buf2)!=success){
             //   printf("copy fail from %s to %s!\n", buf, buf2);
            //}
        } else {
            printf("not a file or dir: %s\n", buf);
        }

    }
    closedir(dir);
    //printf("copy dir completed: %s to %s\n", src, dst);
    free(buf);
    free(buf2);
}






void path_cat(char *buf, const char *base, const char *child){
    int len = strlen(base);
    memmove(buf, base, len);

    buf[len++]='/';
    buf[len++]='\0';
    strcat(buf, child);
}



void copy_dir(const char *src, const char *dst)
{
    DIR * dir;
    struct dirent * ptr;
    //int i;
    dir = opendir(src);
    {
        DIR *dst_dir = opendir(dst);
        if(!dst_dir && mkdir(dst, 0777)){
            printf("create dir fail!\n");
        }
    }
    char *buf = (char*)malloc(100);
    char *buf2 = (char*)malloc(100);
    while((ptr = readdir(dir)) != NULL)
    {
        if (!strcmp("..", ptr->d_name) || !strcmp(".", ptr->d_name))
            continue;
        path_cat(buf, src,ptr->d_name);
        path_cat(buf2, dst,ptr->d_name);

        if (ptr->d_type == DT_DIR){
            copy_dir(buf, buf2);
        }
        else if (ptr->d_type == DT_REG){
            if(copy_file(buf,buf2)!=success){
                printf("copy fail from %s to %s!\n",buf, buf2);
            }
        } else {
            printf("not a file or dir: %s\n", buf);
        }

    }
    closedir(dir);
    free(buf);
    free(buf2);
}


int copy_file(const char *src, const char *dst){

	int infd, outfd;
	char buffer[BUF_SIZE];
	int i;

	if ((infd=open(src,O_RDONLY))<0){
        printf("source file open fail, file name: %s\n", src);
        return src_open_fail;
	}

	if ((outfd=open(dst,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR))<0){
        close(infd);
        printf("destination file open fail, file name: %s\n", dst);
        return dst_open_fail;
	}

	while(1){
		i=read(infd,buffer,BUF_SIZE);
		if (i<=0) break;
		write(outfd,buffer,i);
		if (detail)
            printf("copy file %d bytes from %s to %s\n",i , src, dst);
	}

	close(outfd);
	close(infd);

    return success;
}

int create_thread(void *(*ThreadFunc)(void*),void* arg)
{
    int     err;
    pthread_t tid;
    int  count = 0;
    err= pthread_create(&tid, NULL, ThreadFunc, arg);
    if(err != 0){
        printf("creat threat fail\n");
        return 1;
    }
    return 0;


}


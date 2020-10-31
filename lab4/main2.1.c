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


#define BUF_SIZE 1024


enum { success, src_open_fail, dst_open_fail, unknown };


int copy_file(const char *src, const char *dst);


void muti_copy_dir(const char *src, const char *dst);


int detail = 0;



int main(int argc, char **argv){

    for (int i = 3; i < argc; ++i) {
        if (strcmp("--detail", argv[i]) == 0) {
            detail = 1;
        }
    }
    muti_copy_dir(argv[1], argv[2]);
    return 0;

}





void path_cat(char *buf, const char *base, const char *child);



struct copy_tdata {
    char *src;
    char *dst;
    pthread_t tid;
    int thread_created;
    int ans;
};

char *new_str(const char*s){
    int len = strlen(s)+1;
    char*buf = (char*)malloc(len);
    memcpy(buf,s,len);
    return buf;
}

struct copy_tdata *new_copy_tdata(const char*src, const char *dst){
    struct copy_tdata* ans = (struct copy_tdata*)malloc(sizeof(struct copy_tdata));
    ans->src = new_str(src);
    ans->dst = new_str(dst);
    ans->thread_created = 0;
    ans->ans = unknown;
    return ans;
}

void free_copy_tdata(struct copy_tdata* ppair){
    free(ppair->src);
    free(ppair->dst);
    free(ppair);
}


void *__copy_file(void *pvoid){
    
    struct copy_tdata *ppair = (struct copy_tdata*)pvoid;
    const char *src = ppair->src;
    const char *dst = ppair->dst;

    
    printf("%s ===> %s start\n", src, dst);
    //printf("start copy file from %s to %s\n", src, dst);
    if ((ppair->ans = copy_file(src,dst)) == success) {
        //printf("copy file completed from %s to %s\n", src, dst);
        printf("%s ===> %s completed\n", src, dst);
    }
    else {
        //printf("copy file failed from %s to %s\n", src, dst);
        printf("%s ===> %s failed\n", src, dst);
    }

	return NULL;
	
}



background_copy_file(const char *src, const char *dst, struct copy_tdata *pdata){
    // 创建线程后台复制文件，返回thread id的指针(指向堆中内存)
    // 
    int err;
    pdata->tid = (pthread_t *)malloc(sizeof(pthread_t));
    int  count = 0;
    err= pthread_create(tid, NULL, __copy_file, new_copy_tdata(src, dst));
    if(err != 0){
        printf("creat threat  fail\n");
        
    }
    pdata->thread_created = 1;
}


struct thread_list {
    struct copy_tdata ** list;
    size_t size;
    size_t capacity;
};


void add_thread_data(struct thread_list *list, struct copy_tdata *th) {

    if (!list->list || list->size >= list->capacity) {
        list->capacity = list->size * 3 /2 + 1;
        pthread_t **new_list = (pthread_t **)malloc(list->capacity * sizeof(pthread_t*));
        memmove(new_list, list->list, list->size * sizeof(pthread_t*));
        list->list = new_list;
    }

    list->list [list->size++] = th;

}
void free_threadid(struct thread_list *list)
{

    for(int i = 0 ;i < list->size;++i)
        free_copy_tdata(list->list[i]);
}




void __muti_copy_dir(const char *src, const char *dst, struct thread_list *list);

void muti_copy_dir(const char *src, const char *dst)
{

    struct thread_list list;
    list.list = (pthread_t**)malloc(1);
    list.size = 0;
    list.capacity = 0;

    __muti_copy_dir(src, dst, &list);



    int *thread_ret = NULL;
    int success_count = 0;
    int failed_count = 0;
    for(int i = 0 ;i < list.size; ++i){
        
        pthread_join(list.list[i]->tid, NULL);
        if(thread_ret){
            if (*thread_ret == success)
                ++ success_count;
            else
                ++ failed_count;
            free(thread_ret);
            thread_ret = NULL;
        }
    }
    printf("Copy %ld files in total, %d success, %d failed\n", list.size, success_count, failed_count);
    free_threadid(&list);

}



void __muti_copy_dir(const char *src, const char *dst, struct thread_list *list)
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
            printf("in %s %d\n", __FUNCTION__, __LINE__);
            __muti_copy_dir(buf, buf2, list);
        }
        else if (ptr->d_type == DT_REG){
            struct copy_tdata *data =  new_copy_tdata(buf,buf2);
            pthread_t *ptid = background_copy_file(data->src, data->dst, data);
            if (ptid) {
                // 存储ptid, 到list以供管理
                add_thread_data(list, ptid);
            }
        } else {
            printf("not a file or dir: %s\n", buf);
        }

    }
    closedir(dir);    
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
    char buffer = (char*)malloc(BUF_SIZE);
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
    free(buffer);
    return success;
}





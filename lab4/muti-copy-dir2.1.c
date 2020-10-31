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


// copy_file result enum
enum { success, src_open_fail, dst_open_fail, unknown };


struct copy_tdata {
    char *src; // date in heap, need free
    char *dst; // date in heap, need free
    pthread_t tid;
    int thread_err;
    int result;
};

// background copy file
void background_copy_file(struct copy_tdata *pdata);


int copy_file(const char *src, const char *dst);


// copy directory multi-threads
void muti_copy_dir(const char *src, const char *dst);


// if show copy datil
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
    ans->thread_err = 0;
    ans->result = unknown;
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
    if ((ppair->result = copy_file(src,dst)) == success) {
        //printf("copy file completed from %s to %s\n", src, dst);
        printf("%s ===> %s completed\n", src, dst);
    }
    else {
        //printf("copy file failed from %s to %s\n", src, dst);
        printf("%s ===> %s failed\n", src, dst);
    }

	return NULL;

}



void background_copy_file(struct copy_tdata *pdata){
    // create a thread to copy file, data store into *pdata
    int err;

    int  count = 0;
    pdata->thread_err= pthread_create(&pdata->tid, NULL, __copy_file, pdata);
    if(pdata->thread_err != 0){
        printf("creat threat  fail\n");

    }
}

// a list store thread data, to be manage
struct thread_list {
    struct copy_tdata ** list;
    size_t size;
    size_t capacity;
};


void add_thread_data(struct thread_list *list, struct copy_tdata *th) {

    if (!list->list || list->size >= list->capacity) {
        list->capacity = list->size * 3 /2 + 1;
        struct copy_tdata **new_list = (struct copy_tdata **)malloc(list->capacity * sizeof(pthread_t*));
        memmove(new_list, list->list, list->size * sizeof(pthread_t*));
        list->list = new_list;
    }

    list->list [list->size++] = th;

}
void free_thread_data(struct thread_list *list)
{

    for(int i = 0 ;i < list->size;++i)
        free_copy_tdata(list->list[i]);
}




void __muti_copy_dir(const char *src, const char *dst, struct thread_list *list);

void muti_copy_dir(const char *src, const char *dst)
{

    struct thread_list list;
    list.list = NULL;
    list.size = 0;
    list.capacity = 0;

    __muti_copy_dir(src, dst, &list);

    int success_count = 0;
    int failed_count = 0;
    for(int i = 0 ;i < list.size; ++i){

        struct copy_tdata *data = list.list[i];
        pthread_join(data->tid, NULL);
        if (!data->thread_err && data->result == success)
            ++ success_count;
        else
            ++ failed_count;
        //free_copy_tdata(data); do it in free list
    }
    printf("\nCopy %ld files in total, %d success, %d failed\n", list.size, success_count, failed_count);

    free_thread_data(&list);

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
            background_copy_file(data);
            // 存储ptid, 到list以供管理
            add_thread_data(list, data);
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
    char *buffer = (char*)malloc(BUF_SIZE);
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
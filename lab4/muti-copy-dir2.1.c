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

// 用于记录复制文件线程相关数据的结构体
struct copy_tdata {
    char *src; // 源路径
    char *dst; // 目的路径
    // 由于需要在另一个线程内进行复制，
    // src和dst指向的内存不能在栈中，也不能被共享，
    // 所以需要生成在堆中的副本
    pthread_t tid; //记录 线程id
    int thread_err; // 记录创建线程的返回值
    int result; //记录复制文件的返回值
};

// background copy file，
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




// 连接基础路径和子文件/文件夹的路径
void path_cat(char *buf, const char *base, const char *child);


// 生成字符串在堆中的副本
char *new_str(const char*s){
    int len = strlen(s)+1;
    char*buf = (char*)malloc(len);
    memcpy(buf,s,len);
    return buf;
}

// 用源路径字符串和目的路径字符串，生成用于记录拷贝文件相关数据的结构体
struct copy_tdata *new_copy_tdata(const char*src, const char *dst){
    struct copy_tdata* ans = (struct copy_tdata*)malloc(sizeof(struct copy_tdata));
    ans->src = new_str(src); // 源路径
    ans->dst = new_str(dst); // 目的路径
    // 在堆中创造scr, dst的副本
    ans->thread_err = 0; // 存储创建线程的返回值
    ans->result = unknown;  // 存储复制文件的结果
    return ans;
}

// 释放结构体
void free_copy_tdata(struct copy_tdata* ppair){
    free(ppair->src);
    free(ppair->dst);
    free(ppair);
}

// 用于传入给线程的函数
void *__copy_file(void *pvoid){

    struct copy_tdata *pdata = (struct copy_tdata*)pvoid;
    const char *src = pdata->src;
    const char *dst = pdata->dst;

    // 开始复制文件，将结果赋值给ppdata->result
    printf("%s ===> %s start\n", src, dst);
    if ((pdata->result = copy_file(src,dst)) == success) {
        // 复制文件成功
        printf("%s ===> %s completed\n", src, dst);
    }
    else {
        // 复制文件失败
        printf("%s ===> %s failed\n", src, dst);
    }

	return NULL;

}


// 生成线程后台拷贝文件，将线程信息记录在pdata中
void background_copy_file(struct copy_tdata *pdata){
    // create a thread to copy file, data store into *pdata
    int err;

    int  count = 0;
    // 返回值赋值给pdata->thread_err
    pdata->thread_err= pthread_create(&pdata->tid, NULL, __copy_file, pdata);
    if(pdata->thread_err != 0){
        printf("creat threat  fail\n");

    }
}

// 一个简易的列表
struct thread_list {
    struct copy_tdata ** list;
    size_t size;
    size_t capacity;
};

// 向列表插入数据
void add_thread_data(struct thread_list *list, struct copy_tdata *th) {

    if (!list->list || list->size >= list->capacity) {
        list->capacity = list->size * 3 /2 + 1;
        struct copy_tdata **new_list = (struct copy_tdata **)malloc(list->capacity * sizeof(pthread_t*));
        memmove(new_list, list->list, list->size * sizeof(pthread_t*));
        list->list = new_list;
    }

    list->list [list->size++] = th;

}
// 释放列表中的数据
void free_thread_data(struct thread_list *list)
{

    for(int i = 0 ;i < list->size;++i)
        free_copy_tdata(list->list[i]);
}



// 遍历路径和子文件夹，生成线程拷贝文件，所有的线程存到list中
void __muti_copy_dir(const char *src, const char *dst, struct thread_list *list);


// 多线程拷贝文件夹和子文件夹文件
void muti_copy_dir(const char *src, const char *dst)
{

    // 初始化线程列表
    struct thread_list list;
    list.list = NULL;
    list.size = 0;
    list.capacity = 0;

    // 遍历文件夹及子文件夹，对所有的文件生成线程进行拷贝，将线程信息添加到list中
    __muti_copy_dir(src, dst, &list);

    int success_count = 0;
    int failed_count = 0;
    for(int i = 0 ;i < list.size; ++i){
        // 对所有的线程调用pthread_join，以期完成所有文件的复制才返回
        struct copy_tdata *data = list.list[i];
        pthread_join(data->tid, NULL);
        // 对复制成功和失败的文件进行计数
        if (!data->thread_err && data->result == success)
            ++ success_count;
        else
            ++ failed_count;
        
    }
    printf("\nCopy %ld files in total, %d success, %d failed\n", list.size, success_count, failed_count);
    free_thread_data(&list); //清除所有线程的相关数据

}


// 遍历路径和子文件夹，生成线程拷贝文件，所有的线程存到list中
void __muti_copy_dir(const char *src, const char *dst, struct thread_list *list)
{
    DIR * dir;
    struct dirent * ptr;
    // 打开源目录
    dir = opendir(src);
    // 打开目标目录
    DIR *dst_dir = opendir(dst);
    if(!dst_dir && mkdir(dst, 0777)){
        printf("create dir fail!\n");
    }

    char *buf = (char*)malloc(100);
    char *buf2 = (char*)malloc(100);

    while((ptr = readdir(dir)) != NULL)
    {
        // 跳过 . 和 .. 
        if (!strcmp("..", ptr->d_name) || !strcmp(".", ptr->d_name))
            continue;
        // 连接路径
        path_cat(buf, src,ptr->d_name);
        path_cat(buf2, dst,ptr->d_name);
        if (ptr->d_type == DT_DIR){
            // 若为目录，递归调用此函数
            __muti_copy_dir(buf, buf2, list);
        }
        else if (ptr->d_type == DT_REG) {
            // 若为文件，后台进行复制
            struct copy_tdata *data =  new_copy_tdata(buf,buf2);
            // 后台复制文件，将线程信息存到data中
            background_copy_file(data);
            // data添加到list中
            add_thread_data(list, data);
        } else {
            printf("not a file or dir: %s\n", buf);
        }

    }
    closedir(dir);
    free(buf);
    free(buf2);
}





// 连接基础路径和子文件/文件夹的路径
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
        path_cat(buf, src, ptr->d_name);
        path_cat(buf2, dst, ptr->d_name);

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


// 拷贝文件
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

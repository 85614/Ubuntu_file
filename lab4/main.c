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

void copy_dir(const char *src, const char *dst);
int copy_file(const char *src, const char *dst);
void print_dir_file(const char *path, int depth);

void *ThreadFunc(void*p)
{
    static int count = 1;
    printf ("Create thread %d\n", count);
    count++;

}
volatile int task_count = 0;
volatile int completed_count = 0;

int thread_test(void *(*ThreadFunc)(void*),void* arg)
{
    int     err;
    pthread_t tid;
    int  count = 0;
    err= pthread_create(&tid, NULL, ThreadFunc, arg);
    if(err != 0){
       return 1;
    }
    ++ task_count;

}
struct pair *new_pair(const char*src, const char *dst);

void *muti_copy_dir(void *pvoid);





int main(int argc, char **argv){


    //print_dir_file("../../", 0);
    //thread_test(ThreadFunc,NULL);
    //copy_dir(argv[1], argv[2]);
    muti_copy_dir(new_pair(argv[1],argv[2]));
    usleep(1000);
    while(task_count > completed_count)
    {
        usleep(1000);
    }
    printf("completed %d/%d tasks\n", completed_count, task_count);
    return 0;

}




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
	int infd, outfd;
	char buffer[BUF_SIZE];
	int i;

	if ((infd=open(src,O_RDONLY))<0){
        printf("source file open fail, file name: %s\n", src);
        return NULL;
	}

	if ((outfd=open(dst,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR))<0){
        close(infd);
        printf("destinatin file open fail, file name: %s\n", dst);
        return NULL;
	}

	while(1){
		i=read(infd,buffer,BUF_SIZE);
		printf("copying from %s to %s\n", src, dst);
		if (i<=0) break;
		write(outfd,buffer,i);
	}
	++ completed_count;
    printf("copy file completed: %s to %s\n", src, dst);
	close(outfd);
	close(infd);
    free_pair(ppair);
    return NULL;
}


void *muti_copy_dir(void *pvoid)
{
    struct pair *ppair = (struct pair*)pvoid;
    const char *src = ppair->src;
    const char *dst = ppair->dst;
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
            //thread_test(muti_copy_dir, new_pair(buf,buf2));
            muti_copy_dir(new_pair(buf,buf2));
            //copy_dir(buf, buf2);
        }
        else if (ptr->d_type == DT_REG){
            thread_test(muti_copy_file, new_pair(buf,buf2));
            //muti_copy_file(new_pair(buf,buf2));
            //if(copy_file(buf,buf2)!=success){
             //   printf("copy fail from %s to %s!\n", buf, buf2);
            //}
        } else {
            printf("not a file or dir: %s", buf);
        }

    }
    closedir(dir);
    //printf("copy dir completed: %s to %s\n", src, dst);
    free(buf);
    free(buf2);
    free_pair(ppair);
}






void path_cat(char *buf, const char *base, const char *child){
    int len = strlen(base);
    memcpy(buf,base,len);
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
                printf("copy fail from %s to %s!\n", buf, buf2);
            }
        } else {
            printf("not a file or dir: %s", buf);
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
        return src_open_fail;
	}

	if ((outfd=open(dst,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR))<0){
        close(infd);
        return dst_open_fail;
	}

	while(1){
		i=read(infd,buffer,BUF_SIZE);
		if (i<=0) break;
		write(outfd,buffer,i);
	}

	close(outfd);
	close(infd);

    return success;
}






























void print_dir_full_path(const char *path)
{
    DIR * dir;
    struct dirent * ptr;
    //int i;
    dir = opendir(path);
    char buf[100];
    while((ptr = readdir(dir)) != NULL)
    {
        if (!strcmp("..", ptr->d_name) || !strcmp(".", ptr->d_name))
            continue;

        path_cat(buf, path,ptr->d_name);
        printf("d_name : %s\n", buf);

        if (ptr->d_type == 4){
            path_cat(buf, path,ptr->d_name);
            print_dir_full_path(buf);
        }

    }
    closedir(dir);
}




void print_dir_file(const char *path, int depth)
{
    DIR * dir;
    struct dirent * ptr;
    //int i;
    dir = opendir(path);
    char buf[100];
    while((ptr = readdir(dir)) != NULL)
    {
        if (!strcmp("..", ptr->d_name) || !strcmp(".", ptr->d_name))
            continue;

        for (int i = 0; i < depth; ++i)
            printf("  ");
        printf("d_name : %s\n", ptr->d_name);

        if (ptr->d_type == 4){
            //printf("ptr->d_off: %ld", ptr->d_off);
            int len = strlen(path);
            memcpy(buf,path,len);
            buf[len++]='/';
            buf[len++]='\0';
            strcat(buf, ptr->d_name);
            //printf("\n\n\n the child path is \n: %s\n", buf);
            print_dir_file(buf ,depth+1);
        }

    }
    closedir(dir);
}

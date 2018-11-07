#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <pthread.h>
void setoff(int fd,long long size){
	ioctl(fd,0x6677889C,size);
}
void core_read(int fd,char *buf){
	ioctl(fd,0x6677889b,buf);
}
void core_copy_func(int fd,long long size){
	ioctl(fd,0x6677889a,size);
}

int main(){
	int fd ;
	size_t tmp ;
	char buf[0x50]="123134564654987987897987987987987987";
	for(int i = 0;i<8;i++){
		tmp = *(size_t *)(&buf[i*8]);
		printf("[%d] %p\n",i,tmp);
	}


}
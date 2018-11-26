#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define ADD_ITEM    0x1337
#define SELECT_ITEM 0X1338
#define REMOVE_ITEM 0X1339
#define LIST_HEAD   0X133A

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

int main(){
	static int fd ;
	int i;
	//char a[]="p4nda";
	char *mem = malloc(0x1000);
	char *result = malloc(0x1000);
	
	fd = open("/dev/klist",O_RDWR);
	if(fd < 0){
		puts("[-] open file error!");
		exit(-1);
	}
	list_head(fd,result);
	print_hex(result,0xc0);
}
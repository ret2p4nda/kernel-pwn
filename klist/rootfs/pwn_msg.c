#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#define BUFF_SIZE 96-48




int main(){
struct {
    long mtype;
    char mtext[BUFF_SIZE];
} msg;
int i;
memset(msg.mtext, 0x42, BUFF_SIZE-1);
msg.mtext[BUFF_SIZE] = 0;
msg.mtype = 1;
for(i = 0;i<BUFF_SIZE;i++)
	msg.mtext[i] = '\xff';

int msqid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);

msgsnd(msqid, &msg, sizeof(msg.mtext), 0);

system("pause");

}

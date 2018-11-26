#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
 
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}
 
int main(void)
{
    pid_t pid;
    char buf[1024];
    int fd[2];
    char *p = "test for pipe\n";
    
   if (pipe(fd) == -1) 
       sys_err("pipe");
 
   
}

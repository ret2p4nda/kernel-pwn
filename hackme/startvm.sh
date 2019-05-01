#! /bin/sh
cd `dirname $0`
stty intr ^]
qemu-system-x86_64 \
    -m 256M \
    -nographic \
    -kernel bzImage \
    -append 'console=ttyS0 loglevel=3 oops=panic panic=1 kaslr' \
    -monitor /dev/null \
    -initrd initramfs.cpio \
    -smp cores=4,threads=2 \
    -gdb tcp::4869 -S \
    -cpu qemu64,smep,smap 2>/dev/null


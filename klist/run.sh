#!/bin/sh

qemu-system-x86_64 -enable-kvm -cpu kvm64,+smep -kernel ./bzImage -append "console=ttyS0 root=/dev/ram rw oops=panic panic=1 quiet kaslr" -initrd ./rootfs.cpio -nographic -m 2G -smp cores=2,threads=2,sockets=1 -monitor /dev/null -nographic


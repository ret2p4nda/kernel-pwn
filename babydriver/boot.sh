#!/bin/bash

qemu-system-x86_64 -initrd core.cpio -kernel bzImage -append 'console=ttyS0 root=/dev/ram oops=panic panic=1' -enable-kvm -monitor /dev/null -m 128M --nographic  -smp cores=1,threads=1 -cpu kvm64,+smep -s

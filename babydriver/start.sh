qemu-system-x86_64 \
-m 256M \
-kernel ./bzImage \
-initrd  ./core.cpio \
-append 'console=ttyS0 root=/dev/ram oops=panic panic=1 quiet' \
-enable-kvm -monitor /dev/null -m 128M --nographic  -smp cores=1,threads=1 -cpu kvm64,+smep  \
-gdb tcp::1234 -S \
-netdev user,id=t0, -device e1000,netdev=t0,id=nic0 \
-nographic  \

qemu-system-x86_64 \
-m 256M \
-kernel ./bzImage \
-initrd  ./core.cpio \
-append "root=/dev/ram rw console=ttyS0 oops=panic panic=1 quiet kaslr useradd p4nda" \
-gdb tcp::1234 -S \
-netdev user,id=t0, -device e1000,netdev=t0,id=nic0 \
-nographic  \

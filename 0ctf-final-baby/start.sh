qemu-system-x86_64 \
-m 256M -smp 2,cores=2,threads=1  \
-kernel ./vmlinuz-4.15.0-22-generic \
-initrd  ./core.cpio \
-append "root=/dev/ram rw console=ttyS0 oops=panic panic=1 quiet" \
-cpu qemu64 \
-netdev user,id=t0, -device e1000,netdev=t0,id=nic0 \
-nographic  -enable-kvm  \

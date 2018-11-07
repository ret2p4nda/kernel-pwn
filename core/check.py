from pwn import *
elf = ELF('./core/vmlinux')
print "commit_creds",hex(elf.symbols['commit_creds']-0xffffffff81000000)
print "prepare_kernel_cred",hex(elf.symbols['prepare_kernel_cred']-0xffffffff81000000)
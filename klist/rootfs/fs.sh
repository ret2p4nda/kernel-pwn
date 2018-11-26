#!/bin/sh
find . | cpio -o --format=newc > ../rootfs.cpio


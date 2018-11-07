#!/bin/sh
find . | cpio -o --format=newc > ../core.cpio


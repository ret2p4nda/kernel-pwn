#!/bin/sh
find . | cpio -o --format=newc > ../solid_core.cpio


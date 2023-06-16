#!/bin/sh

set -xe

gcc fasthash.c -o fasthash -Wall -pedantic -I. -O2 -DNO_STDLIB -fno-tree-vectorize -march=native

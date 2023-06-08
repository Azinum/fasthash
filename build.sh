#!/bin/sh

set -xe

gcc fasthash.c -o fasthash -Wall -pedantic -I. -O2 -DNO_STDLIB -fno-tree-vectorize -march=native
gcc fasthash.c -o fasthash_debug -Wall -pedantic -I. -O0 -DNO_STDLIB -fno-tree-vectorize -march=native

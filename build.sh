#!/bin/sh

FLAGS_COMMON='-Wall -pedantic -I. -O2 -DNO_STDLIB -march=native'

set -xe

gcc   fasthash.c -o fasthash_gcc_no_tree_vectorize   ${FLAGS_COMMON} -fno-tree-vectorize -DINFO=\""compiler: $(gcc --version   | head -1), no tree vectorization"\"
gcc   fasthash.c -o fasthash_gcc                     ${FLAGS_COMMON}                     -DINFO=\""compiler: $(gcc --version   | head -1)"\"
clang fasthash.c -o fasthash_clang_no_tree_vectorize ${FLAGS_COMMON} -fno-tree-vectorize -DINFO=\""compiler: $(clang --version | head -1), no tree vectorization"\"
clang fasthash.c -o fasthash_clang                   ${FLAGS_COMMON}                     -DINFO=\""compiler: $(clang --version | head -1)"\"

#!/bin/sh

OUTPUT=test.txt

</dev/null>${OUTPUT}

./fasthash_gcc_no_tree_vectorize   >> ${OUTPUT}
./fasthash_gcc                     >> ${OUTPUT}
./fasthash_clang_no_tree_vectorize >> ${OUTPUT}
./fasthash_clang                   >> ${OUTPUT}

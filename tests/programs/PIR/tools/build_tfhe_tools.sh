#!/bin/bash


TFHE_PREFIX=/usr/local
export C_INCLUDE_PATH=$C_INCLUDE_PATH:$TFHE_PREFIX/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$TFHE_PREFIX/include
export LIBRARY_PATH=$LIBRARY_PATH:$TFHE_PREFIX/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TFHE_PREFIX/lib

gcc keygen.c -o keygen -ltfhe-spqlios-fma

gcc sym_encrypt.c -o sym_encrypt -ltfhe-spqlios-fma

gcc decrypt.c -o decrypt -ltfhe-spqlios-fma


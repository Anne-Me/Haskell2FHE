#!/bin/bash


TFHE_PREFIX=/usr/local
export C_INCLUDE_PATH=$C_INCLUDE_PATH:$TFHE_PREFIX/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$TFHE_PREFIX/include
export LIBRARY_PATH=$LIBRARY_PATH:$TFHE_PREFIX/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TFHE_PREFIX/lib

g++ -g main.cpp Evaluator.cpp CircuitGraph.cpp tests/circuitgraph_tests.cpp -o clash2tfhe -ltfhe-spqlios-fma

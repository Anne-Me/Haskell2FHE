#!/bin/bash

./build/clash2tfhe -c tests/programs/PIR/pir1.json -n 4 tests/programs/PIR/ciphertext0.data tests/programs/PIR/ciphertext1.data tests/programs/PIR/ciphertext2.data tests/programs/PIR/ciphertext3.data -b 8 -out tests/programs/PIR/result.data -cloud tests/programs/PIR/cloud.key

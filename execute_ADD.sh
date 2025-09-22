#!/bin/bash

# Example: addition with 64 bit numbers
# 18446744073709551010 + 38 = 18446744073709551048

# Default KeyGen
./build/keygen

# Encrypt numbers 2147480640 and 38
./build/encrypt -b 64 -n 2 18446744073709551010 38 -key secret.key -prefixout ct_in 

# Evaluate homomorphically
./build/clash2tfhe -cjson tests/programs/ADD/add_64bitu.json -n 2 ct_in_0.data ct_in_1.data -out result.data -boot boots.key -b 64 -t 1

# Decrypt result
./build/decrypt -b 64 -key secret.key result.data
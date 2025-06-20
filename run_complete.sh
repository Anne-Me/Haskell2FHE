#!/bin/bash

#./tfhe-tools/gen_and_encrypt -n 2 340 12 -key /tfhe-tools/secret.key /tfhe-tools/cloud.key

output=$(./build/clash2tfhe -c tests/programs/ADD64/add64.json -n 4 /tfhe-tools/ciphertext0.data /tfhe-tools/ciphertext1.data -b 64 -out /tfhe-tools/result.data -cloud /tfhe-tools/public.key)

echo "$output"

./tfhe-tools/decrpyt -key /tfhe-tools/secret.key /tfhe-tools/result.data

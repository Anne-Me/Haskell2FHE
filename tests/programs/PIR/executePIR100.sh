#!/bin/bash


inputs=""

for i in {0..99}; do
  inputs+="ciphertext${i}.data "
done

echo "./../../../build/clash2tfhe -c PIR100.json -n 101 ciphertexte.data $inputs -b 32 -cloud tools/boots.key -out result.data"

#output=$(./../../../build/clash2tfhe -c PIR100.json -n 101 ciphertexte.data $inputs -b 32 -cloud tools/boots.key -out result.data)

#echo "$output"

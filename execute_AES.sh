#!/bin/bash

# test vectors from NIST: 
#  plaintext = 0x3243f6a8885a308d313198a2e0370734
#        key = 0x2b7e151628aed2a6abf7158809cf4f3c
# ciphertext = 0x3925841d02dc09fbdc118597196a0b32

#ct = 0b0011_1001 :> 0b0010_0101 :> 0b1000_0100 :> 0b0001_1101 :> 0b0000_0010 :> 0b1101_1100 :> 0b0000_1001 :> 0b1111_1011 :> 0b1101_1100 :> 0b0001_0001 :> 0b1000_0101 :> 0b1001_0111 :> 0b0001_1001 :> 0b0110_1010 :> 0b0000_1011 :> 0b0011_0010 :> Nil

./build/keygen              
./build/encrypt -b 128 -n 1 0x3243f6a8885a308d313198a2e0370734 -key secret.key -prefixout ct_in
./build/encrypt -n 11 0x2B7E151628AED2A6ABF7158809CF4F3C 0xA0FAFE1788542CB123A339392A6C7605 0xF2C295F27A96B9435935807A7359F67F 0x3D80477D4716FE3E1E237E446D7A883B 0xEF44A541A8525B7FB671253BDB0BAD00 0xD4D1C6F87c839d87caf2b8bc11f915bc 0x6d88a37a110b3efddbf98641ca0093fd 0x4e54f70e5f5fc9f384a64fb24ea6dc4f 0xead27321b58dbad2312bf5607f8d292f 0xac7766f319fadc2128d12941575c006e 0xd014f9a8c9ee2589e13f0cc8b6630ca6 -b 128 -key secret.key -prefixout ct_key
./build/clash2tfhe -c tests/programs/AES/aesencrypt.json -n 12 ct_in_0.data ct_key_0.data ct_key_1.data ct_key_2.data ct_key_3.data ct_key_4.data ct_key_5.data ct_key_6.data ct_key_7.data ct_key_8.data ct_key_9.data ct_key_10.data  -out result.data -boot boots.key -b 128 -t 5
./build/decrypt -b 128 -key secret.key result.data
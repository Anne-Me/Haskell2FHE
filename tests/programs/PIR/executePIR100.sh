#!/bin/bash


inputs=""

for i in {0..99}; do
  inputs+="ciphertext${i}.data "
done

echo "./../../../build/clash2tfhe -c PIR100.json -n 101 ciphertexte.data $inputs -b 32 -cloud tools/boots.key -out result.data -t 5"

#output=$(./../../../build/clash2tfhe -c PIR100.json -n 101 ciphertexte.data $inputs -b 32 -cloud tools/boots.key -out result.data)

#echo "$output"


./../../../build/clash2tfhe -c PIR100.json -n 101 ciphertext1.data ciphertext0.data ciphertext1.data ciphertext2.data ciphertext3.data ciphertext4.data ciphertext5.data ciphertext6.data ciphertext7.data ciphertext8.data ciphertext9.data ciphertext10.data ciphertext11.data ciphertext12.data ciphertext13.data ciphertext14.data ciphertext15.data ciphertext16.data ciphertext17.data ciphertext18.data ciphertext19.data ciphertext20.data ciphertext21.data ciphertext22.data ciphertext23.data ciphertext24.data ciphertext25.data ciphertext26.data ciphertext27.data ciphertext28.data ciphertext29.data ciphertext30.data ciphertext31.data ciphertext32.data ciphertext33.data ciphertext34.data ciphertext35.data ciphertext36.data ciphertext37.data ciphertext38.data ciphertext39.data ciphertext40.data ciphertext41.data ciphertext42.data ciphertext43.data ciphertext44.data ciphertext45.data ciphertext46.data ciphertext47.data ciphertext48.data ciphertext49.data ciphertext50.data ciphertext51.data ciphertext52.data ciphertext53.data ciphertext54.data ciphertext55.data ciphertext56.data ciphertext57.data ciphertext58.data ciphertext59.data ciphertext60.data ciphertext61.data ciphertext62.data ciphertext63.data ciphertext64.data ciphertext65.data ciphertext66.data ciphertext67.data ciphertext68.data ciphertext69.data ciphertext70.data ciphertext71.data ciphertext72.data ciphertext73.data ciphertext74.data ciphertext75.data ciphertext76.data ciphertext77.data ciphertext78.data ciphertext79.data ciphertext80.data ciphertext81.data ciphertext82.data ciphertext83.data ciphertext84.data ciphertext85.data ciphertext86.data ciphertext87.data ciphertext88.data ciphertext89.data ciphertext90.data ciphertext91.data ciphertext92.data ciphertext93.data ciphertext94.data ciphertext95.data ciphertext96.data ciphertext97.data ciphertext98.data ciphertext99.data  -b 32 -cloud tools/boots.key -out result.data -t 1


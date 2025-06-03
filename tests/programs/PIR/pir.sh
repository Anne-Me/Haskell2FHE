
start=$(date +%s.%N)

./gen_and_encrypt -n 9 10 20 30 40 50 60 70 80 4 -secret secret.key cloud.key 
./clash2tfhe -c programs/pir.json -n 9 ciphertext0.data ciphertext1.data ciphertext2.data ciphertext3.data ciphertext4.data ciphertext5.data ciphertext6.data ciphertext7.data ciphertext8.data -cloud cloud.key -out result.data -b 8 
./decrypt -key secret.key result.data  


end=$(date +%s.%N)
runtime=$(echo "$end - $start" | bc -l)
echo "Execution time: $runtime seconds"

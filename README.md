
### Compile

mkdir build  
cd build  
cmake ..  
make  

### Execute
(currently runs ori example)

./run.sh 


Install yosys: https://yosyshq.net/yosys/documentation.html  


install clash https://clash-lang.org/ 


### Run a clash example program for addition of two numbers:
cd my-clash-project/src/Examples
#rename the add2.js to Project.hs

cd my-clash-project/tests/Tests/Example
# rename the add2.js to Project.hs

### Go to Clash main folder

stack build
stack test
stack run clash -- Example.Project --verilog 

### Run Yosys for optimisation
read_verilog verilog/Example.Project.topEntity/topEntity.v 
flatten 
Synth 
write_json add2.json 

# copy add2.json /FHE-Compiler/programs

### To run the direct script:
./fhe.sh

### Output ###
input_length: 16 result_length: 8
done evaluating, numgates: 40
Total execution time: 0.000000 seconds (663.000 milliseconds)
Plaintext 0: 8
Execution time: 1.836806707 seconds


### Run command by command

# Encrypt the numbers
cd ../FHE-Compiler
./gen_and_encrypt -n 2 25 32 -secret secret.key cloud.key 


# Perform homomorphic operations:
./clash2tfhe -c programs/add2.json -n 2 ciphertext0.data ciphertext1.data -cloud cloud.key -out result.data -b 8 

# Decrypt the result
./decrypt -key secret.key result.data  



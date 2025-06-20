## A Haskell to FHE Transpiler 

The compiler uses 4 tools: Clash, Yosys, tfhe and this library.
You can execute examples even if only the tfhe library and this repo are installed (Step 1 and Step 2)
To compile Haskell prgrams from scratch please install Clash and Yosys too and follow the instructions in Compiling Haskell from Scratch

### Step 1: Clone and Compile tfhe
Maybe you need some essentials first
```
sudo apt update
sudo apt install build-essentials
```

```
git clone git@github.com:tfhe/tfhe.git
mkdir build  
cd build  
cmake ../src -DENABLE_TESTS=off -DENABLE_FFTW=on -DCMAKE_BUILD_TYPE=optim 
make  
```

### Step 2: Clone and Compile this library 
```
git clone git@github.com:Anne-Me/Haskell2FHE.git
mkdir build  
cd build  
cmake ..  
make  
```

### Step 3: Install Clash
install clash https://clash-lang.org/ 


### Step 4: Install Yosys
Install yosys: https://yosyshq.net/yosys/documentation.html  


### Execute Examples



### OLD INSTRUCTIONS TODO

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



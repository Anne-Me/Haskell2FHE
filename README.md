## A Haskell to FHE Transpiler 

The compiler takes Haskell programs and transforms them into Boolean circuits using Clash and Yosys. Then the program evaluated using the tfhe library. You can evaluate the program in parallel using any number of threads by specifying the thread parameter.

### Step 1: Clone and Compile tfhe
Install build tools.

```
sudo apt-get update
sudo apt-get install build-essential cmake
```

```
git clone git@github.com:tfhe/tfhe.git
cd tfhe
make
sudo make install
cd ..
```

### Step 2: Clone and Compile this library 
```
git clone git@github.com:Anne-Me/Haskell2FHE.git
cd Haskell2FHE
mkdir build  
cd build  
cmake ..  
make  
cd ../..
```

At this point you can already execute the example programs (see "Executing Examples") and you can run programs in Bristal Fashion format or programs that were created by the tojson command in Yosys.

### Step 3: Install Clash
install clash https://clash-lang.org/ 


### Step 4: Install Yosys
Installing yosys (https://yosyshq.net/yosys/documentation.html)

```
sudo apt-get install build-essential clang lld bison flex \
	libreadline-dev gawk tcl-dev libffi-dev git \
	graphviz xdot pkg-config python3 libboost-system-dev \
	libboost-python-dev libboost-filesystem-dev zlib1g-dev
git clone --recurse-submodules https://github.com/YosysHQ/yosys.git
cd yosys
git checkout v0.53
make
sudo make install
cd ..
```


### Executing Examples

#### Addition of 64-bit numbers

The following script runs key generation, encryption of two numbers, evaluation and decryption.

```
./execute_Add.sh
```


### Compile Haskell program into FHE compatible boolean circuit

### Now you can create your ciphertexts and run the program

```
./build/encrypt
```


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



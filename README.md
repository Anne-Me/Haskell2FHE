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

At this point you can already execute the example programs using precompiled circuits (see "Executing Examples") and you can run programs in Bristol Fashion format or programs that were created by the tojson command in Yosys.

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

#### AES-128 encryption
The following script encrypts a plaintext and the expanded keys of a AES-128 KeySchedule and then encrypts the plaintext under FHE. 

```
./execute_AES.sh
```


#### PIR-100 
The following script creates a dummy database which encrypts values 0 to 99 as 32 bit integers and a query integer. Then it evaluates PIR homomorphically using 5 parallel threads and decrypts the output. 

```
./executePIR100.sh 
```


### Compile Haskell program into FHE compatible boolean circuit

TODO: add explanation

### Now you can create your ciphertexts and run the program

TODO: add explanation


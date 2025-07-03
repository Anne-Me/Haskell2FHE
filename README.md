## A Haskell to FHE Transpiler 

The compiler takes Haskell programs and transforms them into Boolean circuits using Clash and Yosys. Then the program evaluated using the tfhe library. You can evaluate the program in parallel using any number of threads by specifying the thread parameter.

## Install

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
install clash https://clash-lang.org/install/linux


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


## Executing Examples

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
./execute_PIR100.sh 
```


## Creating your own Haskell program and compiling it into a circuit

#### Encryption of Inputs

Create a secret key and a bootstrapping by using the keygen tool. Optionally specifiy a security parameter (default = 128) and a seed. It creates the files cloud.key and secret.key. 
Then encrypt your messages using the encrypt tool. It takes the parameters -b followed by the bitlength of your messages, -n followed by the the number of messages and then the messages itself, -key followed by the secret.key file, and -prefixout followed by a prefix for the ciphertext. The tool creates ciphertext files ct_in_0.data and counting. 
For messages of 128 bitlength and more the input is expected in hex format, the option -reverse can be given to encrypt the bits in reverse order. 

```
./build/keygen -lambda 128 -seed 571           
./build/encrypt -b 32 -n 2 25 70 -key secret.key -prefixout ct_in
```

#### Compiling and Running the program

Setup a new Haskell project using stack, the command below creates the example project simple from Clash in the my-clash-project directory.

```
stack new my-clash-project clash-lang/simple
```

Edit the source file my-clash-project/src/Example/Project.hs to contain your code. Don't remove the function topEntity. Clash expects this to be the entry point to your program.

Run the compilation tool that takes as input the program and a filename for the compiled circuit. If you changed the module name from Example.Project to something new, also provide the new module name. The first compilation will take additional time for setup.

```
./compile.sh my-clash-project/src/Example/Project.hs circuit.json [-module modulename]
```

The evaluation tool takes as input -c and the circuitfile, -n followed by the number of inputfiles and then the inputfiles themselves, -out and a file name for the result ciphertext, -b and the bitlength of the inputs or -bb and multiple numbers indicating the length of each input, -boot and the boostrapping key file, optionally -t and the number of threads to use.

```
./build/clash2tfhe -c tests/programs/ADD/add_32bit.json -n 2 ct_in_0.data ct_in_1.data -out result.data -boot boots.key -b 32 
```

#### Decryption

The decryption tool takes arguments -b and the bitlength, -key and the secret key file, and one or multiple ciphertexts.

```
./build/decrypt -b 32 -key secret.key result.data
```


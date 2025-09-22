#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

//function to encrypt a bit using secret key
void encrypt_bit(LweSample* ciphertext, int bit, TFheGateBootstrappingSecretKeySet* key) {
    bootsSymEncrypt(ciphertext, bit, key);
}

void print_error(const char *prog_name) {
    printf("Usage: %s -n <number of plaintexts> [<plaintext1> <plaintext2> ...] -b bits -key secret.key [-reverse] -prefixout fileprefix\n", prog_name);
}


int main(int argc, char *argv[]) {
    if (argc < 7) {
        print_error(argv[0]);
        return 1;
    }

    int n;
    char *secret_key_filename;

    int b;
    int reverse = 0;
    char *prefixout;
    int n_index = 0;


    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            n = atoi(argv[i + 1]);
            if (i + n >= argc) {
                fprintf(stderr, "Error: `-n` requires n messages.\n");
                print_error(argv[0]);
                return 0;
            }
            if (n < 0) {
                fprintf(stderr, "Error: Invalid number for `-n`: %s\n", argv[i + 1]);
                return 0;
            }
            n_index = i+2;
            i++;  // Skip past the integer we just read 
        }
        else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: `-b` requires an integer argument.\n");
                print_error(argv[0]);
                return 0;
            }
            b = atoi(argv[i + 1]);
            if (b <= 0) {
                fprintf(stderr, "Error: Invalid number of bits for `-b`: %s\n", argv[i + 1]);
                return 0;
            }
            i++;
        }
        else if (strcmp(argv[i], "-key") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: `-key` requires a filename argument.\n");
                print_error(argv[0]);
                return 0;
            }
            secret_key_filename = argv[i + 1];
            i++;
        }else if (strcmp(argv[i], "-reverse") == 0) {
            reverse = 1;
            i++;
        } else if(strcmp(argv[i], "-prefixout") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: `-prefix` requires a filename argument.\n");
                print_error(argv[0]);
                return 0;
            }
            prefixout = argv[i + 1];
            i++;
        }
        else if (argv[i][0] == '-') {
            /* Unknown flag */
            fprintf(stderr, "Error: Unknown option `%s`\n", argv[i]);
            print_error(argv[0]);
            return 0;
        }
    }


    FILE* secret_key = fopen(secret_key_filename, "rb");
    if (secret_key == NULL) {
                perror("Failed to open secret key file");
                return 1;
            }
    TFheGateBootstrappingSecretKeySet* sk = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);

    TFheGateBootstrappingParameterSet* params = sk->params;

    if (n > 0) {
        for (int i = 0; i < n; i++) {
            if(b<=64){
                uint64_t plaintext = strtoull(argv[n_index+i], NULL,10);
                LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(b, params);

                for (int j = 0; j < b; j++) {
                    int bit = (plaintext >> j) & 1; // LSB first ordering
                    // printf("%d",bit); 
                    encrypt_bit(&ciphertext[j], bit, sk);
                }

                char filename[256];
                sprintf(filename, "%s_%d.data",prefixout, i);
                FILE *ciphertext_file = fopen(filename, "wb");
                if (ciphertext_file == NULL) {
                    perror("Failed to open ciphertext file");
                    return 1;
                }
                printf("Encrypting = %lu in ct %s_%d \n", plaintext, prefixout, i);


                for (int j = 0; j < b; j++) {
                    export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &ciphertext[j], params);
                }

                fclose(ciphertext_file);

                delete_gate_bootstrapping_ciphertext_array(b, ciphertext);
            } else { // if b > 64 then input is expected in hex
                    if(reverse == 0){  // Standard: LSB first
                        char *hexStr = argv[n_index+i];
                        if ((hexStr[0] == '0') && (hexStr[1] == 'x' || hexStr[1] == 'X')) {
                            hexStr += 2;
                        }
                        size_t len = strlen(hexStr);
                        size_t chunks = (len + 15) / 16;
                        //printf("chunks: %ld , stringlength %ld\n", chunks, len);
                        size_t totalLen = chunks * 16;
                        if(totalLen != len) {
                            fprintf(stderr, "Error: Hexadecimal input must be a multiple of 16 characters.\n");
                            return 1;
                        }
                        uint64_t *hexValues = malloc(chunks * sizeof(uint64_t));
                        for (int j = 0; j < chunks; ++j) {
                            char buf[17] = {0};
                            memcpy(buf, hexStr+ j * 16, 16);
                            uint64_t v = strtoull(buf, NULL, 16);
                            hexValues[j] = v;
                            //printf("Chunk %d: 0x%016" PRIx64 "\n", j, v);
                        }
                        LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(b, params);
                        
                        int previousbits = 0;
                        for (int j = 1; j <= chunks; ++j) {
                            uint64_t plaintext = hexValues[chunks-j]; // start with last chunk to preserve LSB ordering
                            printf("Encrypting = 0x%016" PRIx64 " in ct %s_%d \n", plaintext, prefixout, i);
                            for (int k = 0; k < 64; k++) {
                                int bit = (plaintext >> k) & 1; 
                                //printf("%d",bit);
                                encrypt_bit(&ciphertext[k+previousbits], bit, sk);
                            }
                            previousbits += 64;

                        }

                    char filename[256];
                    sprintf(filename, "%s_%d.data",prefixout, i);
                    FILE *ciphertext_file = fopen(filename, "wb");
                    if (ciphertext_file == NULL) {
                        perror("Failed to open ciphertext file");
                        return 1;
                    }

                    for (int j = 0; j < b; j++) {
                        export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &ciphertext[j], params);
                    }

                    fclose(ciphertext_file);

                    delete_gate_bootstrapping_ciphertext_array(b, ciphertext);
                } else { // MSB
                    char *hexStr = argv[n_index+i];
                    if ((hexStr[0] == '0') && (hexStr[1] == 'x' || hexStr[1] == 'X')) {
                        hexStr += 2;
                    }
                    size_t len = strlen(hexStr);
                    size_t chunks = (len + 15) / 16; 
                    size_t totalLen = chunks * 16;
                    if(totalLen != len) {
                        fprintf(stderr, "Error: Hexadecimal input must be a multiple of 16 characters.\n");
                        return 1;
                    }


                    uint64_t *hexValues = malloc(chunks * sizeof(uint64_t));
                    for (int j = 0; j < chunks; ++j) {
                        char buf[17] = {0};
                        memcpy(buf, hexStr+ j * 16, 16);
                        uint64_t v = strtoull(buf, NULL, 16);
                        hexValues[j] = v;
                        //printf("Chunk %d: 0x%016" PRIx64 " (%" PRIu64 ")\n", j, v, v);
                    }
                    
                    LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(b, params);
                    int previousbits = 0;
                    for (int j = 0; j < chunks; ++j) {
                        uint64_t plaintext = hexValues[j]; // normal order
                        printf("Encrypting = 0x%016" PRIx64 " in ct %s_%d \n", plaintext, prefixout, i);
                        for (int k = 63; k >= 0; k--) { // this is reversed
                            int bit = (plaintext >> k) & 1;
                            //printf("%d",bit);
                            encrypt_bit(&ciphertext[63-k+previousbits], bit, sk);
                        }
                        previousbits += 64;
                    }
                    //printf("b: %d, encryptedbits: %d\n", b, previousbits);

                    char filename[256];
                    sprintf(filename, "%s_%d.data",prefixout, i);
                    FILE *ciphertext_file = fopen(filename, "wb");
                    if (ciphertext_file == NULL) {
                        perror("Failed to open ciphertext file");
                        return 1;
                    }

                    for (int j = 0; j < b; j++) {
                        export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &ciphertext[j], params);
                    }

                    fclose(ciphertext_file);

                    delete_gate_bootstrapping_ciphertext_array(b, ciphertext);
                            
                    free(hexValues);
                }
            }
        }
    }

    delete_gate_bootstrapping_secret_keyset(sk);
    delete_gate_bootstrapping_parameters(params);
    return 0;
}
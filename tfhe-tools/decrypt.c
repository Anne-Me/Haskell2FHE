#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

void print_error(const char *progname) {
    printf("Usage: decrypt %s -key <secret_keyfile> -b bits <ciphertext1.data> [<ciphertext2.data> ...]\n", progname);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_error(argv[0]);
        return 1;
    }

    char *secret_key_filename;
    int bits;
    int num_ciphertexts = argc - 5; 

    if (argc < 6) {
        print_error(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-key") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: `-key` requires a filename argument.\n");
                print_error(argv[0]);
                return 1;
            }
            secret_key_filename = argv[i + 1];
            i++;  /* Skip past the filename we just consumed */
        }
        else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: `-b` requires an integer argument.\n");
                print_error(argv[0]);
                return 1;
            }
            bits = atoi(argv[i + 1]);
            if (bits <= 0) {
                fprintf(stderr, "Error: Invalid number of bits for `-b`: %s\n", argv[i + 1]);
                return 1;
            }
            i++;
        }
        else if (argv[i][0] == '-') {
            /* Unknown flag */
            fprintf(stderr, "Error: Unknown option `%s`\n", argv[i]);
            print_error(argv[0]);
            return 1;
        }
    }




    // Read the secret key from file
    FILE *secret_key = fopen(secret_key_filename, "rb");
    if (secret_key == NULL) {
        perror("Failed to open secret key file");
        return 1;
    }
    TFheGateBootstrappingSecretKeySet *key = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);
    fclose(secret_key);

    const TFheGateBootstrappingParameterSet *params = key->params;

    for (int i = 0; i < num_ciphertexts; i++) {
        char *ciphertext_filename = argv[5 + i];
        FILE *ciphertext_file = fopen(ciphertext_filename, "rb");
        if (ciphertext_file == NULL) {
            perror("Failed to open ciphertext file");
            return 1;
        }

        LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(bits, params);
        for (int j = 0; j < bits; j++) {
            import_gate_bootstrapping_ciphertext_fromFile(ciphertext_file, &ciphertext[j], params);
        }
        fclose(ciphertext_file);

        if (bits <= 64){
            uint64_t plaintext = 0;
            for (int j = 0; j < bits; j++) {
                int bit = bootsSymDecrypt(&ciphertext[j], key);
                plaintext |= ((uint64_t )bit << j);
            }

            printf("Plaintext %d: %lu\n", i, plaintext);
        } else {
            int chunks = bits/64;
            //printf("chunks: %d \n", chunks);

            for(int k = chunks-1; k >=0 ; k--){
                uint64_t plaintext = 0;
                for (int j = 0; j < 64; j++) {
                    int bit = bootsSymDecrypt(&ciphertext[j+ k*64], key);
                    plaintext |= ((uint64_t )bit << j);
                }
                printf("part %d = 0x%016" PRIx64 "\n", chunks-k, plaintext);
            }
            
        }
        

        delete_gate_bootstrapping_ciphertext_array(bits, ciphertext);
    }

    delete_gate_bootstrapping_secret_keyset(key);
    return 0;
}


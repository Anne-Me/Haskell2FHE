#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

void print_error(const char *prog_name) {
    printf("Usage: %s [-lambda 128] [-seed 32091] \n", prog_name);
}

int main(int argc, char *argv[]) {

    int minimum_lambda = 128; // default
    uint32_t seed[] = { 325, 3348, 84982 };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-lambda") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Error: `-lambda` requires an integer argument.\n");
                    print_error(argv[0]);
                    return 0;
                }
                minimum_lambda = atoi(argv[i + 1]);
                i++;
            }
        else if (strcmp(argv[i], "-seed") == 0) {
            if (i + 1 >= argc) {
                    fprintf(stderr, "Error: `-seed` requires an integer argument.\n");
                    print_error(argv[0]);
                    return 0;
                }
                seed[2] = atoi(argv[i + 1]); // replace third seed 
                i++;
        }
    }

    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(minimum_lambda);

    //generate a random key

    tfhe_random_generator_setSeed(seed,3);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);

    FILE* secret_key = fopen("secret.key","wb");
    export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
    fclose(secret_key);

    FILE* cloud_key = fopen("boots.key","wb");
    export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
    fclose(cloud_key);
   
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);

}

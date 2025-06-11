#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>


//function to encrypt a bit using secret key
void encrypt_bit(LweSample* ciphertext, int bit, TFheGateBootstrappingSecretKeySet* key) {
    bootsSymEncrypt(ciphertext, bit, key);
}


int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Usage: %s -n <number of plaintexts> [<plaintext1> <plaintext2> ...] -key secret.key cloud.key\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[2]);
    char *secret_key_filename = argv[argc - 2];
    char *cloud_key_filename = argv[argc - 1];


    if ((n > 0 && argc != n + 6) || (n == 0 && argc != 6)) {
        printf("Number of plaintexts does not match the given number %d\n", n);
        return 1;
    }


     const int minimum_lambda = 110;
    // Generate parameters
    TFheGateBootstrappingParameterSet *params = new_default_gate_bootstrapping_parameters(minimum_lambda);
    uint32_t seed[] = {314, 1592, 657};
    tfhe_random_generator_setSeed(seed, 3);


    // Generate keys
    TFheGateBootstrappingSecretKeySet *key = new_random_gate_bootstrapping_secret_keyset(params);

  
    // Export secret key to file
    FILE *secret_key = fopen(secret_key_filename, "wb");
    export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
    fclose(secret_key);

    
    // Export cloud key to file
    FILE *cloud_key = fopen(cloud_key_filename, "wb");
    export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
    fclose(cloud_key);

    if (n > 0) {
        for (int i = 0; i < n; i++) {
            int plaintext = atoi(argv[3 + i]);
            LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(8, params);

            for (int j = 0; j < 8; j++) {
                int bit = (plaintext >> j) & 1;
                encrypt_bit(&ciphertext[j], bit, key);
            }

            char filename[256];
            sprintf(filename, "ciphertext%d.data", i);
            FILE *ciphertext_file = fopen(filename, "wb");
            if (ciphertext_file == NULL) {
                perror("Failed to open ciphertext file");
                return 1;
            }

            for (int j = 0; j < 8; j++) {
                export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &ciphertext[j], params);
            }

            fclose(ciphertext_file);

            delete_gate_bootstrapping_ciphertext_array(8, ciphertext);
        }
    }

    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);
    return 0;
}

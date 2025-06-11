#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>

void encrypt_bit(LweSample* ciphertext, int bit, TFheGateBootstrappingSecretKeySet* key) {
    bootsSymEncrypt(ciphertext, bit, key);
}

int main(int argc, char *argv[]) {
    if (argc < 8) {
        printf("Usage: %s -n <number of plaintexts> [<plaintext1> <plaintext2> ...] -secret <secret_keyfile> -cloud <cloud_keyfile>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[2]);
    char *secret_key_filename = argv[argc - 3];
    char *cloud_key_filename = argv[argc - 1];

    if ((n > 0 && argc != n + 7) || (n == 0 && argc != 7)) {
        printf("Number of plaintexts does not match the given number %d\n", n);
        return 1;
    }

    // Read the secret key from file
    FILE *secret_key_file = fopen(secret_key_filename, "rb");
    if (secret_key_file == NULL) {
        perror("Failed to open secret key file");
        return 1;
    }
    TFheGateBootstrappingSecretKeySet *key = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key_file);
    fclose(secret_key_file);

    const TFheGateBootstrappingParameterSet *params = key->params;

    // Read the cloud key from file
    FILE *cloud_key_file = fopen(cloud_key_filename, "rb");
    if (cloud_key_file == NULL) {
        perror("Failed to open cloud key file");
        return 1;
    }
    TFheGateBootstrappingCloudKeySet *cloud_key = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key_file);
    fclose(cloud_key_file);

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
    delete_gate_bootstrapping_cloud_keyset(cloud_key);
    return 0;
}

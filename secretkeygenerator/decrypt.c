#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s -key <secret_keyfile> <ciphertext1.data> [<ciphertext2.data> ...]\n", argv[0]);
        return 1;
    }

    char *secret_key_filename = argv[2];
    int num_ciphertexts = argc - 3;

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
        char *ciphertext_filename = argv[3 + i];
        FILE *ciphertext_file = fopen(ciphertext_filename, "rb");
        if (ciphertext_file == NULL) {
            perror("Failed to open ciphertext file");
            return 1;
        }

        LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(8, params);
        for (int j = 0; j < 8; j++) {
            import_gate_bootstrapping_ciphertext_fromFile(ciphertext_file, &ciphertext[j], params);
        }
        fclose(ciphertext_file);

        uint32_t plaintext = 0;
        for (int j = 0; j < 8; j++) {
            int bit = bootsSymDecrypt(&ciphertext[j], key);
            plaintext |= (bit << j);
        }

        printf("Plaintext %d: %u\n", i, plaintext);

        delete_gate_bootstrapping_ciphertext_array(8, ciphertext);
    }

    delete_gate_bootstrapping_secret_keyset(key);
    return 0;
}

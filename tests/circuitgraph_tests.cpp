#include <gtest/gtest.h>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

TEST(TfheCircuitTest, XORGateBehavior) {
    int minimum_lambda = 128;
    uint32_t seed[] = { 325, 3348, 84982 };

    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(minimum_lambda);
    tfhe_random_generator_setSeed(seed, 3);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);
    const TFheGateBootstrappingCloudKeySet *bk = &key->cloud;

    LweSample *ciphertext = new_gate_bootstrapping_ciphertext_array(5, params);

    // Inputs
    bootsSymEncrypt(&ciphertext[0], 0, key); // 0
    bootsSymEncrypt(&ciphertext[1], 1, key); // 1
    bootsSymEncrypt(&ciphertext[2], 0, key); // 0
    bootsSymEncrypt(&ciphertext[3], 1, key); // 1

    // 0 ^ 1 = 1
    bootsXOR(&ciphertext[4], &ciphertext[0], &ciphertext[1], bk);
    int bit = bootsSymDecrypt(&ciphertext[4], key);
    EXPECT_EQ(bit, 1);

    // 0 ^ 0 = 0
    bootsXOR(&ciphertext[4], &ciphertext[0], &ciphertext[2], bk);
    bit = bootsSymDecrypt(&ciphertext[4], key);
    EXPECT_EQ(bit, 0);

    // 1 ^ 1 = 0
    bootsXOR(&ciphertext[4], &ciphertext[1], &ciphertext[3], bk);
    bit = bootsSymDecrypt(&ciphertext[4], key);
    EXPECT_EQ(bit, 0);

    // 1 ^ 0 = 1
    bootsXOR(&ciphertext[4], &ciphertext[1], &ciphertext[2], bk);
    bit = bootsSymDecrypt(&ciphertext[4], key);
    EXPECT_EQ(bit, 1);

    delete_gate_bootstrapping_ciphertext_array(5, ciphertext);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);
}
#include <gtest/gtest.h>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

TEST(TfheLib, XORGate) {
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


TEST(TfheLib, XNORGate) {
    int minimum_lambda = 128;
    uint32_t seed[] = { 325, 3348, 84982 };

    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(minimum_lambda);
    tfhe_random_generator_setSeed(seed, 3);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);
    const TFheGateBootstrappingCloudKeySet* bk = &key->cloud;

    LweSample* ciphertext = new_gate_bootstrapping_ciphertext_array(3, params);

    // Inputs
    bootsSymEncrypt(&ciphertext[0], 0, key); // a=0
    bootsSymEncrypt(&ciphertext[1], 1, key); // b=1

    // 0 XNOR 0 = 1
    bootsXNOR(&ciphertext[2], &ciphertext[0], &ciphertext[0], bk);
    int bit = bootsSymDecrypt(&ciphertext[2], key);
    EXPECT_EQ(bit, 1);

    // 0 XNOR 1 = 0
    bootsXNOR(&ciphertext[2], &ciphertext[0], &ciphertext[1], bk);
    bit = bootsSymDecrypt(&ciphertext[2], key);
    EXPECT_EQ(bit, 0);

    // 1 XNOR 0 = 0
    bootsXNOR(&ciphertext[2], &ciphertext[1], &ciphertext[0], bk);
    bit = bootsSymDecrypt(&ciphertext[2], key);
    EXPECT_EQ(bit, 0);

    // 1 XNOR 1 = 1
    bootsXNOR(&ciphertext[2], &ciphertext[1], &ciphertext[1], bk);
    bit = bootsSymDecrypt(&ciphertext[2], key);
    EXPECT_EQ(bit, 1);

    // cleanup
    delete_gate_bootstrapping_ciphertext_array(3, ciphertext);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);
}

TEST(TfheLib, MUXGate) {
    // Setup parameters and keys
    const int minimum_lambda = 128;
    uint32_t seed[] = {325, 3348, 84982};

    TFheGateBootstrappingParameterSet* params =
        new_default_gate_bootstrapping_parameters(minimum_lambda);

    tfhe_random_generator_setSeed(seed, 3);
    TFheGateBootstrappingSecretKeySet* key =
        new_random_gate_bootstrapping_secret_keyset(params);
    const TFheGateBootstrappingCloudKeySet* bk = &key->cloud;

    // Allocate ciphertexts: [0]=s, [1]=t, [2]=f, [3]=out
    LweSample* ct = new_gate_bootstrapping_ciphertext_array(4, params);

    // Helper lambda to run one MUX case and check expected bit
    auto check_mux = [&](int s, int a, int b, int expected) {
        bootsSymEncrypt(&ct[0], s, key); // selector s
        bootsSymEncrypt(&ct[1], a, key); // true-branch t
        bootsSymEncrypt(&ct[2], b, key); // false-branch f
        bootsMUX(&ct[3], &ct[0], &ct[1], &ct[2], bk);
        int bit = bootsSymDecrypt(&ct[3], key);
        EXPECT_EQ(bit, expected);
    };

    // Truth-table checks: result = s ? t : f
    check_mux(0, 0, 0, 0); // s=0 -> pick f=0
    check_mux(0, 0, 1, 1); // s=0 -> pick f=1
    check_mux(0, 1, 0, 0); // s=0 -> pick f=0
    check_mux(0, 1, 1, 1); // s=0 -> pick f=1

    check_mux(1, 0, 0, 0); // s=1 -> pick t=0
    check_mux(1, 0, 1, 0); // s=1 -> pick t=0
    check_mux(1, 1, 0, 1); // s=1 -> pick t=1
    check_mux(1, 1, 1, 1); // s=1 -> pick t=1

    // Cleanup
    delete_gate_bootstrapping_ciphertext_array(4, ct);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);
}
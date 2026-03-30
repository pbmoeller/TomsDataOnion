#include "aes.hpp"

#include <openssl/evp.h>

#include <algorithm>
#include <stdexcept>

int unwrap(uint8_t *wrappedKey, int wrappedKeyLength, uint8_t *kek, uint8_t *kekIv, uint8_t *unwrappedKey)
{
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintextLength;

    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_wrap(), nullptr, kek, kekIv);

    EVP_DecryptUpdate(ctx, unwrappedKey, &len, wrappedKey, wrappedKeyLength);
    plaintextLength = len;

    EVP_DecryptFinal_ex(ctx, unwrappedKey + len, &len);
    plaintextLength += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintextLength;
}

std::vector<uint8_t> aes_ctr_decrypt(const std::vector<uint8_t>& ciphertext,
                                     const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& iv)
{
    if(key.size() != 32) {
        throw std::runtime_error("Key must be 32 bytes for 256-bit AES.");
    }
    if(iv.size() != 16) {
        throw std::runtime_error("IV must be 16 bytes for CTR");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        throw std::runtime_error("Failed to create context");
    }

    std::vector<uint8_t> plaintext(ciphertext.size());
    int len = 0;
    int plaintextLength = 0;

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        std::runtime_error("DecryptInit failed.");
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if(EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        std::runtime_error("DecryptUpdate failed.");
    }
    plaintextLength = len;

    if(EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        std::runtime_error("DecryptFinal failed.");
    }
    plaintextLength += len;

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(plaintextLength);
    return plaintext;
}

std::vector<uint8_t> aes_decode(const std::vector<uint8_t> &inVec)
{
    std::vector<uint8_t> kek(32, 0);
    std::copy(inVec.data(), inVec.data() + 32, kek.begin());

    std::vector<uint8_t> key_iv(8, 0);
    std::copy(&inVec[32], &inVec[40], key_iv.begin());

    std::vector<uint8_t> key(40, 0);
    std::copy(&inVec[40], &inVec[80], key.begin());

    std::vector<uint8_t> payload_iv(16, 0);
    std::copy(&inVec[80], &inVec[96], payload_iv.begin());

    std::vector<uint8_t> payload(inVec.size() - 96, 0);
    std::copy(inVec.begin() + 96, inVec.end(), payload.begin());

    std::vector<uint8_t> unwrappedKey(32, 0);
    int len = unwrap(key.data(), key.size(), kek.data(), key_iv.data(), unwrappedKey.data());

    std::vector<uint8_t> result = aes_ctr_decrypt(payload, unwrappedKey, payload_iv);

    return result;
}
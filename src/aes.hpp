#ifndef TOMSDATAONION_AES_H
#define TOMSDATAONION_AES_H

#include <vector>
#include <cstdint>

std::vector<uint8_t> aes_decode(const std::vector<uint8_t> &inVec);

#endif // TOMSDATAONION_AES_H
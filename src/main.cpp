#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

#include "aes.hpp"
#include "ascii85.h"
#include "bitwise.h"
#include "network.h"

static std::map<int, std::string> layerMap = {
    {0, "data/layer0_payload.txt"},
    {1, "data/layer1_payload.txt"},
    {2, "data/layer2_payload.txt"},
    {3, "data/layer3_payload.txt"},
    {4, "data/layer4_payload.txt"},
    {5, "data/layer5_payload.txt"},
};

std::string readPayloadFromFile(std::string fileName)
{
    // Read file into filestream
    std::stringstream strm;
    std::ifstream file(fileName);
    strm << file.rdbuf();
    file.close();

    return strm.str();
}

std::vector<uint8_t> getEncodedLayer(int layer)
{
    auto it = layerMap.find(layer);
    if(it != layerMap.end()) {
        // get filename to layer
        std::string fileName = it->second;
        // get encoded payload from file
        std::string encoded_payload = readPayloadFromFile(fileName);
        // decode the payload
        return ascii85_decode(std::vector<uint8_t>(encoded_payload.begin(),
                                                   encoded_payload.end()));
    } else {
        return std::vector<uint8_t>();
    }
}

int main(int argc, char **argv)
{
    const int layer = 5;

    // Get encoded payload from file
    auto encoded_payload = getEncodedLayer(layer);

    std::vector<uint8_t> encoded_data;

    if(layer == 0) {
        encoded_data = encoded_payload;
    } else if(layer == 1) {
        // decode layer 1
        encoded_data = bitwise_decode(encoded_payload);
    } else if(layer == 2) {
        // decode layer 2
        encoded_data = parity_decode(encoded_payload);
    } else if(layer == 3) {
        // decode layer 3
        encoded_data = xor_decode(encoded_payload);
    } else if(layer == 4) {
        // decode layer 4
        encoded_data = network_decode(encoded_payload);
    } else if(layer == 5) {
        // decode layer 5
        encoded_data = aes_decode(encoded_payload);
    }

    // Print Layer
    std::cout << std::string(encoded_data.begin(), encoded_data.end()) << std::endl;

    return 0;
}



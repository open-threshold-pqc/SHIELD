#include <iostream>
#include "dilithium/fips202.h"
#include <iomanip>
#include <bitset>
#include "dilithium/fips202.h"

#define KECCAK_STATE_SIZE (135)

int main(){

    keccak_state state;

    uint8_t input[KECCAK_STATE_SIZE] = {0};
    uint8_t output[KECCAK_STATE_SIZE] = {0};

    shake256_init(&state);
    shake256_absorb(&state, input, KECCAK_STATE_SIZE);
    shake256_finalize(&state);


    for (int i = 0; i < 25; i++) {
        std::bitset<64> bits(state.s[i]);  // Convert each uint64_t to a bitset of 64 bits
        std::cout << "s[" << i << "]: " << bits << std::endl;
    }
    std::cout << "\n";

    // shake256_squeeze(output, 136, &state);
    //
    // for (size_t i = 0; i < 25; i++) {
    //     std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0')
    //               << state.s[i] << " ";
    //
    //     if ((i + 1) % 5 == 0) std::cout << "\n";  // Format as 5x5 matrix
    // }
    // std::cout << state.s[0] << std::endl;

    return 0;
}
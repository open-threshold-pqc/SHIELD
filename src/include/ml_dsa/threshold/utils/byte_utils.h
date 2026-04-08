#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <memory>

#include "mpc/sharing/boolean_sharing.h"
#include "math/number.h"

namespace otpqc::threshold_signatures::utils {
    /**
     * \brief Given a byte array as input, it creates (boolean) shares of it stored as an array of bytes.
     * \param byte_array Byte array consisting of bytes of a secret
     * \param byte_array_length Number of bytes in the key array
     * \param number_of_shares Number of parties that we want to share the key among
     * \return Array of share bytes for each party
     */
    inline std::vector<std::unique_ptr<uint8_t[]> > boolean_distribute_byte_array(
        const uint8_t *byte_array, const int byte_array_length, const int number_of_shares) {
        std::vector<std::unique_ptr<uint8_t[]> > byte_array_shares;
        byte_array_shares.reserve(number_of_shares);

        /* Allocate memory for each party's share */
        for (int p = 0; p < number_of_shares; ++p)
            byte_array_shares.emplace_back(std::make_unique<uint8_t[]>(byte_array_length));

        /* Generate boolean shares for each byte */
        //todo improve by packing more (upto sizeof(base)) |= for number class
        for (int i = 0; i < byte_array_length; ++i) {
            math::Number<> input_byte{byte_array[i]};
            auto shares = mpc::sharing::BooleanSharing<>::generate_random_shares(
                input_byte, 8, number_of_shares);
            for (int p = 0; p < number_of_shares; ++p) {
                byte_array_shares[p][i] = shares[p].get_share().get_value();
            }
        }
        return byte_array_shares;
    }

    /**
     * \brief Converts an array of unsigned bytes to a vector of bools in Big Endian order.
     * \param byte_array Pointer to the array of bytes
     * \param byte_array_length Number of bytes in the array
     * \return Vector of bools representing the bits of the array bytes
     */
    inline std::vector<bool> byte_to_vector_bool_be(const uint8_t *byte_array, const int byte_array_length) {
        std::vector<bool> bits;
        bits.reserve(byte_array_length * 8);

        for (size_t i = 0; i < byte_array_length; ++i) {
            uint8_t byte = byte_array[i];
            for (int bit = 7; bit >= 0; --bit) {
                bits.push_back((byte >> bit) & 0x01);
            }
        }

        return bits;
    }
}
#endif

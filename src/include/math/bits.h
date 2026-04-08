#ifndef BITS_H
#define BITS_H

#include "math/number.h"
#include <vector>
#include <cstdint>

namespace otpqc::math {
    /**
     * \brief This function converts a vector of bools to a vector of bytes (LSB last)
     * \param bits Vector of bools (bits)
     * \return Vector of bytes
     */
    static std::vector<uint8_t> bits_to_bytes_be(const std::vector<bool> &bits) noexcept {
        std::size_t bit_count = bits.size();
        std::size_t padded_bit_count = ((bit_count + 7) / 8) * 8; // next multiple of 8
        std::size_t padding = padded_bit_count - bit_count;

        std::vector<uint8_t> bytes(padded_bit_count / 8, 0);

        for (std::size_t i = 0; i < bit_count; ++i) {
            std::size_t total_bit_index = i + padding; // shifted to right for left padding
            std::size_t byte_index = total_bit_index / 8;
            std::size_t bit_pos_in_byte = 7 - (total_bit_index % 8); // MSB-first

            if (bits[i]) {
                bytes[byte_index] |= static_cast<uint8_t>(1u << bit_pos_in_byte);
            }
        }

        return bytes;
    }

    /**
     * \brief This function converts a vector of bools to a vector of bytes (LSB first)
     * \param bits Vector of bools (bits)
     * \return Vector of bytes
     */
    static std::vector<uint8_t> bits_to_bytes_le(const std::vector<bool> &bits) noexcept {
        auto bytes = bits_to_bytes_be(bits);
        std::ranges::reverse(bytes);
        return bytes;
    }

    /**
     * \brief This function returns the true (minimum) bits required for representing the number
     * \param number Input number
     * \return Minimum bits for representing the given number
     */
    template<IntegralNumeric T>
    static int true_bit_length(T number) {
        if constexpr (GmpNumeric<T>) {
            return mpz_sizeinbase(number.get_mpz_t(), 2);
        } else {
            using U = std::make_unsigned_t<T>;
            U uval = static_cast<U>(number);

            if constexpr (std::is_signed_v<T>) {
                if (number < 0) {
                    uval = ~static_cast<U>(number); // two's complement magnitude
                }
            }
            return uval == 0 ? 1 : std::bit_width(uval);
        }
    }
}


#endif //BITS_H

#ifndef BOOLEAN_SHARING_H
#define BOOLEAN_SHARING_H

#include "math/number.h"
#include "math/bits.h"

#include <vector>


/**
 * \brief Contain various sharing classes such as Arithmetic and Boolean sharing
 */
namespace otpqc::mpc::sharing {

    template<otpqc::math::IntegralNumeric T>
    class BooleanSharing;

    template <otpqc::math::IntegralNumeric T>
    using BooleanShareVec = std::vector<BooleanSharing<T>>;

    /**
     * \class BooleanSharing
     * \brief Boolean Sharing Over a Binary Field
     *
     * This class demonstrates boolean sharing for secure multi-party computation.
     * It allows binary field operations while maintaining data privacy.
     *
     * \subsection Usage Usage
     * Learn how to work with Boolean shares by following the tutorial examples:
     * \include tutorial_boolean_sharing.cpp
     */
    template <otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class BooleanSharing {
    public:
        explicit BooleanSharing(const int bit_length) {
            if (bit_length <= 0)
                throw std::invalid_argument("[MPC::BooleanSharing] Invalid bit length (Should > 0)");
            m_bit_length = bit_length;
        }

        explicit BooleanSharing(const otpqc::math::Number<T> &number, const int bit_length)
            : BooleanSharing(bit_length){

            const int number_true_bit_length = math::true_bit_length(number.get_value());

            if constexpr (otpqc::math::PrimitiveNumeric<T>) {
                if (bit_length > sizeof(T) * 8)
                    throw std::invalid_argument("[MPC::BooleanSharing] Invalid bit length (Should <= sizeof(T) * 8)");
            }
            if (bit_length < number_true_bit_length)
                throw std::invalid_argument("[MPC::BooleanSharing] Invalid bit length (Should >= number's true bit length)");
            m_share = number;
        }

        explicit BooleanSharing(const std::vector<bool> &bits) {
            if (bits.empty())
                throw std::invalid_argument("[MPC::BooleanSharing] Invalid bit length (Vector.size() > 0)");

            auto byte_vector = math::bits_to_bytes_be(bits);
            m_share = otpqc::math::Number<T>(byte_vector);
            m_bit_length = static_cast<int>(bits.size());
        }

        BooleanSharing(const BooleanSharing &other) = default;

        BooleanSharing(BooleanSharing &&other) = default;

        BooleanSharing &operator=(const BooleanSharing &other) = default;

        BooleanSharing &operator=(BooleanSharing &&other) noexcept = default;

        ~BooleanSharing() = default;

        const otpqc::math::Number<T> &get_share() const {
            return m_share;
        }

        [[nodiscard]] int get_length() const {
            return m_bit_length;
        }

        [[nodiscard]] bool is_zero() const {
            return m_share == 0;
        }

        [[nodiscard]] bool is_one() const {
            return m_share == 1;
        }

        [[nodiscard]] std::vector<bool> bits_le() const {
            auto bits = m_share.bits_le_ze(m_bit_length);
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_be() const {
            auto bits = m_share.bits_be_ze(m_bit_length);
            return bits;
        }

        static BooleanShareVec<T> generate_random_shares(const otpqc::math::Number<T> &number,
                                                                const int bit_length, const int count) {
            BooleanShareVec<T> shares;
            shares.reserve(count);

            /* Check if the given number fits in the provided bit length */
            const BooleanSharing number_share{number, bit_length};

            otpqc::math::Number<T> xor_sum {};
            for (int i = 0; i < count - 1; ++i) {
                auto random_share = otpqc::math::Number<T>::random(bit_length);
                xor_sum ^= random_share;
                shares.emplace_back(random_share, bit_length);
            }
            shares.emplace_back(number_share.get_share() ^ xor_sum, bit_length);

            return shares;
        }

    private:
        otpqc::math::Number<T> m_share{};    /* Boolean share of the number stored as Number<T> */

        int m_bit_length {};    /* Bit-length (minimum) of the share */
    };
}


#endif

#ifndef DABIT_H
#define DABIT_H

#include "mpc/sharing/arithmetic_sharing.h"
#include "mpc/sharing/boolean_sharing.h"
#include "math/number.h"

/**
 * Containing MPC functionalities
 */
namespace otpqc::mpc {
    /**
     * \brief Helper struct to hod daBit shares for all bits. Hence, each struct is for a shareholder.
     * \tparam T Template parameter defining underlying numerical type
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    struct DabitShares {
        sharing::ArithmeticShareVec<T> a_shares; /* Arithmetic shares of each bit */
        sharing::BooleanShareVec<T> b_shares; /* Boolean shares of each bit */
        int length{}; /* Length of the dabit */
        otpqc::math::Number<T> modulus; /* Modulus of the daBit */

        DabitShares(const sharing::ArithmeticShareVec<T> &arithmetic_shares,
                    const sharing::BooleanShareVec<T> &boolean_shares, otpqc::math::Number<T> dabit_modulus,
                    const int dabit_length): a_shares(arithmetic_shares), b_shares(boolean_shares),
                                             length(dabit_length), modulus(dabit_modulus) {
        }
    };

    /**
     * \brief Doubly authenticated bit (daBit)
     *
     * \subsection Tutorial Tutorial
     * Following tutorial shows how to work with daBits in QST.
     * \include tutorial_dabit.cpp
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class Dabit {
    public:
        Dabit() = delete;

        explicit Dabit(int value) = delete;

        Dabit(const Dabit &other) = delete;

        Dabit(const otpqc::math::Number<T> &modulus, const otpqc::math::Number<T> &key, const int bit_length, const int share_count):
        m_bit_length(bit_length), m_share_count(share_count), m_modulus(modulus) {
            /* Check if the bit length does not exceed the modulus */
            if (bit_length > otpqc::math::true_bit_length(modulus.get_value()))
                throw std::runtime_error("[MPC::Dabit] Bit length exceeds the modulus");

            if (modulus < 0 || key < 0)
                throw std::runtime_error("[MPC::Dabit] Modulus/Key must be positive");

            if (key >= modulus)
                throw std::runtime_error("[MPC::Dabit] Key must be less than the modulus");

            /* Generate a random number */
            auto dabit_value = otpqc::math::Number<T>::random(modulus);

            /* Generate the shares for each bit starting from LSB */
            auto dabit_bits = dabit_value.bits_le_ze(m_bit_length);

            m_arithmetic_shares.resize(m_share_count);
            m_boolean_shares.resize(m_share_count);

            for (int i = 0; i < m_share_count; i++) {
                m_arithmetic_shares[i].reserve(m_bit_length);
                m_boolean_shares[i].reserve(m_bit_length);
            }

            for (int i = 0; i < m_bit_length; i++) {
                const auto target_number = dabit_bits[i] == true ? otpqc::math::Number<T>{1} : otpqc::math::Number<T>{0};

                auto dabit_a_shares = otpqc::mpc::sharing::ArithmeticSharing<T>::generate_random_shares(
                    modulus, target_number, key, m_share_count);
                auto dabit_b_shares = otpqc::mpc::sharing::BooleanSharing<T>::generate_random_shares(
                    target_number, 1, m_share_count);

                for (int j = 0; j < m_share_count; j++) {
                    m_arithmetic_shares[j].emplace_back(dabit_a_shares[j].get_modulus(),
                                                        dabit_a_shares[j].get_share(),
                                                        dabit_a_shares[j].get_mac_share());
                    m_boolean_shares[j].emplace_back(dabit_b_shares[j].get_share(), 1);
                }
            }
        }

        ~Dabit() = default;

        [[nodiscard]] auto get_dabit_shares() const {
            std::vector<DabitShares<T> > dabit_shares;
            dabit_shares.reserve(m_share_count);
            for (int i = 0; i < m_share_count; i++)
                dabit_shares.emplace_back(m_arithmetic_shares[i], m_boolean_shares[i], m_modulus, m_bit_length);
            return dabit_shares;
        }

    private:
        int m_bit_length{}; /* daBit bit length */
        int m_share_count{}; /* Number of shares(parties) we want to share each bit among */
        otpqc::math::Number<T> m_modulus{}; /* Modulus of the daBit */

        /* Arithmetic and Boolean shares of each bit. Each element of these vectors are shares of all bits for
         * a shareholder.
         */
        std::vector<sharing::ArithmeticShareVec<T> > m_arithmetic_shares;

        std::vector<sharing::BooleanShareVec<T> > m_boolean_shares;
    };
}


#endif

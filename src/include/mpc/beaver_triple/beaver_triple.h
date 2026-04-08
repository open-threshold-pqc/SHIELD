#ifndef BEAVER_TRIPLET_HPP
#define BEAVER_TRIPLET_HPP

#include "mpc/sharing/arithmetic_sharing.h"
#include "math/number.h"
#include <vector>


/**
 * Containing MPC functionalities
 */
namespace otpqc::mpc {
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    struct BeaverTripleShares {
        explicit BeaverTripleShares(
            otpqc::math::Number<T> a_share,
            otpqc::math::Number<T> b_share,
            otpqc::math::Number<T> c_share,
            otpqc::math::Number<T> a_mac_share,
            otpqc::math::Number<T> b_mac_share,
            otpqc::math::Number<T> c_mac_share) : a_share(std::move(a_share)),
                                     b_share(std::move(b_share)),
                                     c_share(std::move(c_share)),
                                     a_mac_share(std::move(a_mac_share)),
                                     b_mac_share(std::move(b_mac_share)),
                                     c_mac_share(std::move(c_mac_share)) {
        }

        otpqc::math::Number<T> a_share;
        otpqc::math::Number<T> b_share;
        otpqc::math::Number<T> c_share;
        otpqc::math::Number<T> a_mac_share;
        otpqc::math::Number<T> b_mac_share;
        otpqc::math::Number<T> c_mac_share;
    };

    /**
     * \brief Beaver Triple
     *
     * \subsection Tutorial Tutorial
     * Following tutorial shows how to generate Beaver Triples in QST.
     * \include tutorial_beaver_triple.cpp
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class BeaverTriple {
    public:
        BeaverTriple(const otpqc::math::Number<T> &modulus, const otpqc::math::Number<T> &global_mac_key, int share_count) {
            m_a = otpqc::math::Number<T>::random(modulus);
            m_b = otpqc::math::Number<T>::random(modulus);
            m_c = (m_a * m_b) % modulus;

            auto a_shares = otpqc::mpc::sharing::ArithmeticSharing<T>::generate_random_shares(
                modulus, m_a, global_mac_key, share_count);
            auto b_shares = otpqc::mpc::sharing::ArithmeticSharing<T>::generate_random_shares(
                modulus, m_b, global_mac_key, share_count);
            auto c_shares = otpqc::mpc::sharing::ArithmeticSharing<T>::generate_random_shares(
                modulus, m_c, global_mac_key, share_count);
            m_abc_arithmetic_shares.reserve(share_count);
            for (int i = 0; i < share_count; ++i) {
                m_abc_arithmetic_shares.emplace_back(
                    std::move(a_shares[i].get_share()),
                    std::move(b_shares[i].get_share()),
                    std::move(c_shares[i].get_share()),
                    std::move(a_shares[i].get_mac_share()),
                    std::move(b_shares[i].get_mac_share()),
                    std::move(c_shares[i].get_mac_share()));
            }
        }

        ~BeaverTriple() = default;

        [[nodiscard]] const std::vector<BeaverTripleShares<T>> &get_shares() const {
            return m_abc_arithmetic_shares;
        }

    private:
        /*  m_c = m_a * m_b mod m_modulus */
        otpqc::math::Number<T> m_a;
        otpqc::math::Number<T> m_b;
        otpqc::math::Number<T> m_c;
        otpqc::math::Number<T> m_a_mac;
        otpqc::math::Number<T> m_b_mac;
        otpqc::math::Number<T> m_c_mac;

        std::vector<BeaverTripleShares<T>> m_abc_arithmetic_shares;
    };;
}

#endif
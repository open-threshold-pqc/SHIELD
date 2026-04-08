#ifndef ARITHMETIC_SHARING_H
#define ARITHMETIC_SHARING_H

#include "math/number.h"
#include <vector>


/**
 * \brief Contain various sharing classes such as Arithmetic and Boolean sharing
 */
namespace otpqc::mpc::sharing {
    template<otpqc::math::IntegralNumeric T>
    class ArithmeticSharing;

    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using ArithmeticShareVec = std::vector<ArithmeticSharing<T> >;

    /**
     * \class ArithmeticSharing
     * \brief Arithmetic Sharing Over a Finite Field
     *
     * This class illustrates how to perform arithmetic sharing for secure multi-party computation using
     * a finite field. It ensures data privacy while allowing secure operations across multiple parties.
     *
     * \subsection Usage
     * Learn how to work with Arithmetic shares by following the following examples:
     * \include tutorial_arithmetic_sharing.cpp
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class ArithmeticSharing {
    public:
        ArithmeticSharing(const otpqc::math::Number<T> &modulus, const otpqc::math::Number<T> &share, const otpqc::math::Number<T> &mac) {
            if (modulus <= 0)
                throw std::invalid_argument("[MPC::ArithmeticSharing] Invalid modulus (Should > 0)");
            if (share < 0 || share >= modulus)
                throw std::invalid_argument("[MPC::ArithmeticSharing] Share should be in range [0, modulus-1]");
            if (mac < 0 || mac >= modulus)
                throw std::invalid_argument("[MPC::ArithmeticSharing] Mac should be in range [0, modulus-1]");
            m_modulus = modulus;
            m_share = share;
            m_mac = mac;
        }

        ArithmeticSharing(const ArithmeticSharing &) = default;

        ArithmeticSharing(ArithmeticSharing &&) noexcept = default;

        ArithmeticSharing &operator=(const ArithmeticSharing &) = default;

        ArithmeticSharing &operator=(ArithmeticSharing &&) noexcept = default;

        const otpqc::math::Number<T> &get_share() const { return m_share; }
        const otpqc::math::Number<T> &get_mac_share() const { return m_mac; }
        const otpqc::math::Number<T> &get_modulus() const { return m_modulus; }

        void set_mac_share(const otpqc::math::Number<T>& mac) { m_mac = mac; }

        ArithmeticSharing operator+(const ArithmeticSharing &rhs) const {
            return ArithmeticSharing(
                m_modulus,
                (m_share + rhs.m_share) % m_modulus,
                (m_mac + rhs.m_mac) % m_modulus
            );
        }

        ArithmeticSharing &operator+=(const ArithmeticSharing &rhs) {
            m_share = (m_share + rhs.m_share) % m_modulus;
            m_mac = (m_mac + rhs.m_mac) % m_modulus;
            return *this;
        }

        ArithmeticSharing operator-(const ArithmeticSharing &rhs) const {
            return ArithmeticSharing(
                m_modulus,
                (m_share - rhs.m_share) % m_modulus,
                (m_mac - rhs.m_mac) % m_modulus
            );
        }

        ArithmeticSharing &operator-=(const ArithmeticSharing &rhs) {
            m_share = (m_share - rhs.m_share) % m_modulus;
            m_mac = (m_mac - rhs.m_mac) % m_modulus;
            return *this;
        }

        friend ArithmeticSharing operator+(int lhs, const ArithmeticSharing& rhs) {
            otpqc::math::Number<T> lhs_number{lhs};
            return ArithmeticSharing(
               rhs.m_modulus,
               (lhs_number + rhs.m_share) % rhs.m_modulus,
               (lhs_number + rhs.m_mac) % rhs.m_modulus
           );
        }
        

        friend ArithmeticSharing operator+(const ArithmeticSharing& lhs, int rhs) {
            return rhs + lhs;
        }

        // generic operator overloaded with Number<T> and ArithmeticSharinggtfhhgfth
        friend ArithmeticSharing operator+(const otpqc::math::Number<T>& lhs, const ArithmeticSharing& rhs) {
            return ArithmeticSharing(
                rhs.m_modulus,
                (lhs + rhs.m_share) % rhs.m_modulus,
                (lhs + rhs.m_mac) % rhs.m_modulus
            );
        }

        friend ArithmeticSharing operator+(const ArithmeticSharing& lhs, const otpqc::math::Number<T>& rhs) {
            return rhs + lhs;
        }

        ArithmeticSharing& operator+=(const otpqc::math::Number<T>& rhs) {
            m_share = (rhs + m_share) % m_modulus;
            m_mac = (rhs + m_mac) % m_modulus;
            return *this;
        }

        friend ArithmeticSharing operator-(const otpqc::math::Number<T>& lhs, const ArithmeticSharing& rhs) {
            return ArithmeticSharing(
                rhs.m_modulus,
                (lhs - rhs.m_share) % rhs.m_modulus,
                (lhs - rhs.m_mac) % rhs.m_modulus
            );
        }

        friend ArithmeticSharing operator-(const ArithmeticSharing& lhs, const otpqc::math::Number<T>& rhs) {
            return ArithmeticSharing(
                lhs.m_modulus,
                (lhs.m_share - rhs) % lhs.m_modulus,
                (lhs.m_mac - rhs) % lhs.m_modulus
            );
        }

        ArithmeticSharing& operator-=(const otpqc::math::Number<T>& rhs) {
            m_share = (m_share - rhs) % m_modulus;
            m_mac = (m_mac - rhs) % m_modulus;
            return *this;
        }

        friend ArithmeticSharing operator*(int lhs, const ArithmeticSharing& rhs) {
            otpqc::math::Number<T> lhs_number{lhs};
            return ArithmeticSharing(
               rhs.m_modulus,
               (lhs_number * rhs.m_share) % rhs.m_modulus,
               (lhs_number * rhs.m_mac) % rhs.m_modulus
           );
        }

        friend ArithmeticSharing operator*(const ArithmeticSharing& lhs, int rhs) {
            return rhs * lhs;
        }

        friend ArithmeticSharing operator*(const otpqc::math::Number<T>& lhs, const ArithmeticSharing& rhs) {
            return ArithmeticSharing(
                rhs.m_modulus,
                (lhs * rhs.m_share) % rhs.m_modulus,
                (lhs * rhs.m_mac) % rhs.m_modulus
            );
        }

        friend ArithmeticSharing operator*(const ArithmeticSharing& lhs, const otpqc::math::Number<T>& rhs) {
            return rhs * lhs;
        }
        ArithmeticSharing& operator*=(const otpqc::math::Number<T>& rhs) {
            m_share = (m_share * rhs) % m_modulus;
            m_mac = (m_mac * rhs) % m_modulus;
            return *this;
        }



        template<otpqc::math::IntegralNumeric U>
        auto operator*(U number) const {
            using R = otpqc::math::PromotedCommon<T, U>;

            const R modulus = static_cast<R>(m_modulus.get_value());
            const R num = static_cast<R>(number);
            const R share = static_cast<R>(m_share.get_value());
            const R mac = static_cast<R>(m_mac.get_value());

            R new_share = (share * num) % modulus;
            R new_mac = (mac * num) % modulus;

            return ArithmeticSharing<R>(otpqc::math::Number{modulus}, otpqc::math::Number{new_share}, otpqc::math::Number{new_mac});
        }

        template<otpqc::math::IntegralNumeric U>
            requires (otpqc::math::GmpNumeric<T> || (!otpqc::math::GmpNumeric<T> && otpqc::math::PrimitiveIntegral<U>))
        auto &operator*=(U number) {
            using R = otpqc::math::PromotedCommon<T, U>;

            const R modulus = static_cast<R>(m_modulus.get_value());
            const R num = static_cast<R>(number);

            m_share = otpqc::math::Number{static_cast<T>((static_cast<R>(m_share.get_value()) * num) % modulus)};
            m_mac = otpqc::math::Number{static_cast<T>((static_cast<R>(m_mac.get_value()) * num) % modulus)};

            return *this;
        }

        template<otpqc::math::PrimitiveIntegral U>
        ArithmeticSharing operator<<(U shift) const {
            return ArithmeticSharing(
                m_modulus,
                (m_share << shift) % m_modulus,
                (m_mac << shift) % m_modulus
            );
        }

        static ArithmeticShareVec<T> generate_random_shares(const otpqc::math::Number<T> &modulus, const otpqc::math::Number<T> &number,
                                                            const otpqc::math::Number<T> &key, const int count) {
            ArithmeticShareVec<T> shares;
            shares.reserve(count);

            otpqc::math::Number<otpqc::math::PromotedNumeric<T> > total{};
            for (int i = 0; i < count - 1; ++i) {
                auto random_share = math::Number<T>::random(modulus);
                shares.emplace_back(modulus, random_share, random_share);
                total = (total + random_share) % modulus;
            }
            shares.emplace_back(modulus, static_cast<otpqc::math::Number<T>>((number - total) % modulus), static_cast<otpqc::math::Number<T>>(number * key -
                                                                                                          total) % modulus);
            return shares;

        }

        static std::vector<otpqc::math::Number<T>> generate_random_global_key_shares(const otpqc::math::Number<T> &modulus, const otpqc::math::Number<T> &key,
                                                                   const int count) {
            std::vector<otpqc::math::Number<T>> shares;
            shares.reserve(count);

            otpqc::math::Number<otpqc::math::PromotedNumeric<T> > total{};
            for (int i = 0; i < count - 1; ++i) {
                auto random_share = math::Number<T>::random(modulus);
                shares.emplace_back(random_share);
                total = (total + random_share) % modulus;
            }
            shares.emplace_back(static_cast<otpqc::math::Number<T>>((key - total) % modulus));
            return shares;
        }

    private:
        otpqc::math::Number<T> m_modulus{}; /* Modulus of the field we are distributing shares over */
        otpqc::math::Number<T> m_share{}; /* Share of the actual value */
        otpqc::math::Number<T> m_mac{}; /* Share of the SPDZ mac */
    };
}

#endif

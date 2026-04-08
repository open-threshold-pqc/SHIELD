#ifndef NUMBER_H
#define NUMBER_H

#include <string>
#include <gmpxx.h>
#include <random>

/**
 * \brief Contains mathematical utility classes and functions used in MPC.
 *
 * Includes core numeric types such as @c Number that will server for representing both primitive and GMP types
 */
namespace otpqc::math {
    template<typename T>
    concept CompilerPrimitiveIntegral = std::is_same_v<__int128, T> || std::is_same_v<unsigned __int128, T>;

    template<typename T>
    concept PrimitiveIntegral = std::is_integral_v<T> || CompilerPrimitiveIntegral<T>;

    template<typename T>
    concept PrimitiveFloatingPoint = std::is_floating_point_v<T>;

    template<typename T>
    concept PrimitiveNumeric = PrimitiveIntegral<T> || PrimitiveFloatingPoint<T>;

    template<typename T>
    concept GmpNumeric = std::is_same_v<T, mpz_class>;

    template<typename T>
    concept IntegralNumeric = PrimitiveIntegral<T> || GmpNumeric<T>;

    template<typename T>
    concept FloatingNumeric = PrimitiveFloatingPoint<T>;

    template<typename T>
    concept Numeric = IntegralNumeric<T> || FloatingNumeric<T>;


    /*
     * GMP <- (GMP, primitive integral types, strings)
     * Primitive numerical types <- Primitive numerical types
     */
    template<typename T, typename U>
    concept NumericAssignable = (GmpNumeric<T> && (IntegralNumeric<U> || std::is_convertible_v<U, std::string>)) ||
                                (PrimitiveNumeric<T> && PrimitiveNumeric<U>);

    /*
     * Type promotion trait will increase the bit length of the given type. This promotion is only for primitive
     * numerical types. Passing GMP type will return GMP as the promoted type.
     */
    template<Numeric T>
    struct PromotedNumericHelper {
        static constexpr bool is_signed = std::is_signed_v<T>;

        using promoted_integral = std::conditional_t<
            sizeof(T) <= 1, std::conditional_t<is_signed, int32_t, uint32_t>,
            std::conditional_t<
                sizeof(T) == 2, std::conditional_t<is_signed, int32_t, uint32_t>,
                std::conditional_t<
                    sizeof(T) == 4, std::conditional_t<is_signed, int64_t, uint64_t>,
                    std::conditional_t<
#ifdef __SIZEOF_INT128__
                        sizeof(T) == 8, std::conditional_t<is_signed, __int128_t, __uint128_t>,
#else
    false, void, // fallback if int128 not available
#endif
                        T> > > >;

        using promoted_floating = std::conditional_t<
            sizeof(T) <= 4, double,
            std::conditional_t<sizeof(T) == 8, long double, T> >;

        using type = std::conditional_t<PrimitiveIntegral<T>, promoted_integral,
            std::conditional_t<PrimitiveFloatingPoint<T>, promoted_floating, T> >;
    };

    /* Alias for the type promotion trait */
    template<typename T>
    using PromotedNumeric = typename PromotedNumericHelper<T>::type;

    /*
     * Common promotion type trait which given two types, returns the promoted type that includes the both.
     * If any of two types is GMP, the common type is GMP. Otherwise, the we use std::common_type for primitve types.
     */
    template<Numeric T, Numeric U>
    struct PromotedCommonHelper {
        using type = std::conditional_t<
            (GmpNumeric<T> && IntegralNumeric<U>) || (IntegralNumeric<T> && GmpNumeric<U>),
            mpz_class,
            std::common_type_t<T, U>
        >;
    };

    /* Alias for the common type promotion trait */
    template<typename T, typename U>
    using PromotedCommon = typename PromotedCommonHelper<T, U>::type;

    /**
     * \class Number
     * \brief Number class for representing numerical values
     * \tparam T Numerical type to instantiate this class with
     *
     * Number class for representing numerical types including primitive (integral and floating) and GMP type.
     *
     * \subsection Usage
     * Number class can be used in different ways:
     * \include tutorial_math_number.cpp
     */
    template<Numeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class Number {
    public:
        Number() : m_value{T(0)} {
        }

        explicit Number(const T &v) : m_value{v} {
        }

        Number(const Number &other) = default;

        template<typename U>
            requires NumericAssignable<T, U>
        explicit Number(U v) {
            if constexpr (GmpNumeric<T>)
                m_value = mpz_class(v);
            else
                m_value = static_cast<T>(v);
        }

        explicit Number(const std::vector<uint8_t> &bytes) {
            if constexpr (PrimitiveNumeric<T>) {
                if (bytes.size() > sizeof(T))
                    throw std::invalid_argument("[Math::Number] Byte vector too large for primitive type");
                T result = 0;
                for (unsigned char byte: bytes)
                    result = (result << 8) | static_cast<T>(byte);

                m_value = result;
            } else if constexpr (GmpNumeric<T>)
                mpz_import(m_value.get_mpz_t(), bytes.size(), 1, 1, 1, 0, bytes.data());
        }

        const T &get_value() const { return m_value; }

        template<typename U>
        auto operator+(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) + static_cast<R>(rhs.get_value()));
        }

        template<typename U>
        auto operator-(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) - static_cast<R>(rhs.get_value()));
        }

        template<typename U>
        auto operator*(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) * static_cast<R>(rhs.get_value()));
        }

        template<typename U>
        auto operator/(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) / static_cast<R>(rhs.get_value()));
        }

        template<typename U>
        auto operator%(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            R rem = static_cast<R>(m_value) % static_cast<R>(rhs.get_value());
            if (rem < 0)
                rem += static_cast<R>(rhs.get_value());
            return Number<R>(rem);
        }

        template<typename U>
        auto operator^(const Number<U> &rhs) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) ^ static_cast<R>(rhs.get_value()));
        }

        template<typename U>
        Number &operator+=(const Number<U> &rhs) {
            using R = PromotedCommon<T, U>;
            R result = static_cast<R>(m_value) + static_cast<R>(rhs.get_value());
            m_value = static_cast<T>(result);
            return *this;
        }

        template<typename U>
        Number &operator-=(const Number<U> &rhs) {
            using R = PromotedCommon<T, U>;
            R result = static_cast<R>(m_value) - static_cast<R>(rhs.get_value());
            m_value = static_cast<T>(result);
            return *this;
        }

        template<typename U>
        Number &operator*=(const Number<U> &rhs) {
            using R = PromotedCommon<T, U>;
            R result = static_cast<R>(m_value) * static_cast<R>(rhs.get_value());
            m_value = static_cast<T>(result);
            return *this;
        }

        template<typename U>
        Number &operator/=(const Number<U> &rhs) {
            using R = PromotedCommon<T, U>;
            R result = static_cast<R>(m_value) / static_cast<R>(rhs.get_value());
            m_value = static_cast<T>(result);
            return *this;
        }

        template<typename U>
        Number &operator^=(const Number<U> &rhs) {
            using R = PromotedCommon<T, U>;
            R result = static_cast<R>(m_value) ^ static_cast<R>(rhs.get_value());
            m_value = static_cast<T>(result);
            return *this;
        }

        template<PrimitiveIntegral U>
        Number operator<<(U bits) const {
            return Number(m_value << bits);
        }

        template<Numeric U>
        auto operator*(U number) const {
            using R = PromotedCommon<T, U>;
            return Number<R>(static_cast<R>(m_value) * static_cast<R>(number));
        }

        // Unary minus operator
        Number operator-() const {
            return Number(-m_value);
        }

        bool operator==(const Number &rhs) const { return m_value == rhs.m_value; }

        template<Numeric U>
        bool operator==(const U &rhs) const { return m_value == rhs; }

        template<Numeric U>
        bool operator>(const U &rhs) const { return m_value > rhs; }

        bool operator>(const Number &rhs) const { return m_value > rhs.m_value; }

        template<Numeric U>
        bool operator>=(const U &rhs) const { return m_value >= rhs; }

        bool operator>=(const Number &rhs) const { return m_value >= rhs.m_value; }

        template<Numeric U>
        bool operator<(const U &rhs) const { return m_value < rhs; }

        bool operator<(const Number &rhs) const { return m_value < rhs.m_value; }

        template<Numeric U>
        bool operator<=(const U &rhs) const { return m_value <= rhs; }

        bool operator<=(const Number &rhs) const { return m_value <= rhs.m_value; }

        template<Numeric U>
        friend bool operator<(const U &lhs, const Number &rhs) { return lhs < rhs.m_value; }

        template<Numeric U>
        friend bool operator>(const U &lhs, const Number &rhs) { return lhs > rhs.m_value; }

        template<Numeric U>
        friend bool operator<=(const U &lhs, const Number &rhs) { return lhs <= rhs.m_value; }

        template<Numeric U>
        friend bool operator>=(const U &lhs, const Number &rhs) { return lhs >= rhs.m_value; }

        explicit operator T() const { return m_value; }

        Number &operator=(const Number &) = default;

        template<Numeric U>
            requires (!(GmpNumeric<T> && PrimitiveNumeric<U>))
        explicit operator Number<U>() const {
            return Number<U>(static_cast<U>(m_value));
        }

        friend std::ostream &operator<<(std::ostream &os, const Number &number) {
            return os << number.m_value;
        }

        [[nodiscard]] std::vector<uint8_t> linearized() const {
            std::vector<uint8_t> bytes;

            if constexpr (PrimitiveNumeric<T>) {
                constexpr size_t num_bytes = sizeof(T);
                bytes.resize(num_bytes);

                for (size_t i = 0; i < num_bytes; ++i)
                    bytes[i] = static_cast<uint8_t>(m_value >> (8 * (num_bytes - 1 - i)));
            } else if constexpr (GmpNumeric<T>) {
                size_t count = 0;
                void *data = mpz_export(nullptr, &count, 1, 1, 1, 0, m_value.get_mpz_t());
                if (!data)
                    throw std::runtime_error("[Math::Number::linearize] GMP export failed");

                bytes.assign(reinterpret_cast<uint8_t *>(data), reinterpret_cast<uint8_t *>(data) + count);
                free(data);
            }

            return bytes;
        }

        [[nodiscard]] std::vector<bool> bits_le() const {
            std::vector<bool> bits;

            if (m_value == 0) {
                bits.push_back(false);
                return bits;
            }

            if constexpr (PrimitiveNumeric<T>) {
                constexpr size_t max_bits = sizeof(T) * 8;
                const size_t highest_bit = max_bits - std::countl_zero(static_cast<std::make_unsigned_t<T>>(m_value));
                bits.reserve(highest_bit);

                for (size_t i = 0; i < highest_bit; ++i)
                    bits.push_back((m_value >> i) & 1);
            } else if constexpr (GmpNumeric<T>) {
                auto bit_length{mpz_sizeinbase(m_value.get_mpz_t(), 2)};
                bits.reserve(bit_length);
                for (size_t i = 0; i < bit_length; ++i)
                    bits.push_back(mpz_tstbit(m_value.get_mpz_t(), i));
            }
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_be() const {
            std::vector<bool> bits = bits_le();
            std::reverse(bits.begin(), bits.end());
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_le_se(const int bit_length) const {
            std::vector<bool> bits = bits_le();
            if (bit_length < static_cast<int>(bits.size()))
                throw std::invalid_argument(
                    "[Math::Number::bits_le_se] Bit length must be at least the size of the number");
            const bool signed_bit = m_value < 0 ? true : false;
            bits.resize(bit_length, signed_bit);
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_be_se(const int bit_length) const {
            std::vector<bool> bits = bits_le_se(bit_length);
            std::reverse(bits.begin(), bits.end());
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_le_ze(const int bit_length) const {
            std::vector<bool> bits = bits_le();
            if (bit_length < static_cast<int>(bits.size()))
                throw std::invalid_argument(
                    "[Math::Number::bits_le_ze] Bit length must be at least the size of the number");
            bits.resize(bit_length, false);
            return bits;
        }

        [[nodiscard]] std::vector<bool> bits_be_ze(const int bit_length) const {
            std::vector<bool> bits = bits_le_ze(bit_length);
            std::reverse(bits.begin(), bits.end());
            return bits;
        }

        /**
         * \brief This function returns a random number in range [0, q-1]
         * \param modulus The passed modulus q
         * \return Random number
         */
        static Number random(const Number &modulus)
            requires (IntegralNumeric<T>) {
            if (modulus.get_value() <= 0)
                throw std::invalid_argument("[Math::Number::random] Modulus for random generation must be positive.");

            T random_val{};
            if constexpr (GmpNumeric<T>) {
                gmp_randstate_t state;
                gmp_randinit_default(state);

                std::random_device rd;
                const unsigned long seed = rd();
                gmp_randseed_ui(state, seed);

                random_val = T();
                mpz_urandomm(random_val.get_mpz_t(), state, modulus.get_value().get_mpz_t());
                gmp_randclear(state);
            } else {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<T> distrib(static_cast<T>(0), modulus.get_value() - 1);
                random_val = distrib(gen);
            }
            return Number(random_val);
        }

        /**
         * \brief This function returns a random number of a specific bit length
         * \param bit_length Desired bit length
         * \return Random number
         */
        static Number random(const int bit_length)
            requires (IntegralNumeric<T>) {
            if (bit_length <= 0)
                throw std::invalid_argument("[Math::Number::Random] Bit length must be positive.");

            T random_val{};
            if constexpr (GmpNumeric<T>) {
                gmp_randstate_t state;
                gmp_randinit_default(state);

                std::random_device rd;
                gmp_randseed_ui(state, rd());

                mpz_class rand_mpz;
                mpz_urandomb(rand_mpz.get_mpz_t(), state, bit_length);

                random_val = rand_mpz;
                gmp_randclear(state);
            } else {
                constexpr int type_bits = sizeof(T) * 8;

                if (bit_length > type_bits)
                    throw std::invalid_argument("[Math::Number::random] Bit length exceeds size of primitive type.");

                using unsigned_type = std::make_unsigned_t<T>;

                std::random_device rd;
                std::mt19937_64 gen(rd());

                unsigned_type mask = (bit_length == type_bits)
                                         ? ~unsigned_type(0)
                                         : (unsigned_type(1) << bit_length) - 1;

                std::uniform_int_distribution<unsigned_type> distrib(0, mask);
                random_val = static_cast<T>(distrib(gen));
            }

            return Number(random_val);
        }
    private:
        T m_value;
    };
}

#endif

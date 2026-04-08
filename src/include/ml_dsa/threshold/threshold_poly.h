#ifndef THRESHOLD_POLY_H
#define THRESHOLD_POLY_H

#include <variant>
#include <vector>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/threshold_symmetric_shake.h"
#include "ml_dsa/threshold/threshold_decompose.h"
#include "ml_dsa/threshold/threshold_sample_in_ball.h"
#include "ml_dsa/threshold/threshold_check_norm.h"
#include "ml_dsa/threshold/threshold_make_hint.h"

#include "mpc/protocols/share_conversion/share_conversion.h"
#include "mpc/protocols/share_multiplication/share_multiplication.h"
#include "mpc/sharing/arithmetic_sharing.h"
#include "mpc/sharing/boolean_sharing.h"
#include "math/number.h"
#include "party.h"

/**
 * \brief Name space that contains threshold functionalities of Dilithium Polynomial operations
 */
namespace otpqc::threshold_signatures::dilithium::poly {
    /* Coefficient share can be an MPC arithmetic share, which we call it mpc share */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolyCoeffMPCArithShare = otpqc::mpc::sharing::ArithmeticSharing<T>;

    /* Coefficient share can be an MPC Boolean share */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolyCoeffMPCBoolShare = otpqc::mpc::sharing::BooleanSharing<T>;

    /* Coefficient share can be a constant value, which we call it a constant share */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolyCoeffConstantShare = otpqc::math::Number<T>;

    /* Coefficient share can either be constant or MPC share */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using CoefficientShare = std::variant<PolyCoeffConstantShare<T>, PolyCoeffMPCArithShare<T>, PolyCoeffMPCBoolShare<
        T> >;

    /* Polynomial share is a vector of coefficient shares */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolynomialShare = std::vector<CoefficientShare<T> >;

    /* Type two represent a vector of polynomial shares */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolynomialVectorShare = std::vector<PolynomialShare<T> >;

    /* Type two represent a 2D matrix of polynomial shares */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    using PolynomialMatrixShare = std::vector<PolynomialVectorShare<T> >;

    // Operator overloads for CoefficientShare<T>
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    CoefficientShare<T> operator+(const CoefficientShare<T> &lhs_coeff, const CoefficientShare<T> &rhs_coeff) {
        return std::visit([](const auto &a, const auto &b) -> CoefficientShare<T> {
            using DecayedA = std::decay_t<decltype(a)>;
            using DecayedB = std::decay_t<decltype(b)>;

            if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                          std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                return a + b; // qst::math::Number<T> + qst::math::Number<T>
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffMPCArithShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffMPCArithShare<T> >) {
                return a + b; // ArithmeticSharing<T> + ArithmeticSharing<T>
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffMPCArithShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> + PolyCoeffConstantShare<T> is defined and returns PolyCoeffMPCShare<T>
                return b + a;
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffMPCArithShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> + PolyCoeffConstantShare<T> is defined and returns PolyCoeffMPCShare<T>
                return a + b;
            } else {
                throw std::logic_error("Unsupported type combination for CoefficientShare addition");
            }
        }, lhs_coeff, rhs_coeff);
    }

    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    CoefficientShare<T> operator*(const CoefficientShare<T> &lhs_coeff, const CoefficientShare<T> &rhs_coeff) {
        return std::visit([](const auto &a, const auto &b) -> CoefficientShare<T> {
            using DecayedA = std::decay_t<decltype(a)>;
            using DecayedB = std::decay_t<decltype(b)>;

            if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                          std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                return a * b; // qst::math::Number<T> * qst::math::Number<T>
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffMPCArithShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> * PolyCoeffConstantShare<T> (public multiplication) is defined
                return b * a;
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffMPCArithShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> * PolyCoeffConstantShare<T> (public multiplication) is defined
                return a * b;
            } else {
                throw std::logic_error("Unsupported type combination for CoefficientShare multiplication");
            }
        }, lhs_coeff, rhs_coeff);
    }

    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    CoefficientShare<T> operator-(const CoefficientShare<T> &lhs_coeff, const CoefficientShare<T> &rhs_coeff) {
        return std::visit([](const auto &a, const auto &b) -> CoefficientShare<T> {
            using DecayedA = std::decay_t<decltype(a)>;
            using DecayedB = std::decay_t<decltype(b)>;

            if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                          std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                return (a - b) % otpqc::math::Number<T>{DILITHIUM_Q}; // qst::math::Number<T> - qst::math::Number<T>
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffMPCArithShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffMPCArithShare<T> >) {
                return a - b; // ArithmeticSharing<T> - ArithmeticSharing<T>
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffConstantShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffMPCArithShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> - PolyCoeffConstantShare<T> is defined
                return a - b;
            } else if constexpr (std::is_same_v<DecayedA, PolyCoeffMPCArithShare<T> > &&
                                 std::is_same_v<DecayedB, PolyCoeffConstantShare<T> >) {
                // Assumes PolyCoeffMPCShare<T> - PolyCoeffConstantShare<T> is defined
                return a - b;
            } else {
                throw std::logic_error("Unsupported type combination for CoefficientShare subtraction");
            }
        }, lhs_coeff, rhs_coeff);
    }

    // Operator overloads for PolynomialShare<T>
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> operator+(const PolynomialShare<T> &lhs, const PolynomialShare<T> &rhs) {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument(
                "PolynomialShare sizes must match for addition. LHS size: " + std::to_string(lhs.size()) +
                ", RHS size: " + std::to_string(rhs.size()));
        }
        PolynomialShare<T> result;
        result.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i) {
            result.push_back(lhs[i] + rhs[i]); // Uses CoefficientShare operator+
        }
        return result;
    }

    // Operator overload for PolynomialShare<T> subtraction
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> operator-(const PolynomialShare<T> &lhs, const PolynomialShare<T> &rhs) {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument(
                "PolynomialShare sizes must match for subtraction. LHS size: " + std::to_string(lhs.size()) +
                ", RHS size: " + std::to_string(rhs.size()));
        }
        PolynomialShare<T> result;
        result.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i) {
            result.push_back(lhs[i] - rhs[i]); // Uses CoefficientShare operator-
        }
        return result;
    }

    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> operator*(const PolynomialShare<T> &lhs, const PolynomialShare<T> &rhs) {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument(
                "PolynomialShare sizes must match for element-wise multiplication. LHS size: " +
                std::to_string(lhs.size()) + ", RHS size: " + std::to_string(rhs.size()));
        }
        PolynomialShare<T> result;
        result.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i)
            result.push_back(lhs[i] * rhs[i]);
        return result;
    }

    /**
     * \brief Adds two PolynomialVectorShare objects element-wise
     * \param lhs Left-hand side PolynomialVectorShare
     * \param rhs Right-hand side PolynomialVectorShare
     * \return A new PolynomialVectorShare containing the element-wise sum of lhs and rhs
     * \throws std::invalid_argument if lhs and rhs sizes do not match
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialVectorShare<T> operator+(const PolynomialVectorShare<T> &lhs, const PolynomialVectorShare<T> &rhs) {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument(
                "PolynomialVectorShare sizes must match for addition. LHS size: " + std::to_string(lhs.size()) +
                ", RHS size: " + std::to_string(rhs.size()));
        }
        PolynomialVectorShare<T> result;
        result.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i) {
            result.emplace_back(lhs[i] + rhs[i]); // Uses PolynomialShare operator+
        }
        return result;
    }

    /**
     * \brief Substraction of two PolynomialVectorShare objects element-wise
     * \param lhs Left-hand side PolynomialVectorShare
     * \param rhs Right-hand side PolynomialVectorShare
     * \return A new PolynomialVectorShare containing the element-wise difference of lhs and rhs
     * \throws std::invalid_argument if lhs and rhs sizes do not match
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialVectorShare<T> operator-(const PolynomialVectorShare<T> &lhs, const PolynomialVectorShare<T> &rhs) {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument(
                "PolynomialVectorShare sizes must match for subtraction. LHS size: " + std::to_string(lhs.size()) +
                ", RHS size: " + std::to_string(rhs.size()));
        }
        PolynomialVectorShare<T> result;
        result.reserve(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i) {
            result.emplace_back(lhs[i] - rhs[i]); // Uses PolynomialShare operator-
        }
        return result;
    }

    /**
     * \brief Given the input buffer containing random bytes, it extracts bits as the polynomial coefficients
     * \param buffer Input buffer to random bytes (boolean shared)
     * \return Array of DILITHIUM_N uint32_t-type coefficients
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::array<uint32_t, DILITHIUM_N> threshold_polyz_unpack(const uint8_t *buffer) {
        /* Array of polynomial coefficients. Note that values are boolean shared but stored as uint32_t. */
        std::array<uint32_t, DILITHIUM_N> coeffs{};

#if DILITHIUM_GAMMA1 == (1 << 17)
        for (int i = 0; i < DILITHIUM_N / 4; ++i) {
            coeffs[4 * i + 0] = buffer[9 * i + 0];
            coeffs[4 * i + 0] |= (uint32_t) buffer[9 * i + 1] << 8;
            coeffs[4 * i + 0] |= (uint32_t) buffer[9 * i + 2] << 16;
            coeffs[4 * i + 0] &= 0x3FFFF;

            coeffs[4 * i + 1] = buffer[9 * i + 2] >> 2;
            coeffs[4 * i + 1] |= (uint32_t) buffer[9 * i + 3] << 6;
            coeffs[4 * i + 1] |= (uint32_t) buffer[9 * i + 4] << 14;
            coeffs[4 * i + 1] &= 0x3FFFF;

            coeffs[4 * i + 2] = buffer[9 * i + 4] >> 4;
            coeffs[4 * i + 2] |= (uint32_t) buffer[9 * i + 5] << 4;
            coeffs[4 * i + 2] |= (uint32_t) buffer[9 * i + 6] << 12;
            coeffs[4 * i + 2] &= 0x3FFFF;

            coeffs[4 * i + 3] = buffer[9 * i + 6] >> 6;
            coeffs[4 * i + 3] |= (uint32_t) buffer[9 * i + 7] << 2;
            coeffs[4 * i + 3] |= (uint32_t) buffer[9 * i + 8] << 10;
            coeffs[4 * i + 3] &= 0x3FFFF;
        }
#elif DILITHIUM_GAMMA1 == (1 << 19)
        for (int i = 0; i < DILITHIUM_N / 2; ++i) {
            coeffs[2 * i + 0] = buffer[5 * i + 0];
            coeffs[2 * i + 0] |= (uint32_t) buffer[5 * i + 1] << 8;
            coeffs[2 * i + 0] |= (uint32_t) buffer[5 * i + 2] << 16;
            coeffs[2 * i + 0] &= 0xFFFFF;

            coeffs[2 * i + 1] = buffer[5 * i + 2] >> 4;
            coeffs[2 * i + 1] |= (uint32_t) buffer[5 * i + 3] << 4;
            coeffs[2 * i + 1] |= (uint32_t) buffer[5 * i + 4] << 12;
        }
#endif
        /* Subtracting Gamma1 from coefficients will not happen here. */
        return coeffs;
    }

#define POLY_UNIFORM_GAMMA1_NBLOCKS ((DILITHIUM_POLYZ_PACKEDBYTES + STREAM256_BLOCKBYTES - 1)/STREAM256_BLOCKBYTES)
    /**
     * \brief Given the boolean shared input seed and nonce, it generates a polynomial with random polynomials
     * \param party MPC party running this function
     * \param seed Input seed passed in boolean shared format
     * \param nonce Nonce to randomize the seed given in boolean shared format
     * \return Polynomial with random coefficients (coefficients are in Arithmetic share format)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> threshold_poly_uniform_gamma1(MPCParty<T> &party, const uint8_t seed[DILITHIUM_CRHBYTES],
                                                     const uint16_t nonce) {
        /* Polynomial with DILITHIUM_N coefficients */
        PolynomialShare<T> poly_share;
        poly_share.reserve(DILITHIUM_N);

        /* Buffer to store random bytes extract from calling the shake_stram XOF */
        uint8_t buf[POLY_UNIFORM_GAMMA1_NBLOCKS * STREAM256_BLOCKBYTES];

        stream256_state state;
        threshold_primitives::shake::threshold_shake256_stream_init(party, state, seed, nonce);
        threshold_primitives::shake::threshold_shake256_squeezeblocks(party, buf, POLY_UNIFORM_GAMMA1_NBLOCKS, state);

        /* Unpacking coefficients from the randomly generated bytes */
        for (const auto coeffs = threshold_polyz_unpack(buf); const unsigned int coeff: coeffs) {
            /* Creating a boolean share from each extracted coefficient for Y2A conversion */
            auto coeff_boolean_share = otpqc::mpc::sharing::BooleanSharing<T>{otpqc::math::Number<T>{coeff}, DILITHIUM_Q_BITLEN};

            /* To have (Gamma1 - coeff) shares:
             * Party 1 computes (Gamma1 - [coeff1]) while others compute 0-[coeffi] as their shares.
             */
            const auto &mac_key_share = party.get_mpc_context().get_global_mac_key_share();
            if (party.get_id() == 1) {
                auto coeff_arithmetic_share =
                        otpqc::mpc::sharing::ArithmeticSharing<T>{
                            otpqc::math::Number<T>{DILITHIUM_Q}, otpqc::math::Number<T>{DILITHIUM_GAMMA1},
                            otpqc::math::Number<T>{DILITHIUM_GAMMA1} * mac_key_share % otpqc::math::Number<T>{DILITHIUM_Q}
                        }
                        - mpc::protocols::ShareConversion<T>::y2a(party, coeff_boolean_share);
                poly_share.emplace_back(coeff_arithmetic_share);
            } else {
                auto coeff_arithmetic_share =
                        otpqc::mpc::sharing::ArithmeticSharing<T>{
                            otpqc::math::Number<T>{DILITHIUM_Q}, otpqc::math::Number<T>{0},
                            otpqc::math::Number<T>{DILITHIUM_GAMMA1} * mac_key_share % otpqc::math::Number<T>{DILITHIUM_Q}
                        }
                        - mpc::protocols::ShareConversion<T>::y2a(party, coeff_boolean_share);
                poly_share.emplace_back(coeff_arithmetic_share);
            }
        }
        return poly_share;
    }

    /**
     * \brief Threshold implementation of Polynomial decompose function
     * \param party MPC party running this function
     * \param p Reference to input polynomial
     * \return Tuple of polynomials where the first element is HighBits(p), and the second is LowBits(p)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<PolynomialShare<T>, PolynomialShare<T> >
    threshold_poly_decompose(MPCParty<T> &party, const PolynomialShare<T> &p) {
        PolynomialShare<T> high;
        high.reserve(DILITHIUM_N);

        PolynomialShare<T> low;
        low.reserve(DILITHIUM_N);

        for (int i = 0; i < DILITHIUM_N; ++i) {
            /* Check if coefficient is ArithmeticSharing */
            if (const auto *coeff = std::get_if<PolyCoeffMPCArithShare<T> >(&p[i])) {
                auto coeff_bool_share = mpc::protocols::ShareConversion<T>::a2y(party, *coeff);
                auto [hi_bool, lo_bool] = rounding::threshold_decompose(party, coeff_bool_share);

                high.emplace_back(CoefficientShare<T>{std::move(hi_bool)});
                auto lo_arith = mpc::protocols::ShareConversion<T>::y2a(party, lo_bool);
                low.emplace_back(CoefficientShare<T>{std::move(lo_arith)});
            } else {
                throw std::runtime_error(
                    "[threshold_poly_decompose] Coefficients should be in Arithmetic share format.");
            }
        }

        return std::make_tuple(std::move(high), std::move(low));
    }

    /**
     * \brief Threshold implementation of polynomial bit-packing
     * \param r Pointer to output byte array with at least POLYW1_PACKEDBYTES bytes
     * \param p Reference to input polynomial
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_polyw1_pack(uint8_t *r, const PolynomialShare<T> &p) {
        /* Extract the boolean shared coefficients of polynomial share p as an array */
        std::array<int32_t, DILITHIUM_N> coeffs{};

        for (int i = 0; i < DILITHIUM_N; ++i) {
            if (const auto *coeff = std::get_if<PolyCoeffMPCBoolShare<T> >(&p[i])) {
                coeffs[i] = coeff->get_share().get_value();
            } else
                throw std::runtime_error("Poly pack share not bool\n");
        }
#if DILITHIUM_GAMMA2 == (DILITHIUM_Q-1)/88
        for (int i = 0; i < DILITHIUM_N / 4; ++i) {
            r[3 * i + 0] = coeffs[4 * i + 0];
            r[3 * i + 0] |= coeffs[4 * i + 1] << 6;
            r[3 * i + 1] = coeffs[4 * i + 1] >> 2;
            r[3 * i + 1] |= coeffs[4 * i + 2] << 4;
            r[3 * i + 2] = coeffs[4 * i + 2] >> 4;
            r[3 * i + 2] |= coeffs[4 * i + 3] << 2;
        }
#elif DILITHIUM_GAMMA2 == (DILITHIUM_Q-1)/32
        for (int i = 0; i < DILITHIUM_N / 2; ++i)
            r[i] = coeffs[2 * i + 0] | (coeffs[2 * i + 1] << 4);
#endif
    }

    /**
    * \brief Threshold implementation of H. Samples polynomial (challenge) with TAU nonzero
    *  coefficients using the output stream of SHAKE256(seed)
    * \param party MPC party running this function
    * \param seed Input seed passed in boolean shared format
    * \return Share of the challenge polynomial
    */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> threshold_poly_challenge(MPCParty<T> &party, const uint8_t seed[DILITHIUM_CTILDEBYTES]) {
        PolynomialShare<T> challenge;
        challenge.reserve(DILITHIUM_N);

        uint8_t buf[SHAKE256_RATE];
        /* Calling Shake to generate random indices for Fisher-Yates shuffling */
        keccak_state state;
        threshold_primitives::shake::threshold_shake256_init(state);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, seed, DILITHIUM_CTILDEBYTES);
        threshold_primitives::shake::threshold_shake256_finalize(party, state);
        threshold_primitives::shake::threshold_shake256_squeezeblocks(party, buf, 1, state);

        /* Call the SampleInBall garbled circuit */
        auto coeffs = poly::threshold_sample_in_ball(party, buf);

        /* Converting coeffs to their corresponding arithmetic shares */
        for (int i = 0; i < DILITHIUM_N; ++i) {
            auto coeff_boolean_share = mpc::sharing::BooleanSharing<T>{otpqc::math::Number<T>{coeffs[i]}, DILITHIUM_Q_BITLEN};
            auto coeff_arith_share = mpc::protocols::ShareConversion<T>::y2a(party, coeff_boolean_share);
            challenge.emplace_back(coeff_arith_share);
        }
        return challenge;
    }


    /**
     * \brief Threshold implementation of poly checknorm that checks infinity norm of polynomial against a bound
     * \param party MPC party running this function
     * \param p Reference to input polynomial
     * \param bound Norm bound
     * \return Returns 0 if norm is strictly smaller than bound and 1 otherwise
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    int threshold_poly_chknorm(MPCParty<T> &party, const PolynomialShare<T> &p, int32_t bound) {
        /* Convert coefficients to Boolean share */
        std::vector<bool> packed_coefficients;
        packed_coefficients.reserve(DILITHIUM_N);

        for (int i = 0; i < DILITHIUM_N; ++i) {
            if (const auto *coeff = std::get_if<PolyCoeffMPCArithShare<T> >(&p[i])) {
                auto coeff_bool_share = mpc::protocols::ShareConversion<T>::a2y(party, *coeff);
                for (const auto &b: coeff_bool_share.get_share().bits_be_ze(DILITHIUM_Q_BITLEN))
                    packed_coefficients.emplace_back(b);
            }
        }
        return poly::threshold_chknorm(party, packed_coefficients, bound);
    }

    /**
     * \brief Threshold implementation of computing hint vector
     * \param party MPC party running this function
     * \param p1 Reference to input polynomial
     * \param p2 Reference to input polynomial
     * \return Returns shares of number of 1 bits and shares of hint vector
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<otpqc::mpc::sharing::ArithmeticSharing<T>, PolynomialShare<T> > threshold_poly_make_hint(
        MPCParty<T> &party, const PolynomialShare<T> &p1, const PolynomialShare<T> &p2) {
        /* Convert coefficients of input polynomials to Boolean shares */
        std::vector<bool> packed_coefficients1;
        packed_coefficients1.reserve(DILITHIUM_N);

        for (int i = 0; i < DILITHIUM_N; ++i) {
            if (const auto *coeff = std::get_if<PolyCoeffMPCArithShare<T> >(&p1[i])) {
                auto coeff_bool_share = mpc::protocols::ShareConversion<T>::a2y(party, *coeff);
                for (const auto &b: coeff_bool_share.get_share().bits_be_ze(DILITHIUM_Q_BITLEN))
                    packed_coefficients1.emplace_back(b);
            } else
                throw std::runtime_error("[threshold_poly_make_hint] pv1 vector should be in Arithmetic share");
        }

        /* For efficiency, we require the HighBits(w) to be in boolean shared. This is because the threshold
         * implementation of HighBits(.) returns w1 and w0 as boolean shares. This has the benefit that w1 can later
         * be used in boolean shared input to the hash function to compute the commitment (c_tilda)
         */
        std::vector<bool> packed_coefficients2;
        packed_coefficients2.reserve(DILITHIUM_N);
        for (int i = 0; i < DILITHIUM_N; ++i) {
            if (const auto *coeff = std::get_if<PolyCoeffMPCBoolShare<T> >(&p2[i])) {
                for (const auto &b: coeff->get_share().bits_be_ze(DILITHIUM_Q_BITLEN))
                    packed_coefficients2.emplace_back(b);
            } else
                throw std::runtime_error("[threshold_poly_make_hint] HighBits(w) vector should be in Boolean share");
        }

        auto [number_of_ones, poly_hint] = poly::threshold_make_hint(party, packed_coefficients1, packed_coefficients2);
        /* Convert number_of_ones to arithmetic_share */
        otpqc::mpc::sharing::BooleanSharing<T> number_of_ones_boolean{otpqc::math::Number<T>{number_of_ones}, DILITHIUM_Q_BITLEN};
        auto number_of_ones_arithmetic = mpc::protocols::ShareConversion<T>::y2a(party, number_of_ones_boolean);

        /* Creat Boolean-shared Polynomial for the hint vector */
        PolynomialShare<T> hint;
        hint.reserve(DILITHIUM_N);

        for (const auto &coeff: poly_hint)
            hint.emplace_back(mpc::sharing::BooleanSharing<T>{otpqc::math::Number<T>{coeff}, DILITHIUM_Q_BITLEN});

        return std::make_tuple(number_of_ones_arithmetic, hint);
    }

    /**
    * \brief Computes the product of point-wise polynomials with mpc arithmetic shares as their coefficients
    * \param party MPC party running this function
    * \param p1 First polynomial share
    * \param p2 Second polynomial share
    * \return Polynomial share of p1 * p2
    */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialShare<T> threshold_poly_pointwise_product(MPCParty<T> &party, const PolynomialShare<T> &p1,
                                                        const PolynomialShare<T> &p2) {
        if (!std::holds_alternative<PolyCoeffMPCArithShare<T> >(p1[0]) ||
            !std::holds_alternative<PolyCoeffMPCArithShare<T> >(p2[0]))
            throw std::runtime_error(
                "[threshold_poly_pointwise_product] Coefficients should be in Arithmetic share format.");

        if (p1.size() != p2.size())
            throw std::runtime_error("[threshold_poly_pointwise_product] Polynomial shares should have the same size.");

        const auto poly_size = p1.size();
        PolynomialShare<T> result;
        result.reserve(poly_size);

        for (int i = 0; i < poly_size; ++i) {
            const auto &coeff1 = std::get<PolyCoeffMPCArithShare<T> >(p1[i]);
            const auto &coeff2 = std::get<PolyCoeffMPCArithShare<T> >(p2[i]);
            auto multiplied_coeff = mpc::protocols::ShareMultiplication<T>::multiply_arithmetic_shares(
                party, coeff1, coeff2);
            result.emplace_back(std::move(multiplied_coeff));
        }
        return result;
    }
}


#endif

#ifndef THRESHOLD_POLYVEC_H
#define THRESHOLD_POLYVEC_H

#include "ml_dsa/threshold/threshold_poly.h"
#include "party.h"

/**
 * \brief Name space that contains threshold functionalities of Dilithium Vector of Polynomial operations
 */
namespace otpqc::threshold_signatures::dilithium::poly {
    /**
     * \brief Samples a vector (L) of polynomials.
     * \param party MPC party running this function
     * \param seed  Seed to generate random values used for extracting coefficients
     * \param nonce Nonce value to be appended to seed to randomize the input of random number generation
     * \return Vector of polynomials where their coefficients are arithmetic shares
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialVectorShare<T> threshold_polyvecl_uniform_gamma1(MPCParty<T> &party,
                                                               const uint8_t seed[DILITHIUM_CRHBYTES],
                                                               const uint16_t nonce) {
        /* Vector of polynomials (L polynomials) */
        PolynomialVectorShare<T> vec;
        vec.reserve(DILITHIUM_L);

        for (int i = 0; i < DILITHIUM_L; ++i) {
            /* Nonce is boolean shared as: Party 1 appends the nonce and others append 0 */
            const uint16_t actual_nonce = party.get_id() == 1 ? (DILITHIUM_L * nonce + i) : 0;
            auto poly_share = poly::threshold_poly_uniform_gamma1(party, seed, actual_nonce);
            vec.emplace_back(poly_share);
        }
        return vec;
    }

    /**
     * \brief Threshold implementation of Polynomial Vector decompose function
     * \param party MPC party running this function
     * \param pv Polynomial vector as input
     * \return Tuple of polynomial vectors where the first element is HighBits(pv), and the second is LowBits(pv)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<PolynomialVectorShare<T>, PolynomialVectorShare<T> > threshold_polyveck_decompose(MPCParty<T> &party,
        const PolynomialVectorShare<T> &pv) {
        PolynomialVectorShare<T> high_vec;
        high_vec.reserve(pv.size());

        PolynomialVectorShare<T> low_vec;
        low_vec.reserve(pv.size());

        for (int i = 0; i < DILITHIUM_K; ++i) {
            auto [hi_poly, lo_poly] = threshold_poly_decompose(party, const_cast<PolynomialShare<T> &>(pv[i]));

            high_vec.emplace_back(std::move(hi_poly));
            low_vec.emplace_back(std::move(lo_poly));
        }

        return std::make_tuple(std::move(high_vec), std::move(low_vec));
    }

    /**
     * \brief Threshold implementation of polynomial vector bit-packing
     * \param r Pointer to output byte array with at least POLYW1_PACKEDBYTES bytes
     * \param pv Reference to polynomial vector
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_polyveck_pack_w1(uint8_t r[DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES],
                                    const PolynomialVectorShare<T> &pv) {
        for (int i = 0; i < DILITHIUM_K; ++i)
            threshold_polyw1_pack(&r[i * DILITHIUM_POLYW1_PACKEDBYTES], pv[i]);
    }

    /**
     * \brief Computes the matrix-vector product of a polynomial matrix and a polynomial vector.
     * \param mat Polynomial matrix (K x L)
     * \param pv Polynomial vector (L)
     * \return Resulting polynomial vector (K)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialVectorShare<T> threshold_matrix_product_vector(const PolynomialMatrixShare<T> &mat,
                                                             const PolynomialVectorShare<T> &pv) {
        PolynomialVectorShare<T> result;
        result.resize(DILITHIUM_K);

        for (int i = 0; i < DILITHIUM_K; ++i) {
            for (int j = 0; j < DILITHIUM_L; ++j) {
                if (j == 0)
                    result[i] = mat[i][j] * pv[j];
                else
                    result[i] = result[i] + (mat[i][j] * pv[j]);
                if (std::holds_alternative<PolyCoeffConstantShare<T> >(result[i][0])) {
                    for (int n = 0; n < DILITHIUM_N; ++n) {
                        auto &coeff = std::get<PolyCoeffConstantShare<T> >(result[i][n]);
                        coeff = coeff % otpqc::math::Number<T>{DILITHIUM_Q}; // Ensure coefficients are reduced modulo DILITHIUM_Q
                    }
                }
            }
        }
        return result;
    }

    /**
     * \brief Check the infinity norm of polynomials in vector of length n (K or L)
     * \param party MPC party running this function
     * \param pv Input polynomial vector
     * \param bound Norm bound
     * \return 0 if norm of all polynomials is strictly smaller than the bound and 1 otherwise
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    int threshold_polyvecn_checknorm(MPCParty<T> &party, const PolynomialVectorShare<T> &pv, const int32_t bound) {
        /* Determining n (L or K) = pv.size() */
        const int num_polynomials{static_cast<int>(pv.size())};
        for (int i = 0; i < num_polynomials; ++i) {
            auto poly_check_norm_bool_result = threshold_poly_chknorm(party, pv[i], bound);

            /* Send the norm checking result to Party 1 for aggregation */
            auto io = party.get_io();
            if (party.get_id() == 1) {
                int peer_check_result{};
                for (int p = 2; p <= QST_NUM_OF_MPC_PARTIES; p++) {
                    io->recv_data(p, &peer_check_result, sizeof(int));
                    /* Xor the response received from other parties */
                    poly_check_norm_bool_result ^= peer_check_result;
                }
            } else {
                io->send_data(1, &poly_check_norm_bool_result, sizeof(int));
                io->flush();
            }
            int aggregated_check_result{};

            /* Party 1 sends the aggregated result to other parties */
            if (party.get_id() == 1) {
                /* Sets its own result */
                aggregated_check_result = poly_check_norm_bool_result;
                for (int p = 2; p <= QST_NUM_OF_MPC_PARTIES; p++) {
                    io->send_data(p, &aggregated_check_result, sizeof(int));
                    io->flush();
                }
            } else
                io->recv_data(1, &aggregated_check_result, sizeof(int));

            /* If a polynomial fails in norm checking, we skip processing the other polynomials */
            if (aggregated_check_result == 1)
                return 1;
        }
        return 0;
    }

    /**
     * \brief Compute hint vector
     * \param party MPC party running this function
     * \param pv1 Pointer to low part of input vector
     * \param pv2 Pointer to high part of input vector
     * \return Tuple of the form (share of number of ones in hint vector, hint vector)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<otpqc::mpc::sharing::ArithmeticSharing<T>, PolynomialVectorShare<T> > threshold_polyveck_make_hint(
        MPCParty<T> &party, const PolynomialVectorShare<T> &pv1, const PolynomialVectorShare<T> &pv2) {
        PolynomialVectorShare<T> h;
        h.reserve(DILITHIUM_K);

        otpqc::mpc::sharing::ArithmeticSharing<T> ones_accumulator{otpqc::math::Number<T>{DILITHIUM_Q}, otpqc::math::Number<T>{}, otpqc::math::Number<T>{}};
        for (int i = 0; i < DILITHIUM_K; ++i) {
            auto [ones_share, poly_hint] = threshold_poly_make_hint(party, pv1[i], pv2[i]);
            h.emplace_back(poly_hint);
            ones_accumulator += ones_share;
        }
        return std::make_tuple(ones_accumulator, h);
    }

    /**
     * \brief Computes the product of polynomial share with a polynomial vector share
     * \param party MPC party running this function
     * \param p Polynomial share to be multiplied
     * \param pv Polynomial vector share to be multiplied
     * \return Polynomial share of p * pv
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    PolynomialVectorShare<T> threshold_poly_pointwise_product_vector(MPCParty<T> &party, const PolynomialShare<T> &p,
                                                                     const PolynomialVectorShare<T> &pv) {
        if (!std::holds_alternative<PolyCoeffMPCArithShare<T> >(p[0]))
            throw std::runtime_error(
                "[threshold_poly_pointwise_product_vector] Coefficients should be in Arithmetic share format.");

        if (pv.empty())
            throw std::runtime_error("[threshold_poly_pointwise_product_vector] Polynomial vector share is empty.");

        PolynomialVectorShare<T> result;
        result.reserve(pv.size());
        for (const auto &poly: pv)
            result.emplace_back(std::move(threshold_poly_pointwise_product(party, p, poly)));

        return result;
    }
}


#endif

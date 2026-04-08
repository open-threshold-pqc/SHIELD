#ifndef THRESHOLD_KEYGEN_H
#define THRESHOLD_KEYGEN_H

#include <tuple>

#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/sign.h"
#include "ml_dsa/threshold/threshold_poly.h"
#include "ml_dsa/threshold/utils/poly_utils.h"
#include "ml_dsa/threshold/utils/byte_utils.h"
#include "ml_dsa/threshold/packing.h"

namespace otpqc::threshold_signatures::dilithium {
    /**
     * \brief (Trusted Dealer) Generates Dilithium public and private keys and return distributed shares of the (sk)
     * \param dilithium_modulus Dilithium modulus in Number<>{} format
     * \param global_key_share Global key share in Number<>{} format to distribute the private key elements
     * \return A tuple <rho, [[key]], tr, [[s1]], [[s2]], [[t0]], pk>
     *
     * ** We note that in above, the first 6 elements are part of private key. tr can be computed using rho and t1 and
     * hence, it can be considered as a public constant value that is part of the private key (See FIPS 204).
     */
    inline auto keygen(const otpqc::math::Number<> &dilithium_modulus, const otpqc::math::Number<> &global_key_share) {
        /* Allocating space for private and public key elements */
        const auto sk = std::make_unique<uint8_t[]>(DILITHIUM_CRYPTO_SECRETKEYBYTES);
        auto pk = std::make_unique<uint8_t[]>(DILITHIUM_CRYPTO_PUBLICKEYBYTES);

        /* Generate public and private keys using the original implementation of ML-DSA */
        crypto_sign_keypair(pk.get(), sk.get());

        /* A buffer to unpack elements of sk generated using ML-DSA's original keygen() */
        uint8_t sk_unpack_buffer[2 * DILITHIUM_SEEDBYTES + DILITHIUM_TRBYTES + 2 * DILITHIUM_CRHBYTES];
        polyveck t0, s2;
        polyvecl s1;

        uint8_t *rho_buffer = sk_unpack_buffer;
        uint8_t *tr_buffer = rho_buffer + DILITHIUM_SEEDBYTES;
        uint8_t *key_buffer = tr_buffer + DILITHIUM_TRBYTES;

        unpack_sk(rho_buffer, tr_buffer, key_buffer, &t0, &s1, &s2, sk.get());

        /* Convert ML-DSA's primitive polyveck polynomials to our Constant Polynomial type */
        const auto t0_constant_poly = poly::utils::polyveck_to_polynomial_vector_share(&t0);
        const auto s1_constant_poly = poly::utils::polyvecl_to_polynomial_vector_share(&s1);
        const auto s2_constant_poly = poly::utils::polyveck_to_polynomial_vector_share(&s2);

        /* Create arithmetic polynomial shares of constant polynomials for QST_NUM_OF_MPC_PARTIES parties */
        auto t0_arith_poly_shares = poly::utils::polynomial_vector_to_arithmetic_shares(
            t0_constant_poly, dilithium_modulus, global_key_share, QST_NUM_OF_MPC_PARTIES);
        auto s1_arith_poly_shares = poly::utils::polynomial_vector_to_arithmetic_shares(
            s1_constant_poly, dilithium_modulus, global_key_share, QST_NUM_OF_MPC_PARTIES);
        auto s2_arith_poly_shares = poly::utils::polynomial_vector_to_arithmetic_shares(
            s2_constant_poly, dilithium_modulus, global_key_share, QST_NUM_OF_MPC_PARTIES);

        /* Generating boolean shares of the key K for QST_NUM_OF_MPC_PARTIES parties */
        auto key_shares{
            utils::boolean_distribute_byte_array(key_buffer, DILITHIUM_SEEDBYTES,
                                                 QST_NUM_OF_MPC_PARTIES)
        };

        /* Allocate and copy public value rho */
        auto rho = std::make_unique<uint8_t[]>(DILITHIUM_SEEDBYTES);
        std::memcpy(rho.get(), rho_buffer, DILITHIUM_SEEDBYTES);

        /* Allocate and copy public value tr */
        auto tr = std::make_unique<uint8_t[]>(DILITHIUM_TRBYTES);
        std::memcpy(tr.get(), tr_buffer, DILITHIUM_TRBYTES);

        return std::make_tuple(
            std::move(rho),
            std::move(key_shares),
            std::move(tr),
            std::move(t0_arith_poly_shares),
            std::move(s1_arith_poly_shares),
            std::move(s2_arith_poly_shares),
            std::move(pk)
        );
    }
}


#endif

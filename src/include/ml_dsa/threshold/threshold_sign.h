#ifndef THRESHOLD_SIGN_H
#define THRESHOLD_SIGN_H

#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/threshold_fips202.h"
#include "ml_dsa/threshold/threshold_polyvec.h"
#include "ml_dsa/threshold/threshold_poly.h"
#include "ml_dsa/threshold/threshold_ntt.h"
#include "ml_dsa/threshold/threshold_hint_ones_comparison.h"
#include "packing.h"
#include "sign.h"
#include "party.h"
#include "mpc/protocols/mac_check/batch_checking.h"


namespace otpqc::threshold_signatures::dilithium {
    /**
     * \brief Reconstruct the final signature
     * \param party MPC party running this function
     * \param commitment_share Boolean shares of the commitment
     * \param z_share Arithmetic shares of the z
     * \param h_share Arithmetic shares of the h
     * \return A tuple (<commitment, z, h>) of reconstructed signature
     */
    inline std::tuple<std::array<uint8_t, DILITHIUM_CTILDEBYTES>, polyvecl, polyveck>
    threshold_combine_and_pack_signature(
        otpqc::MPCParty<> &party,
        const std::array<uint8_t, DILITHIUM_CTILDEBYTES> &commitment_share,
        poly::PolynomialVectorShare<> &z_share,
        poly::PolynomialVectorShare<> &h_share
    ) {
        auto io = party.get_io();

        /* Combining the commitment */
        std::array<uint8_t, DILITHIUM_CTILDEBYTES> signature_commitment{};

        if (party.get_id() != 1) {
            io->send_data(1, &commitment_share, DILITHIUM_CTILDEBYTES * sizeof(uint8_t));
            io->flush();
        } else {
            std::array<uint8_t, DILITHIUM_CTILDEBYTES> received_share{};

            for (int pid = 2; pid <= QST_NUM_OF_MPC_PARTIES; ++pid) {
                io->recv_data(pid, received_share.data(), DILITHIUM_CTILDEBYTES * sizeof(uint8_t));
                for (size_t i = 0; i < DILITHIUM_CTILDEBYTES; ++i)
                    signature_commitment[i] ^= received_share[i];
            }

            /* Combine with own share */
            for (size_t i = 0; i < DILITHIUM_CTILDEBYTES; ++i)
                signature_commitment[i] ^= commitment_share[i];
        }

        /* Combining the z */
        std::array<int32_t, DILITHIUM_L * DILITHIUM_N> z_share_linearized{};
        for (std::size_t i = 0; i < DILITHIUM_L; ++i) {
            for (std::size_t j = 0; j < DILITHIUM_N; ++j)
                if (const auto *coeff = std::get_if<poly::PolyCoeffMPCArithShare<> >(&z_share[i][j]))
                    z_share_linearized[i * DILITHIUM_N + j] = static_cast<int32_t>(coeff->get_share().get_value());
        }
        polyvecl signature_z{};
        if (party.get_id() != 1) {
            io->send_data(1, z_share_linearized.data(), DILITHIUM_L * DILITHIUM_N * sizeof(int32_t));
            io->flush();
        } else {
            std::array<int32_t, DILITHIUM_L * DILITHIUM_N> received_share{};

            for (int pid = 2; pid <= QST_NUM_OF_MPC_PARTIES; ++pid) {
                io->recv_data(pid, received_share.data(), DILITHIUM_L * DILITHIUM_N * sizeof(int32_t));

                for (std::size_t i = 0; i < DILITHIUM_L; ++i) {
                    for (std::size_t j = 0; j < DILITHIUM_N; ++j) {
                        signature_z.vec[i].coeffs[j] =
                                (signature_z.vec[i].coeffs[j] + received_share[i * DILITHIUM_N + j]) % DILITHIUM_Q;
                    }
                }
            }

            /* Combine with own share */
            for (std::size_t i = 0; i < DILITHIUM_L; ++i) {
                for (std::size_t j = 0; j < DILITHIUM_N; ++j) {
                    if (const auto *coeff = std::get_if<poly::PolyCoeffMPCArithShare<> >(&z_share[i][j])) {
                        const auto z_coeff = static_cast<int32_t>(
                            (signature_z.vec[i].coeffs[j] + coeff->get_share().get_value()) % DILITHIUM_Q);
                        signature_z.vec[i].coeffs[j] = (z_coeff > (DILITHIUM_Q - 1) / 2)
                                                           ? z_coeff - DILITHIUM_Q
                                                           : z_coeff;
                    }
                }
            }
        }

        /* Combining the h */
        std::array<int32_t, DILITHIUM_K * DILITHIUM_N> h_share_linearized{};
        for (std::size_t i = 0; i < DILITHIUM_K; ++i) {
            for (std::size_t j = 0; j < DILITHIUM_N; ++j)
                if (const auto *coeff = std::get_if<poly::PolyCoeffMPCBoolShare<> >(&h_share[i][j]))
                    h_share_linearized[i * DILITHIUM_N + j] = static_cast<int32_t>(coeff->get_share().get_value());
        }
        polyveck signature_h{};
        if (party.get_id() != 1) {
            io->send_data(1, h_share_linearized.data(), DILITHIUM_K * DILITHIUM_N * sizeof(int32_t));
            io->flush();
        } else {
            std::array<int32_t, DILITHIUM_K * DILITHIUM_N> received_share{};

            for (int pid = 2; pid <= QST_NUM_OF_MPC_PARTIES; ++pid) {
                io->recv_data(pid, received_share.data(), DILITHIUM_K * DILITHIUM_N * sizeof(int32_t));

                for (std::size_t i = 0; i < DILITHIUM_K; ++i) {
                    for (std::size_t j = 0; j < DILITHIUM_N; ++j) {
                        signature_h.vec[i].coeffs[j] ^= received_share[i * DILITHIUM_N + j];
                    }
                }
            }

            /* Combine with own share */
            for (std::size_t i = 0; i < DILITHIUM_K; ++i) {
                for (std::size_t j = 0; j < DILITHIUM_N; ++j) {
                    if (const auto *coeff = std::get_if<poly::PolyCoeffMPCBoolShare<> >(&h_share[i][j])) {
                        const auto h_coeff = signature_h.vec[i].coeffs[j] ^= static_cast<int32_t>(coeff->get_share().
                                                 get_value());
                        signature_h.vec[i].coeffs[j] = (h_coeff > (DILITHIUM_Q - 1) / 2)
                                                           ? h_coeff - DILITHIUM_Q
                                                           : h_coeff;
                    }
                }
            }
        }

        return std::make_tuple(
            signature_commitment,
            signature_z,
            signature_h
        );
    }

    /**
     * \brief Threshold signing algorithm
     * \param party MPC party running this function
     * \param key Boolean shares of the key
     * \param rnd Boolean shares of the randomness
     * \param mu Boolean shares of mu
     * \param A Constant Matrix A
     * \param s1 Arithmetic shares of s1
     * \param s2 Arithmetic shares of s2
     * \param t0 Arithmetic shares of t0
     * \param temp_nonce Temporary nonce to use for one successful signing iteration
     * \param signature Output parameter to return the packed signature
     * \param res_batch_check Output parameter to indicate if the batch check was successful
     * \return 0 (success)
     */
    inline void threshold_sign_signature_internal(otpqc::MPCParty<> &party, const uint8_t *key, const uint8_t *rnd,
                                                  const uint8_t *mu, const poly::PolynomialMatrixShare<> &A,
                                                  poly::PolynomialVectorShare<> &s1,
                                                  poly::PolynomialVectorShare<> &s2,
                                                  poly::PolynomialVectorShare<> &t0, const int temp_nonce,
                                                  std::tuple<std::array<uint8_t, DILITHIUM_CTILDEBYTES>, polyvecl,
                                                      polyveck> &signature, bool &res_batch_check) {
        using namespace otpqc::threshold_signatures::dilithium::poly; //todo

        /* Computing rho'' = H(K || rnd || mu) */
        std::array<uint8_t, DILITHIUM_CRHBYTES> rhoprime{};
        keccak_state state;
        threshold_primitives::shake::threshold_shake256_init(state);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, key, DILITHIUM_SEEDBYTES);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, rnd, DILITHIUM_RNDBYTES);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, mu, DILITHIUM_CRHBYTES);
        threshold_primitives::shake::threshold_shake256_finalize(party, state);
        threshold_primitives::shake::threshold_shake256_squeeze(party, rhoprime.data(), DILITHIUM_CRHBYTES, state);

        /* Compute y = ExpandMask(rhoprime, nonce) */
        const auto y = poly::threshold_polyvecl_uniform_gamma1(party, rhoprime.data(), temp_nonce);

        /* Compute w = Ay */
        auto y_hat = y; // Make a copy of y for in place NTT. We need y further in z = y + c * s1
        threshold_NTT::threshold_ntt(y_hat);
        auto w = poly::threshold_matrix_product_vector(A, y_hat);
        threshold_NTT::threshold_invntt(w);

        /* Compute w1 = HighBits (w, 2Gamma2) */
        auto [w1, w0] = poly::threshold_polyveck_decompose(party, w);

        /* Compute w1Encode(w1) */
        std::array<uint8_t, DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES> packed_w1{};
        poly::threshold_polyveck_pack_w1(packed_w1.data(), w1);

        /* Compute commitment c~ = H(mu || w1Encode(w1), lambda/4) */
        std::array<uint8_t, DILITHIUM_CTILDEBYTES> c_tilda{};

        threshold_primitives::shake::threshold_shake256_init(state);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, mu, DILITHIUM_CRHBYTES);
        threshold_primitives::shake::threshold_shake256_absorb(party, state, packed_w1.data(),
                                                               DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES);
        threshold_primitives::shake::threshold_shake256_finalize(party, state);
        threshold_primitives::shake::threshold_shake256_squeeze(party, c_tilda.data(), DILITHIUM_CTILDEBYTES, state);

        /* Compute challenge c = SampleInBall(c~) */
        auto c = poly::threshold_poly_challenge(party, c_tilda.data());

        /* Prepare the ntt version (in place):
         * 1. c_hat = NTT(c)
         * 2. s1_hat = NTT(s1)
         * 3. s2_hat = NTT(s2) */
        threshold_NTT::threshold_ntt(c);
        threshold_NTT::threshold_ntt(s1);
        threshold_NTT::threshold_ntt(s2);

        /* Compute:
         * 1. c * s1 = c_hat * s1_hat
         * 2. z = y + c * s1 */
        auto c_s1 = poly::threshold_poly_pointwise_product_vector(party, c, s1);
        threshold_NTT::threshold_invntt(c_s1);
        auto z = y + c_s1;

        if (poly::threshold_polyvecn_checknorm(party, z, DILITHIUM_GAMMA1 - DILITHIUM_BETA) == 0) {
            /* Compute
             * 1. c_s2 = c_hat * s2_hat
             * 2. r0 = LowBits(w − c_s2)*/

            auto c_s2 = poly::threshold_poly_pointwise_product_vector(party, c, s2);
            threshold_NTT::threshold_invntt(c_s2);
            auto r0 = w0 - c_s2;

            if (poly::threshold_polyvecn_checknorm(party, r0, DILITHIUM_GAMMA2 - DILITHIUM_BETA) == 0) {
                /*Compute c_t0 = c_hat * t0_hat*/
                threshold_NTT::threshold_ntt(t0);
                auto c_t0 = poly::threshold_poly_pointwise_product_vector(party, c, t0);
                threshold_NTT::threshold_invntt(c_t0);

                if (poly::threshold_polyvecn_checknorm(party, c_t0, DILITHIUM_GAMMA2) == 0) {
                    /* Compute MakeHint input (w - c * s2 + c * t0) as: r0 + c_t0 */
                    r0 = r0 + c_t0;

                    /* Compute h = MakeHint(w - c * s2 + c * t0, HighBits(w))*/
                    auto [number_of_ones, h] = poly::threshold_polyveck_make_hint(party, r0, w1);

                    /* Compare the number of ones in h against OMEGA */
                    if (poly::threshold_hint_one_comparison(party, number_of_ones) == 0) {
                        /* Construct and output the signature (c_tilda, z, h).
                         * Note: For now, only Party 1 has the full reconstructed signature. Other parties still
                         * return shares of their signature. In the main function (Dealer) we only check the return
                         * value of the first party for signature verification.
                         */
                        signature = threshold_combine_and_pack_signature(party, c_tilda, z, h);
                    }
                }
            }
        }
        res_batch_check = otpqc::mpc::protocols::BatchChecking<>::perform_batch_checking(
            party, otpqc::math::Number<>(DILITHIUM_Q));
    }


    /**
     * \brief Compute Signature
     * \param party MPC party running this function
     * \param sig pointer to output signed message
     * \param siglen pointer to output length of signed message
     * \param m pointer to message to be signed
     * \param mlen length of message
     * \param ctx pointer to context string
     * \param ctxlen length of context string
     * \param tr Pointer to tr public value
     * \param key Boolean shares of the key
     * \param rnd Boolean shares of the randomness
     * \param A Constant Matrix A
     * \param s1 Arithmetic shares of s1
     * \param s2 Arithmetic shares of s2
     * \param t0 Arithmetic shares of t0
     * \param temp_nonce Temporary nonce to use for one successful signing iteration
     * \return 0 (success) or -1 (context string too long)
     */
    inline int sign_signature(otpqc::MPCParty<> &party, uint8_t *sig,
                              size_t *siglen,
                              const uint8_t *m,
                              size_t mlen,
                              const uint8_t *ctx,
                              size_t ctxlen, const uint8_t *tr,
                              const uint8_t *key, const uint8_t *rnd, const poly::PolynomialMatrixShare<> &A,
                              poly::PolynomialVectorShare<> &s1,
                              poly::PolynomialVectorShare<> &s2,
                              poly::PolynomialVectorShare<> &t0, const int temp_nonce, bool &res_batch_check) {
        uint8_t pre[257];

        if (ctxlen > 255)
            return -1;

        /* Prepare pre = (0, ctxlen, ctx) */
        pre[0] = 0;
        pre[1] = ctxlen;
        for (size_t i = 0; i < ctxlen; i++)
            pre[2 + i] = ctx[i];

        /* Compute mu = CRH(tr, pre, msg) */
        uint8_t mu_buffer[DILITHIUM_CRHBYTES] = {0};
        if (party.get_id() == 1) {
            keccak_state state;
            shake256_init(&state);
            shake256_absorb(&state, tr, DILITHIUM_TRBYTES);
            shake256_absorb(&state, pre, 2 + ctxlen);
            shake256_absorb(&state, m, mlen);
            shake256_finalize(&state);
            shake256_squeeze(mu_buffer, DILITHIUM_CRHBYTES, &state);
        }

        std::tuple<std::array<uint8_t, DILITHIUM_CTILDEBYTES>, polyvecl, polyveck> signature;
        threshold_sign_signature_internal(party, key, rnd, mu_buffer, A, s1, s2, t0, temp_nonce, signature,
                                          res_batch_check);
        pack_sig(sig, std::get<0>(signature).data(), &std::get<1>(signature), &std::get<2>(signature));
        *siglen = static_cast<size_t>(DILITHIUM_CRYPTO_BYTES);
        return 0;
    }

    /**
     * \brief Compute signed message
     * \param party MPC party running this function
     * \param sm pointer to output signed message
     * \param smlen pointer to output length of signed message
     * \param m pointer to message to be signed
     * \param mlen length of message
     * \param ctx pointer to context string
     * \param ctxlen length of context string
     * \param tr Pointer to tr public value
     * \param key Boolean shares of the key
     * \param rnd Boolean shares of the randomness
     * \param A Constant Matrix A
     * \param s1 Arithmetic shares of s1
     * \param s2 Arithmetic shares of s2
     * \param t0 Arithmetic shares of t0
     * \param temp_nonce Temporary nonce to use for one successful signing iteration
     * \return 0 (success) or -1 (context string too long)
     */
    inline int sign(otpqc::MPCParty<> &party, uint8_t *sm,
                    size_t *smlen,
                    const uint8_t *m,
                    size_t mlen,
                    const uint8_t *ctx,
                    size_t ctxlen, uint8_t *tr,
                    const uint8_t *key, const uint8_t *rnd, const poly::PolynomialMatrixShare<> &A,
                    poly::PolynomialVectorShare<> &s1,
                    poly::PolynomialVectorShare<> &s2,
                    poly::PolynomialVectorShare<> &t0, const int temp_nonce, bool &res_batch_check) {
        for (size_t i = 0; i < mlen; ++i)
            sm[DILITHIUM_CRYPTO_BYTES + mlen - 1 - i] = m[mlen - 1 - i];

        const int ret = sign_signature(party, sm, smlen, sm + DILITHIUM_CRYPTO_BYTES, mlen, ctx, ctxlen, tr,
                                       key, rnd, A, s1, s2, t0, temp_nonce, res_batch_check);

        *smlen += mlen;
        return ret;
    }
}


#endif

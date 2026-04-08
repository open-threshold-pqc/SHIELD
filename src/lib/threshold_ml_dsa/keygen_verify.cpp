#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/sign.h"
#include "ml_dsa/threshold/packing.h"
#include "ml_dsa/threshold/polyvec.h"
#include "ml_dsa/threshold/poly.h"
#include "ml_dsa/threshold/fips202.h"

/*************************************************
* Name:        crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
**************************************************/
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk) {
    uint8_t seedbuf[2 * DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES] = {};
    uint8_t tr[DILITHIUM_TRBYTES];
    const uint8_t *rho, *rhoprime, *key;
    polyvecl mat[DILITHIUM_K];
    polyvecl s1, s1hat;
    polyveck s2, t1, t0;

    /* Get randomness for rho, rhoprime and key */
    // randombytes(seedbuf, DILITHIUM_SEEDBYTES);
    seedbuf[DILITHIUM_SEEDBYTES + 0] = DILITHIUM_K;
    seedbuf[DILITHIUM_SEEDBYTES + 1] = DILITHIUM_L;
    shake256(seedbuf, 2 * DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES, seedbuf, DILITHIUM_SEEDBYTES + 2);
    rho = seedbuf;
    rhoprime = rho + DILITHIUM_SEEDBYTES;
    key = rhoprime + DILITHIUM_CRHBYTES;

    /* Expand matrix */
    polyvec_matrix_expand(mat, rho);

    /* Sample short vectors s1 and s2 */
    polyvecl_uniform_eta(&s1, rhoprime, 0);
    polyveck_uniform_eta(&s2, rhoprime, DILITHIUM_L);

    /* Matrix-vector multiplication */
    s1hat = s1;
    polyvecl_ntt(&s1hat);
    polyvec_matrix_pointwise_montgomery(&t1, mat, &s1hat);
    polyveck_reduce(&t1);
    polyveck_invntt_tomont(&t1);

    /* Add error vector s2 */
    polyveck_add(&t1, &t1, &s2);

    /* Extract t1 and write public key */
    polyveck_caddq(&t1);
    polyveck_power2round(&t1, &t0, &t1);
    pack_pk(pk, rho, &t1);

    /* Compute H(rho, t1) and write secret key */
    shake256(tr, DILITHIUM_TRBYTES, pk, DILITHIUM_CRYPTO_PUBLICKEYBYTES);
    pack_sk(sk, rho, tr, key, &t0, &s1, &s2);

    return 0;
}

/*************************************************
* Name:        crypto_sign_verify_internal
*
* Description: Verifies signature. Internal API.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *pre: pointer to prefix string
*              - size_t prelen: length of prefix string
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_verify_internal(const uint8_t *sig,
                                size_t siglen,
                                const uint8_t *m,
                                size_t mlen,
                                const uint8_t *pre,
                                size_t prelen,
                                const uint8_t *pk) {
    unsigned int i;
    uint8_t buf[DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES];
    uint8_t rho[DILITHIUM_SEEDBYTES];
    uint8_t mu[DILITHIUM_CRHBYTES];
    uint8_t c[DILITHIUM_CTILDEBYTES];
    uint8_t c2[DILITHIUM_CTILDEBYTES];
    poly cp;
    polyvecl mat[DILITHIUM_K], z;
    polyveck t1, w1, h;
    keccak_state state;

    if (siglen != DILITHIUM_CRYPTO_BYTES)
        return -1;

    unpack_pk(rho, &t1, pk);
    if (unpack_sig(c, &z, &h, sig))
        return -1;
    if (polyvecl_chknorm(&z, DILITHIUM_GAMMA1 - DILITHIUM_BETA))
        return -1;

    /* Compute CRH(H(rho, t1), pre, msg) */
    shake256(mu, DILITHIUM_TRBYTES, pk, DILITHIUM_CRYPTO_PUBLICKEYBYTES);
    shake256_init(&state);
    shake256_absorb(&state, mu, DILITHIUM_TRBYTES);
    shake256_absorb(&state, pre, prelen);
    shake256_absorb(&state, m, mlen);
    shake256_finalize(&state);
    shake256_squeeze(mu, DILITHIUM_CRHBYTES, &state);

    /* Matrix-vector multiplication; compute Az - c2^dt1 */
    poly_challenge(&cp, c);
    polyvec_matrix_expand(mat, rho);

    polyvecl_ntt(&z);
    polyvec_matrix_pointwise_montgomery(&w1, mat, &z);

    poly_ntt(&cp);
    polyveck_shiftl(&t1);
    polyveck_ntt(&t1);
    polyveck_pointwise_poly_montgomery(&t1, &cp, &t1);

    polyveck_sub(&w1, &w1, &t1);
    polyveck_reduce(&w1);
    polyveck_invntt_tomont(&w1);

    /* Reconstruct w1 */
    polyveck_caddq(&w1);
    polyveck_use_hint(&w1, &w1, &h);
    polyveck_pack_w1(buf, &w1);

    /* Call random oracle and verify challenge */
    shake256_init(&state);
    shake256_absorb(&state, mu, DILITHIUM_CRHBYTES);
    shake256_absorb(&state, buf, DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES);
    shake256_finalize(&state);
    shake256_squeeze(c2, DILITHIUM_CTILDEBYTES, &state);
    for (i = 0; i < DILITHIUM_CTILDEBYTES; ++i)
        if (c[i] != c2[i])
            return -1;

    return 0;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *ctx: pointer to context string
*              - size_t ctxlen: length of context string
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_verify(const uint8_t *sig,
                       size_t siglen,
                       const uint8_t *m,
                       size_t mlen,
                       const uint8_t *ctx,
                       size_t ctxlen,
                       const uint8_t *pk) {
    size_t i;
    uint8_t pre[257];

    if (ctxlen > 255)
        return -1;

    pre[0] = 0;
    pre[1] = ctxlen;
    for (i = 0; i < ctxlen; i++)
        pre[2 + i] = ctx[i];

    return crypto_sign_verify_internal(sig, siglen, m, mlen, pre, 2 + ctxlen, pk);
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m: pointer to output message (allocated
*                            array with smlen bytes), can be equal to sm
*              - size_t *mlen: pointer to output length of message
*              - const uint8_t *sm: pointer to signed message
*              - size_t smlen: length of signed message
*              - const uint8_t *ctx: pointer to context tring
*              - size_t ctxlen: length of context string
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signed message could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_open(uint8_t *m,
                     size_t *mlen,
                     const uint8_t *sm,
                     size_t smlen,
                     const uint8_t *ctx,
                     size_t ctxlen,
                     const uint8_t *pk) {
    size_t i;

    if (smlen < DILITHIUM_CRYPTO_BYTES)
        goto badsig;

    *mlen = smlen - DILITHIUM_CRYPTO_BYTES;
    if (crypto_sign_verify(sm, DILITHIUM_CRYPTO_BYTES, sm + DILITHIUM_CRYPTO_BYTES, *mlen, ctx, ctxlen, pk))
        goto badsig;
    else {
        /* All good, copy msg, return 0 */
        for (i = 0; i < *mlen; ++i)
            m[i] = sm[DILITHIUM_CRYPTO_BYTES + i];
        return 0;
    }

badsig:
    /* Signature verification failed */
    *mlen = 0;
    for (i = 0; i < smlen; ++i)
        m[i] = 0;

    return -1;
}

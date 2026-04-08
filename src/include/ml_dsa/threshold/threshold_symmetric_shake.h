#ifndef SYMMETRIC_SHAKE_H
#define SYMMETRIC_SHAKE_H

#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/threshold_fips202.h"
#include "party.h"


/**
 * \brief Name space that contains threshold functionalities of shake stream
 */
namespace otpqc::threshold_primitives::shake {
    /**
     * \brief Shake256 streams which generates a stream of random bytes
     * \param party MPC party running this function
     * \param state Keccak state
     * \param seed Input seed given in boolean shared format
     * \param nonce Nonce to randomize the input along with the seed
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_shake256_stream_init(MPCParty<T> &party, keccak_state &state,
                                        const uint8_t seed[DILITHIUM_CRHBYTES], const uint16_t nonce) {
        uint8_t t[2];
        t[0] = nonce;
        t[1] = nonce >> 8;
        threshold_shake256_init(state);
        shake::threshold_shake256_absorb(party, state, seed, DILITHIUM_CRHBYTES);
        shake::threshold_shake256_absorb(party, state, t, 2);
        shake::threshold_shake256_finalize(party, state);
    }
}


#endif

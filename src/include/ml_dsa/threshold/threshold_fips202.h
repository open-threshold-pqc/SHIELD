#ifndef THRESHOLD_FIPS202_H
#define THRESHOLD_FIPS202_H

#include "ml_dsa/threshold/fips202.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "mpc/mpc_context.h"
#include "party.h"


/**
 * \brief Name space containing helper functionalities regarding fips 202 cryptographic primitives
 */
namespace otpqc::primitives::keccak {
    /* These functionalities were defined mainly as static functions in their source files. Since they become
     * inaccessible, we moved their definition to our own header file for ease of use.
     */

    /**
     * \brief Store a 64-bit integer to array of 8 bytes in little-endian order
     * \param x Pointer to the output byte array (allocated)
     * \param u Input 64-bit unsigned integer
     */
    inline void store64(uint8_t x[8], const uint64_t u) {
        for (unsigned int i = 0; i < 8; i++)
            x[i] = u >> 8 * i;
    }

    /**
     * \brief Initializes the Keccak state
     * \param s Pointer to the Keccak state's byte array (1600 bits)
     */
    inline void keccak_init(uint64_t s[25]) {
        for (unsigned int i = 0; i < 25; i++)
            s[i] = 0;
    }


    /**
     * \brief Finalize the absorb step of the Keccak
     * \param s Pointer to the Keccak state's byte array (1600 bits)
     * \param pos Position in current block to be absorbed
     * \param r Keccak processing rate
     * \param p Domain separation byte
     */
    inline void keccak_finalize(uint64_t s[25], unsigned int pos, const unsigned int r, const uint8_t p) {
        s[pos / 8] ^= (uint64_t) p << 8 * (pos % 8);
        s[r / 8 - 1] ^= 1ULL << 63;
    }


    /* The following functionalities have been provided to manipulate the input to put it in correct order
     * for calling the corresponding GC function. Upon finding more efficient circuit, these will be removed. */

    /**
     * \brief Converts an array of uint64_t bytes to vector of booleans
     * \param s Array of uint64_t bytes
     * \return Vector of booleans showing the bits of the array in Little Endian format
     */
    static std::vector<bool> uint64_to_bool_vector_le(const uint64_t s[25]) {
        std::vector<bool> boolVec;
        boolVec.reserve(25 * 64);

        for (int i = 0; i < 25; ++i) {
            for (int j = 0; j < 64; ++j) {
                boolVec.push_back((s[i] >> (63 - j)) & 1);
            }
        }
        return boolVec;
    }

    /**
     * \brief Converts vector of boolean values to array of uint64_t bytes storing as Big Endian
     * \param vec Vector of bool values
     * \param s Array of uint64_t bytes
     * \return Vector of booleans showing the bits of the array in Little Endian format
     */
    static void bool_vector_to_uint64_be(const std::vector<bool> &vec, uint64_t s[25]) {
        std::memset(s, 0, 25 * sizeof(uint64_t));

        for (int i = 0; i < 25; ++i) {
            for (int j = 0; j < 64; ++j) {
                if (i * 64 + j < vec.size()) {
                    s[i] |= static_cast<uint64_t>(vec[i * 64 + j]) << (63 - j);
                }
            }
        }
    }
}

/**
 * \brief Name space containing threshold implementation of fips 202 Keccak functionalities
 */
namespace otpqc::threshold_primitives::keccak {
    /**
     * \brief Threshold implementation of Keccak Absorb function
     * \param party MPC party running this function
     * \param s Pointer to the Keccak state's byte array (1600 bits)
     * \param pos Position in current block to be absorbed
     * \param r Keccak processing rate
     * \param in Pointer to the input bytes we want to read from
     * \param inlen Size of the input bytes to read
     * \return New position in current block
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    static unsigned int threshold_keccak_absorb(MPCParty<T> &party, uint64_t s[25], unsigned int pos,
                                                const unsigned int r,
                                                const uint8_t *in,
                                                size_t inlen) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

        unsigned int i;

        while (pos + inlen >= r) {
            for (i = pos; i < r; i++)
                s[i / 8] ^= (uint64_t) *in++ << 8 * (i % 8);
            inlen -= r - pos;

            std::vector<bool> circuit_input = primitives::keccak::uint64_to_bool_vector_le(s);
            auto keccak_gc_function = party.get_mpc_context().get_registered_circuit(
                GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600);
            auto circuit_output = run(keccak_gc_function, &circuit_input);
            primitives::keccak::bool_vector_to_uint64_be(circuit_output, s);

            pos = 0;
        }

        for (i = pos; i < pos + inlen; i++)
            s[i / 8] ^= (uint64_t) *in++ << 8 * (i % 8);

        return i;
    }

    /**
     * \brief Threshold implementation of Keccak Squeeze function
     * \param party MPC party running this function
     * \param out Pointer to the output buffer to write squeezed byte into
     * \param outlen Number of bytes to extract and write into output
     * \param s Pointer to the Keccak state's byte array (1600 bits)
     * \param pos Position in current block to be absorbed
     * \param r Keccak processing rate
     * \return Returns new position in current block
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    unsigned int threshold_keccak_squeeze(MPCParty<T> &party, uint8_t *out,
                                          size_t outlen,
                                          uint64_t s[25],
                                          unsigned int pos,
                                          const unsigned int r) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

        unsigned int i;

        while (outlen) {
            if (pos == r) {
                std::vector<bool> circuit_input = primitives::keccak::uint64_to_bool_vector_le(s);
                auto keccak_gc_function = party.get_mpc_context().get_registered_circuit(
                    GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600);
                auto circuit_output = run(keccak_gc_function, &circuit_input);
                primitives::keccak::bool_vector_to_uint64_be(circuit_output, s);

                pos = 0;
            }
            for (i = pos; i < r && i < pos + outlen; i++)
                *out++ = s[i / 8] >> 8 * (i % 8);
            outlen -= i - pos;
            pos = i;
        }

        return pos;
    }

    /**
     * \brief Threshold implementation of Keccak Squeeze Blocks
     * \param party MPC party running this function
     * \param out Pointer to the output buffer to write squeezed byte into
     * \param nblocks Number of blocks to extract and write into output
     * \param s Pointer to the Keccak state's byte array (1600 bits)
     * \param r Keccak processing rate
     * \return Returns new position in current block
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_keccak_squeezeblocks(MPCParty<T> &party, uint8_t *out, size_t nblocks,
                                        uint64_t s[25], const unsigned int r) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

        while (nblocks) {
            std::vector<bool> circuit_input = primitives::keccak::uint64_to_bool_vector_le(s);
            auto keccak_gc_function = party.get_mpc_context().get_registered_circuit(
                GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600);
            auto circuit_output = run(keccak_gc_function, &circuit_input);
            primitives::keccak::bool_vector_to_uint64_be(circuit_output, s);

            for (int i = 0; i < r / 8; i++)
                primitives::keccak::store64(out + 8 * i, s[i]);
            out += r;
            nblocks -= 1;
        }
    }
}


/**
 * \brief Name space containing threshold implementation of fips 202 Shake functionalities
*/
namespace otpqc::threshold_primitives::shake {
    /**
     * \brief Initializes the Keccak state for the Shake usage
     * \param state Pointer to the Keccak state
     */
    inline void threshold_shake256_init(keccak_state &state) {
        primitives::keccak::keccak_init(state.s);
        state.pos = 0;
    }

    /**
     * \brief Threshold implementation of Shake absorb
     * \param party MPC party running this function
     * \param state Pointer to the Keccak state
     * \param in Input buffer to be absorbed
     * \param inlen Number of bytes to be absorbed from the input
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_shake256_absorb(MPCParty<T> &party, keccak_state &state, const uint8_t *in, size_t inlen) {
        state.pos = keccak::threshold_keccak_absorb(party, state.s, state.pos, SHAKE256_RATE, in, inlen);
    }

    /**
     * \brief Threshold implementation of Shake Finalize function
     * \param party MPC party running this function
     * \param state Pointer to the Keccak state
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_shake256_finalize(MPCParty<T> &party, keccak_state &state) {
        /* Only Party 1 will append the padding. Other parties append 0 to keep the padding boolean shared. */
        if (party.get_id() == 1)
            primitives::keccak::keccak_finalize(state.s, state.pos, SHAKE256_RATE, 0x1F);
        state.pos = SHAKE256_RATE;
    }

    /**
     * \brief Threshold implementation of Shake Squeeze function
     * \param party MPC party running this function
     * \param out Pointer to the output buffer to write squeezed byte into
     * \param outlen Number of bytes to extract and write into output
     * \param state Pointer to the Keccak state
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_shake256_squeeze(MPCParty<T> &party, uint8_t *out, size_t outlen, keccak_state &state) {
        state.pos = keccak::threshold_keccak_squeeze(party, out, outlen, state.s, state.pos, SHAKE256_RATE);
    }

    /**
     * \brief Threshold implementation of Shake Squeeze Blocks
     * \param party MPC party running this function
     * \param out Pointer to the output buffer to write squeezed byte into
     * \param nblocks Number of blocks to extract and write into output
     * \param state Pointer to the Keccak state
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_shake256_squeezeblocks(MPCParty<T> &party, uint8_t *out, size_t nblocks, keccak_state &state) {
        keccak::threshold_keccak_squeezeblocks(party, out, nblocks, state.s, SHAKE256_RATE);
    }
}


#endif //THRESHOLD_FIPS202_H

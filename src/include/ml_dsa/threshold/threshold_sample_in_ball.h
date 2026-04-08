#ifndef THRESHOLD_SAMPLE_IN_BALL_H
#define THRESHOLD_SAMPLE_IN_BALL_H

#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/fips202.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "ml_dsa/threshold/utils/byte_utils.h"

#include "party.h"


namespace otpqc::threshold_signatures::dilithium::poly {
    /**
     * \brief Threshold implementation of Sample in Ball function using Garbled Circut
    *  \param party MPC party running this function
    *  \param seed Input seed passed in boolean shared format
     * \return Array of coefficients in boolean shared format
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::array<uint32_t, DILITHIUM_N> threshold_sample_in_ball(MPCParty<T> &party, uint8_t seed[SHAKE256_RATE]) {
        std::array<uint32_t, DILITHIUM_N> coeffs{};

#if DILITHIUM_MODE == 2
        const auto sample_in_ball_circuit = party.get_mpc_context().get_registered_circuit(
            otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE::DILITHIUM2_SAMPLE_IN_BALL);
#elif DILITHIUM_MODE == 3
        const auto sample_in_ball_circuit = party.get_mpc_context().get_registered_circuit(
            otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE::DILITHIUM3_SAMPLE_IN_BALL);
#else
        const auto sample_in_ball_circuit = party.get_mpc_context().get_registered_circuit(
            otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE::DILITHIUM5_SAMPLE_IN_BALL);
#endif

        const std::vector<bool> input_bits = utils::byte_to_vector_bool_be(seed, SHAKE256_RATE);
        auto result = otpqc::mpc::protocols::gc::circuit::run(sample_in_ball_circuit, &input_bits);

        /*
         * To simplify and optimize our circuit, we realized that coefficients in {-1, 0, 1} can be represented as:
         *          1: Q + 1
         *          0: Q
         *          -1: Q - 1
         *  Looking at three above values, we realize that they only differ in their 2 lowe bits while (23-2) higher
         *  bits are similar. Hence, our circuit encodes 0 as 01, 1 as 10 and -1 as 00 in the output. Upon returning,
         *  we append (Q>>2) to the beginning (only party 1) to make the values in range [0, q-1]. More details have
         *  been presented in the circuit's verilog code in the core_circuits folder.
         */

        /* Base is defined based on the party. Only party 1 has to append (Q>>2) as higher bits. We note that
         * Q>>2 is equal to Q-1, and hence, we do not use shift to construct higher bits/
         */
        const int32_t base = (party.get_id() == 1) ? DILITHIUM_Q - 1 : 0;

        /* Our circuit returns the coefficient in reverse order (high order first). To comply with other parts of our
         * code, we reverse our circuit output.
         */
        for (int i = 0; i < 512; i += 2) {
            /* The 2 bits retuned by the circuit are in Boolean sharing format and hence can be {00, 01, 10, 11} */
            const int bit_pair = (result[i] << 1) | result[i + 1];
            const int coeff_idx = DILITHIUM_N - 1 - (i / 2);
            coeffs[coeff_idx] = base | bit_pair;
        }

        return coeffs;
    }
}


#endif

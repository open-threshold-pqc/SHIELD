#ifndef THRESHOLD_ROUNDING_H
#define THRESHOLD_ROUNDING_H

#include "ml_dsa/threshold/params.h"
#include "mpc/sharing/boolean_sharing.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "math/number.h"
#include "party.h"

namespace otpqc::threshold_signatures::dilithium::rounding {
    /**
     * Threshold implementation of Decompose function
     * \param party MPC party running this function
     * \param coeff Booleans share of a polynomial coefficient
     * \return Boolean shares of HighBits and Boolean shares of LowBits
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<mpc::sharing::BooleanSharing<T>, mpc::sharing::BooleanSharing<T> >
    threshold_decompose(MPCParty<T> &party, const mpc::sharing::BooleanSharing<T> &coeff) {
        using namespace otpqc::mpc::protocols::gc::circuit;

        const auto circuit_input = coeff.get_share().bits_le_ze(DILITHIUM_Q_BITLEN);

        auto &ctx = party.get_mpc_context();

        const auto circuit = ctx.get_registered_circuit(
            GC_FUNCTION_CODE{
                (DILITHIUM_MODE == 2)
                    ? GC_FUNCTION_CODE::DILITHIUM2_DECOMPOSE
                    : GC_FUNCTION_CODE::DILITHIUM35_DECOMPOSE
            }
        );

        const auto circuit_output = run(circuit, &circuit_input);

        constexpr int high_bit_count = (DILITHIUM_MODE == 2) ? 6 : 4;
        constexpr int low_bit_start = high_bit_count;
        constexpr int low_bit_end = (DILITHIUM_MODE == 2) ? 29 : 27;

        uint8_t high_bits = 0;
        for (int i = 0; i < high_bit_count; ++i)
            high_bits = (high_bits << 1) | circuit_output[i];

        uint32_t low_bits = 0;
        for (int i = low_bit_start; i < low_bit_end; ++i)
            low_bits = (low_bits << 1) | circuit_output[i];

        return {
            mpc::sharing::BooleanSharing<T>{math::Number<T>{high_bits}, DILITHIUM_Q_BITLEN},
            mpc::sharing::BooleanSharing<T>{math::Number<T>{low_bits}, DILITHIUM_Q_BITLEN}
        };
    }
}


#endif

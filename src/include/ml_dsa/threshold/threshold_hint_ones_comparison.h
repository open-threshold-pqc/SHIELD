#ifndef THRESHOLD_HINT_COMPARISON_H
#define THRESHOLD_HINT_COMPARISON_H

#include "mpc/protocols/share_conversion/share_conversion.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "party.h"

namespace otpqc::threshold_signatures::dilithium::poly {
    /**
     * \brief Threshold implementation of comparing hint vector number of 1 bits against OMEGA
     * \param party MPC party running this function
     * \param number_of_ones Arithmetic shares of number of 1 bits in the hint vector
     * \return Comparison result (0,1)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    int threshold_hint_one_comparison(MPCParty<T> &party, mpc::sharing::ArithmeticSharing<T> number_of_ones) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;
        using otpqc::mpc::protocols::gc::circuit::GCFunctionContext;

        auto &ctx = party.get_mpc_context();
        std::vector<bool> result{};

        /* Convert number of ones from arithmetic sharing to boolean sharing */
        auto number_of_ones_bool_share = mpc::protocols::ShareConversion<>::a2y(party, number_of_ones);

        //todo error when using bits_be_ze(11) directly on share:
        //   what():  [Math::Number::bits_le_ze] Bit length must be at least the size of the number
        auto bits = number_of_ones_bool_share.get_share().bits_be_ze(DILITHIUM_Q_BITLEN);
        std::vector<bool> circuit_input(11);
        std::copy(
            bits.end() - 11,
            bits.end(),
            circuit_input.begin()
        );

        if constexpr (DILITHIUM_MODE == 2) {
            const GCFunctionContext hint_comparison_circuit = ctx.get_registered_circuit(
                GC_FUNCTION_CODE::DILITHIUM2_HINT_ONE_COMPARISON);
            result = run(hint_comparison_circuit, &circuit_input);
        } else if constexpr (DILITHIUM_MODE == 3) {
            const GCFunctionContext hint_comparison_circuit = ctx.get_registered_circuit(
                GC_FUNCTION_CODE::DILITHIUM3_HINT_ONE_COMPARISON);
            result = run(hint_comparison_circuit, &circuit_input);
        } else {
            const GCFunctionContext hint_comparison_circuit = ctx.get_registered_circuit(
                GC_FUNCTION_CODE::DILITHIUM5_HINT_ONE_COMPARISON);
            result = run(hint_comparison_circuit, &circuit_input);
        }

        /* Communicating with other parties to reconstruct the final comparison result */
        int hint_comparison_result_boolean_share{result[0]};

        /* Send the Check norm result to Party 1 for aggregation */
        auto io = party.get_io();
        if (party.get_id() == 1) {
            int hint_comparison_result_other_response{};
            for (int p = 2; p <= QST_NUM_OF_MPC_PARTIES; p++) {
                io->recv_data(p, &hint_comparison_result_other_response, sizeof(int));
                /* Xor the response received from other parties */
                hint_comparison_result_boolean_share ^= hint_comparison_result_other_response;
            }
        } else {
            io->send_data(1, &hint_comparison_result_boolean_share, sizeof(int));
            io->flush();
        }

        int hint_comparison_result{};

        /* Party 1 sends the aggregated result to other parties */
        if (party.get_id() == 1) {
            /* Sets its own result */
            hint_comparison_result = hint_comparison_result_boolean_share;
            for (int p = 2; p <= QST_NUM_OF_MPC_PARTIES; p++) {
                io->send_data(p, &hint_comparison_result, sizeof(int));
                io->flush();
            }
        } else {
            io->recv_data(1, &hint_comparison_result, sizeof(int));
        }


        return hint_comparison_result;
    }
}
#endif

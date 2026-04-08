#ifndef THRESHOLD_MAKE_HINT_H
#define THRESHOLD_MAKE_HINT_H

#include "ml_dsa/threshold/params.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "party.h"

namespace otpqc::threshold_signatures::dilithium::poly {
    /**
     * \brief Threshold implementation of MakeHint function
     * \param party MPC party running this function
     * \param packed_coefficients1
     * \param packed_coefficients2
     * \return Boolean shares number of 1 bits and boolean shares of hint vector
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    std::tuple<int, std::array<int, DILITHIUM_N> > threshold_make_hint(MPCParty<T> &party,
                                                                       const std::vector<bool> &packed_coefficients1,
                                                                       const std::vector<bool> &packed_coefficients2) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;
        using otpqc::mpc::protocols::gc::circuit::GCFunctionContext;

        auto &ctx = party.get_mpc_context();
        std::vector<bool> result{};

        if constexpr (DILITHIUM_MODE == 2) {
            const GCFunctionContext check_norm_circuit = ctx.get_registered_circuit(
                GC_FUNCTION_CODE::DILITHIUM2_MAKE_HINT);
            result = run(check_norm_circuit, &packed_coefficients1, &packed_coefficients2);
        } else{
            const GCFunctionContext check_norm_circuit = ctx.get_registered_circuit(
                GC_FUNCTION_CODE::DILITHIUM35_MAKE_HINT);
            result = run(check_norm_circuit, &packed_coefficients1, &packed_coefficients2);
        }
        /* Technically, the circuit can extract log2(K * N) bits. But since our circuit is for 1 polynomial at a time,
         * we extract the first log2(N) bits as the sum and the remaining as h */
        int number_of_ones {};
        for (int i=0;i<8;i++) {
            number_of_ones <<= 1;
            number_of_ones |= result[i];
        }
        std::array<int, DILITHIUM_N> poly_hint{};
        for (int i=8; i<result.size();i++)
            poly_hint[i-8] = result[i];
        return std::make_tuple(number_of_ones, poly_hint);
    }
}
#endif
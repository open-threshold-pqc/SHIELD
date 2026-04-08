#ifndef THRESHOLD_CHECK_NORM_H
#define THRESHOLD_CHECK_NORM_H

#include "ml_dsa/threshold/params.h"
#include "mpc/protocols/gc/circuit_exec.h"
#include "party.h"

namespace otpqc::threshold_signatures::dilithium::poly {
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    int threshold_chknorm(MPCParty<T> &party, const std::vector<bool> &packed_coefficients, int32_t bound) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;
        using otpqc::mpc::protocols::gc::circuit::GCFunctionContext;

        auto &ctx = party.get_mpc_context();

        const GCFunctionContext check_norm_circuit = [&]() -> GCFunctionContext {
            if constexpr (DILITHIUM_MODE == 2) {
                if (bound == DILITHIUM_GAMMA1 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA1_BETA);
                if (bound == DILITHIUM_GAMMA2 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2_BETA);
                if (bound == DILITHIUM_GAMMA2)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2);
            } else if constexpr (DILITHIUM_MODE == 3) {
                if (bound == DILITHIUM_GAMMA1 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA1_BETA);
                if (bound == DILITHIUM_GAMMA2 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2_BETA);
                if (bound == DILITHIUM_GAMMA2)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2);
            } else {
                if (bound == DILITHIUM_GAMMA1 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA1_BETA);
                else if (bound == DILITHIUM_GAMMA2 - DILITHIUM_BETA)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2_BETA);
                else if (bound == DILITHIUM_GAMMA2)
                    return ctx.get_registered_circuit(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2);
            }
            throw std::runtime_error("Bound not supported for DILITHIUM_MODE " + std::to_string(DILITHIUM_MODE));
        }();

        auto result = otpqc::mpc::protocols::gc::circuit::run(check_norm_circuit, &packed_coefficients);
        return result[0];
    }
}


#endif

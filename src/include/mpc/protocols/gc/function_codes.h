#ifndef FUNCTION_CODES_H
#define FUNCTION_CODES_H

namespace otpqc::mpc::protocols::gc::circuit {
    /**
     * \brief This enum defines different circuits for GC
     */
    enum class GC_FUNCTION_CODE {
        UNSIGNED_ADD_MOD_4294967295_32_32_32,
        /* Adding two 32-bit numbers % 4294967295 (2^32-1) and outputting 32-bit unsigned add */
        UNSIGNED_ADD_MOD_8380417_23_23_23,
        /* Dilithium modular addition % Q (8380417)*/
        UNSIGNED_ADD_MOD_PRIME_256_256_256,
        /* Adding two 256-bit numbers % (2^256-189) and outputting 32-bit unsigned add */
        UNSIGNED_LESS_THAN_256_256_1,
        /* Comparing two unsigned 256-bit numbers and outputting 1 if first is less than second */
        SHA256_512_0_256, /* Takes 512-bit input and produces 256-bit SHA256 compressed output */
        KECCAK_F_PERMUTATION_1600_1600, /* Keccak-f permutation function (1 call) */
        KECCAK_F_PERMUTATION_1600_1600_CHAIN_5, /* Keccak-f permutation function (5 calls) */
        DILITHIUM2_DECOMPOSE, /* Dilithium 2 Decompose function */
        DILITHIUM35_DECOMPOSE, /* Dilithium 3 and 5 Decompose function */
        DILITHIUM2_SAMPLE_IN_BALL, /* Dilithium 2 Sample in ball function */
        DILITHIUM3_SAMPLE_IN_BALL, /* Dilithium 3 Sample in ball function */
        DILITHIUM5_SAMPLE_IN_BALL, /* Dilithium 5 Sample in ball function */
        DILITHIUM2_CHECK_NORM_GAMMA1_BETA, /* Dilithium 2 check norm for GAMMA1 - BETA bound */
        DILITHIUM3_CHECK_NORM_GAMMA1_BETA, /* Dilithium 3 check norm for GAMMA1 - BETA bound */
        DILITHIUM5_CHECK_NORM_GAMMA1_BETA, /* Dilithium 5 check norm for GAMMA1 - BETA bound */
        DILITHIUM2_CHECK_NORM_GAMMA2_BETA, /* Dilithium 2 check norm for GAMMA2 - BETA bound */
        DILITHIUM3_CHECK_NORM_GAMMA2_BETA, /* Dilithium 3 check norm for GAMMA2 - BETA bound */
        DILITHIUM5_CHECK_NORM_GAMMA2_BETA, /* Dilithium 5 check norm for GAMMA2 - BETA bound */
        DILITHIUM2_CHECK_NORM_GAMMA2, /* Dilithium 2 check norm for GAMMA2 bound */
        DILITHIUM3_CHECK_NORM_GAMMA2, /* Dilithium 3 check norm for GAMMA2 bound */
        DILITHIUM5_CHECK_NORM_GAMMA2, /* Dilithium 5 check norm for GAMMA2 bound */
        DILITHIUM2_MAKE_HINT, /* Dilithium 2 Make hint */
        DILITHIUM35_MAKE_HINT, /* Dilithium 3 and 5 Make hint */
        DILITHIUM2_HINT_ONE_COMPARISON, /* Dilithium 2 hint number of one comparison */
        DILITHIUM3_HINT_ONE_COMPARISON, /* Dilithium 3 hint number of one comparison */
        DILITHIUM5_HINT_ONE_COMPARISON, /* Dilithium 5 hint number of one comparison */


    };

    inline std::string GC_FUNCTION_CODE_STRING(const GC_FUNCTION_CODE code) {
        switch (code) {
            case GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_4294967295_32_32_32:
                return "UNSIGNED_ADD_MOD_4294967295_32_32_32";
            case GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23:
                return "UNSIGNED_ADD_MOD_8380417_23_23_23";
            case GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_PRIME_256_256_256:
                return "UNSIGNED_ADD_MOD_PRIME_256_256_256";
            case GC_FUNCTION_CODE::UNSIGNED_LESS_THAN_256_256_1:
                return "UNSIGNED_LESS_THAN_256_256_1";
            case GC_FUNCTION_CODE::SHA256_512_0_256:
                return "SHA256_512_0_256";
            case GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600:
                return "KECCAK_F_PERMUTATION_1600_1600";
            case GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600_CHAIN_5:
                return "KECCAK_F_PERMUTATION_1600_1600_CHAIN_5";
            case GC_FUNCTION_CODE::DILITHIUM2_DECOMPOSE:
                return "DILITHIUM2_DECOMPOSE";
            case GC_FUNCTION_CODE::DILITHIUM35_DECOMPOSE:
                return "DILITHIUM35_DECOMPOSE";
            case GC_FUNCTION_CODE::DILITHIUM2_SAMPLE_IN_BALL:
                return "DILITHIUM2_SAMPLE_IN_BALL";
            case GC_FUNCTION_CODE::DILITHIUM3_SAMPLE_IN_BALL:
                return "DILITHIUM3_SAMPLE_IN_BALL";
            case GC_FUNCTION_CODE::DILITHIUM5_SAMPLE_IN_BALL:
                return "DILITHIUM5_SAMPLE_IN_BALL";
            case GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA1_BETA:
                return "DILITHIUM2_CHECK_NORM_GAMMA1_BETA";
            case GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA1_BETA:
                return "DILITHIUM3_CHECK_NORM_GAMMA1_BETA";
            case GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA1_BETA:
                return "DILITHIUM5_CHECK_NORM_GAMMA1_BETA";
            case GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2_BETA:
                return "DILITHIUM2_CHECK_NORM_GAMMA2_BETA";
            case GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2_BETA:
                return "DILITHIUM3_CHECK_NORM_GAMMA2_BETA";
            case GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2_BETA:
                return "DILITHIUM5_CHECK_NORM_GAMMA2_BETA";
            case GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2:
                return "DILITHIUM2_CHECK_NORM_GAMMA2";
            case GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2:
                return "DILITHIUM3_CHECK_NORM_GAMMA2";
            case GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2:
                return "DILITHIUM5_CHECK_NORM_GAMMA2";
            case GC_FUNCTION_CODE::DILITHIUM2_MAKE_HINT:
                return "DILITHIUM2_MAKE_HINT";
            case GC_FUNCTION_CODE::DILITHIUM35_MAKE_HINT:
                return "DILITHIUM35_MAKE_HINT";
            case GC_FUNCTION_CODE::DILITHIUM2_HINT_ONE_COMPARISON:
                return "DILITHIUM2_HINT_ONE_COMPARISON";
            case GC_FUNCTION_CODE::DILITHIUM3_HINT_ONE_COMPARISON:
                return "DILITHIUM3_HINT_ONE_COMPARISON";
            case GC_FUNCTION_CODE::DILITHIUM5_HINT_ONE_COMPARISON:
                return "DILITHIUM5_HINT_ONE_COMPARISON";
            default:
                return "UNKNOWN_GC_FUNCTION_CODE";
        }
    }
}

#endif

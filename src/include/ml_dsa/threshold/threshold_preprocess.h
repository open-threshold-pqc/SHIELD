#ifndef THRESHOLD_PREPROCESS_H
#define THRESHOLD_PREPROCESS_H

#include <vector>
#include <memory>

#include "ml_dsa/threshold/params.h"
#include "mpc/protocols/gc/function_codes.h"
#include "mpc/mpc_context.h"
#include "party.h"


namespace otpqc::threshold_signatures::dilithium {
    /**
     * Performs Garbled Circuit Preprocessing
     * \param party MPC party running this function
     */
    inline void gc_preprocess(otpqc::MPCParty<> &party) {
        using otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

        party.setup_communication();

        /* Helper lambda to create, preprocess, and register circuits */
        auto setup_circuits = [&](auto code, size_t count) {
            if (party.get_id() == 1)
                std::cout << "Preprocessing " << count << " " <<
                        mpc::protocols::gc::circuit::GC_FUNCTION_CODE_STRING(code) << " Circuits ....\n";

            auto circuits = otpqc::mpc::MPCContext<>::create_circuit(
                code, count, party.get_io(), &party.get_thread_pool(), party.get_id());

            for (const auto &circuit: circuits) {
                circuit.mpc_gc->preprocess();
                party.get_mpc_context().register_circuit(circuit);
            }
        };

        /* A2Y Conversion
         *      1. threshold_polyveck_decompose: Decomposing w to w1 and w0 requires DILITHIUM_K * DILITHIUM_N conversions
         *      2. Checknorm z: DILITHIUM_L * DILITHIUM_N conversion
         *      3. Checknorm w0: DILITHIUM_K * DILITHIUM_N conversion
         *      4. Checknorm h: DILITHIUM_K * DILITHIUM_N conversion
         *      5. MakeHint requires: DILITHIUM_K * DILITHIUM_N  + 1 (converting last s to bool for > OMEGA)
         */
        constexpr int total_modular_addition_circuits{
            DILITHIUM_K * DILITHIUM_N + DILITHIUM_L * DILITHIUM_N +
            DILITHIUM_K * DILITHIUM_N + DILITHIUM_K * DILITHIUM_N +
            DILITHIUM_K * DILITHIUM_N + 1
        };

        setup_circuits(otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23,
                       total_modular_addition_circuits);

        /* KECCAK permutation
         *  1. Computing rhoprime:
         *      [(DILITHIUM_SEEDBYTES+DILITHIUM_CRHBYTES+DILITHIUM_CRHBYTES+DILITHIUM_CRHBYTES) / (SHAKE256_RATE)]
         *  2. Computing y:
         *      a) threshold_poly_uniform_gamma1:
         *          i) threshold_shake256_stream_init: DILITHIUM_L * (DILITHIUM_CRHBYTES / SHAKE256_RATE)
         *          ii) threshold_shake256_squeezeblocks: DILITHIUM_L * (POLY_UNIFORM_GAMMA1_NBLOCKS)
         *  3. Computing commitment c~:
         *      [DILITHIUM_CRHBYTES + DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES + DILITHIUM_CTILDEBYTES / SHAKE256_RATE]
         */
        auto ceil_div = [](double numerator, double denominator) -> int {
            return static_cast<int>(std::ceil(numerator / denominator));
        };
        int total_keccak_circuits = 0;
        total_keccak_circuits += ceil_div(
            DILITHIUM_SEEDBYTES + DILITHIUM_RNDBYTES + 2 * DILITHIUM_CRHBYTES, SHAKE256_RATE); // rhoprime 1
        total_keccak_circuits += DILITHIUM_L * ceil_div(DILITHIUM_CRHBYTES, SHAKE256_RATE); // shake256_stream_init 1
        total_keccak_circuits += DILITHIUM_L * POLY_UNIFORM_GAMMA1_NBLOCKS; // squeeze blocks
        total_keccak_circuits += ceil_div(
            DILITHIUM_CRHBYTES + DILITHIUM_K * DILITHIUM_POLYW1_PACKEDBYTES + DILITHIUM_CTILDEBYTES,
            SHAKE256_RATE); // c~

        setup_circuits(GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600, total_keccak_circuits);


        /* Decomposing w to w1 and w0 requires DILITHIUM_K * DILITHIUM_N decompose calls */
        /* Challenge requires 1 Sample in Ball circuit */
        /* Norm check z requires DILITHIUM_L circuits */
        /* Make hint needs K make hint functions */
        if constexpr (DILITHIUM_MODE == 2) {
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_DECOMPOSE, DILITHIUM_K * DILITHIUM_N);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_SAMPLE_IN_BALL, 1);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA1_BETA, DILITHIUM_L);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2_BETA, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_CHECK_NORM_GAMMA2, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_MAKE_HINT, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM2_HINT_ONE_COMPARISON, 1);
        } else if constexpr (DILITHIUM_MODE == 3) {
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM35_DECOMPOSE, DILITHIUM_K * DILITHIUM_N);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM3_SAMPLE_IN_BALL, 1);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA1_BETA, DILITHIUM_L);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2_BETA, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM3_CHECK_NORM_GAMMA2, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM35_MAKE_HINT, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM3_HINT_ONE_COMPARISON, 1);
        } else {
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM35_DECOMPOSE, DILITHIUM_K * DILITHIUM_N);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM5_SAMPLE_IN_BALL, 1);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA1_BETA, DILITHIUM_L);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2_BETA, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM5_CHECK_NORM_GAMMA2, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM35_MAKE_HINT, DILITHIUM_K);
            setup_circuits(GC_FUNCTION_CODE::DILITHIUM5_HINT_ONE_COMPARISON, 1);
        }
    }

    /**
     * \brief Performs all the necessary MPC-related preprocessing (e.g., Beaver Triple/daBit generation and GC preprocessing)
     * \param parties Vector of parties among which we want to perform MPC preprocessing
     * \param dilithium_modulus Dilithium modulus
     * \param sharing_global_key Arithmetic Sharing Global Key
     */
    inline void mpc_preprocess(std::vector<std::unique_ptr<otpqc::MPCParty<> > > &parties,
                               const otpqc::math::Number<> &dilithium_modulus,
                               const otpqc::math::Number<> &sharing_global_key) {
        /* Initializing the global mac key share key for the MPC parties */
        auto global_mac_key_shares = otpqc::mpc::sharing::ArithmeticSharing<>::generate_random_global_key_shares(
            dilithium_modulus, sharing_global_key, QST_NUM_OF_MPC_PARTIES);

        for (auto &party: parties)
            party->get_mpc_context().set_global_mac_key_share(global_mac_key_shares[party->get_id() - 1]);

        /* Generating daBit
         * Total:
         *      1. Construction of y requires: DILITHIUM_L * DILITHIUM_N daBits
         *      2. Converting w for HighBits(.): DILITHIUM_K * DILITHIUM_N daBits
         *      3. Converting w0 from bool to Arithmetic for w0-cs2: DILITHIUM_K * DILITHIUM_N daBits
         *      3. Conversion of the challenge polynomial from boolean to Arithmetic: DILITHIUM_N daBits
         *      5. Conversion of z to boolean for norm checking: DILITHIUM_L * DILITHIUM_N
         *      6. Conversion of w0 to boolean for norm checking: DILITHIUM_K * DILITHIUM_N
         *      7. Conversion of h to boolean for norm checking: DILITHIUM_K * DILITHIUM_N
         *      8. Make hint requires: DILITHIUM_K * DILITHIUM_N only for converting w0 (w1 is already boolean shared)
         *      9. Converting number_of_ones as the output of makehint for each polynomial: DILITHIUM_K
         *      10. Converting the number_of_ones from arithmetic to bool for comparison: 1
         */
        constexpr int total_dabits{
            DILITHIUM_L * DILITHIUM_N
            + DILITHIUM_K * DILITHIUM_N
            + DILITHIUM_N + DILITHIUM_L * DILITHIUM_N + DILITHIUM_K * DILITHIUM_N
            + DILITHIUM_K * DILITHIUM_N + 2 * DILITHIUM_K * DILITHIUM_N + DILITHIUM_K + 1
        };
        for (int i = 0; i < total_dabits; i++) {
            otpqc::mpc::Dabit dabit(dilithium_modulus, sharing_global_key, DILITHIUM_Q_BITLEN, QST_NUM_OF_MPC_PARTIES);
            auto dabit_shares = dabit.get_dabit_shares();

            /* Adding the daBit shares to each party's MPC context */
            for (int j = 0; j < QST_NUM_OF_MPC_PARTIES; j++)
                parties[j].get()->get_mpc_context().add_dabit(dabit_shares[j]);
        }

        /* Generating Beaver triples
        * Total:
        *      1. Construction of z = c * s1 requires: DILITHIUM_L * DILITHIUM_N Beaver triples
        *      2. Construction of c_s_2 = c * s2 requires: DILITHIUM_K * DILITHIUM_N Beaver triples
        *      3. Construction of c_t_0 = c * t0 requires: DILITHIUM_K * DILITHIUM_N Beaver triples
        */
        constexpr int total_beaver_triples{DILITHIUM_L * DILITHIUM_N + 2 * DILITHIUM_K * DILITHIUM_N};
        for (int k = 0; k < total_beaver_triples; ++k) {
            otpqc::mpc::BeaverTriple<> beaver_triplet(dilithium_modulus, sharing_global_key, QST_NUM_OF_MPC_PARTIES);
            auto &beaver_triple_shares = beaver_triplet.get_shares();
            for (int i = 0; i < QST_NUM_OF_MPC_PARTIES; ++i) {
                parties[i]->get_mpc_context().add_beaver_triple(beaver_triple_shares[i]);
            }
        }

        /* Perform Garbled Circuits preprocessing */
        std::vector<std::thread> parties_threads;
        parties_threads.reserve(QST_NUM_OF_MPC_PARTIES);

        for (int i = 0; i < QST_NUM_OF_MPC_PARTIES; i++)
            parties_threads.emplace_back(gc_preprocess, std::ref(*parties[i].get()));

        for (int i = 0; i < QST_NUM_OF_MPC_PARTIES; i++)
            parties_threads[i].join();

        /* Generating agreed random values for batch checking
        * Total:
        *   1. Shares Multiplication for the random values: total_beaver_triples * 2
        *   2. A2Y Conversion for the random values: total_modular_addition_circuits
        */
        constexpr int total_modular_addition_circuits{
            DILITHIUM_K * DILITHIUM_N + DILITHIUM_L * DILITHIUM_N +
            DILITHIUM_K * DILITHIUM_N + DILITHIUM_K * DILITHIUM_N +
            DILITHIUM_K * DILITHIUM_N + 1
        };
        const auto total_agreed_random_values = total_beaver_triples * 2 + total_modular_addition_circuits;
        for (int i = 0; i < total_agreed_random_values; ++i) {
            otpqc::math::Number<> agreed_random_value = otpqc::math::Number<>::random(dilithium_modulus);
            for (auto &party: parties)
                party->get_mpc_context().add_agreed_random_value(agreed_random_value);
        }
    }
}

#endif

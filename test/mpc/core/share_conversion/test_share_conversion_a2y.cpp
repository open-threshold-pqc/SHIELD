#include <chrono>
#include <thread>
#include <iostream>

// QST MPC headers
#include "mpc/protocols/share_conversion/share_conversion.h"
#include "mpc/protocols/mac_check/batch_checking.h"
#include "mpc/dabit/dabit.h"

using namespace otpqc::mpc::sharing;
using namespace otpqc::math;

// === Run Secure Computation (Share Conversion) ===
void run_party(otpqc::MPCParty<mpz_class> &party,
               const ArithmeticSharing<mpz_class> &input_share,
               BooleanSharing<mpz_class> &output_share) {
    party.setup_communication();

    auto circuits = otpqc::mpc::MPCContext<mpz_class>::create_circuit(
        otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23,
        1,
        party.get_io(),
        &party.get_thread_pool(),
        party.get_id());

    for (const auto &circuit: circuits) {
        circuit.mpc_gc->preprocess();
        party.get_mpc_context().register_circuit(circuit);
    }

    output_share = otpqc::mpc::protocols::ShareConversion<mpz_class>::a2y(party, input_share);
}

// === Run Batch MAC Checking ===
void run_party_batch_checking(otpqc::MPCParty<mpz_class> &party,
                              const otpqc::math::Number<mpz_class> &modulus,
                              bool &result) {
    party.setup_communication();
    result = otpqc::mpc::protocols::BatchChecking<mpz_class>::perform_batch_checking(party, modulus);
}

#define ITER 1

int main() {
    for (int iter = 0; iter < ITER; iter++) {
        constexpr int bit_length = 23;
        constexpr int number_of_parties = 2;

        Number<mpz_class> modulus{8380417};
        Number<mpz_class> key{4295};
        Number<mpz_class> secret{12};

        // Initialize parties
        otpqc::MPCParty party_1(1, "127.0.0.1", 12346, otpqc::mpc::MPCContext<mpz_class>{});
        otpqc::MPCParty party_2(2, "127.0.0.1", 12346, otpqc::mpc::MPCContext<mpz_class>{});

        // Generate arithmetic secret shares
        auto secret_shares = ArithmeticSharing<mpz_class>::generate_random_shares(
            modulus, secret, key, number_of_parties);

        // Generate dabits
        const otpqc::mpc::Dabit dabit(modulus, key, bit_length, number_of_parties);
        auto dabit_shares = dabit.get_dabit_shares();

        // Set MAC keys
        auto global_mac_key_shares = ArithmeticSharing<mpz_class>::generate_random_global_key_shares(
            modulus, key, number_of_parties);
        party_1.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[0]);
        party_2.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[1]);

        // Add agreed random values
        int agreed_random_values = 1;
        for (int i = 0; i < agreed_random_values; ++i) {
            auto agreed_random_value = Number<mpz_class>::random(modulus);
            party_1.get_mpc_context().add_agreed_random_value(agreed_random_value);
            party_2.get_mpc_context().add_agreed_random_value(agreed_random_value);
        }

        // Load dabits into contexts
        party_1.get_mpc_context().add_dabit(dabit_shares[0]);
        party_2.get_mpc_context().add_dabit(dabit_shares[1]);

        // Run parties
        BooleanSharing<mpz_class> out1{bit_length};
        BooleanSharing<mpz_class> out2{bit_length};

        std::thread thread_party_1(run_party, std::ref(party_1), std::cref(secret_shares[0]), std::ref(out1));
        std::thread thread_party_2(run_party, std::ref(party_2), std::cref(secret_shares[1]), std::ref(out2));

        thread_party_1.join();
        thread_party_2.join();

        auto final_result = out1.get_share() ^ out2.get_share();
        std::cout << "Secret: " << final_result << std::endl;

        // Batch MAC checking
        bool batch_checking_result_party_1 = false;
        bool batch_checking_result_party_2 = false;

        std::thread batch_checking_thread_1(run_party_batch_checking, std::ref(party_1), std::cref(modulus),
                                            std::ref(batch_checking_result_party_1));
        std::thread batch_checking_thread_2(run_party_batch_checking, std::ref(party_2), std::cref(modulus),
                                            std::ref(batch_checking_result_party_2));

        batch_checking_thread_1.join();
        batch_checking_thread_2.join();

        std::cout << "Batch MAC Check Results: Party1=" << batch_checking_result_party_1
                << ", Party2=" << batch_checking_result_party_2 << std::endl;
    }

    return 0;
}

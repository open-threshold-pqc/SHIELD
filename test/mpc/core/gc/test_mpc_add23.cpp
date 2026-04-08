#include <chrono>
#include <thread>
#include <iostream>

// QST MPC headers
#include "mpc/protocols/share_conversion/share_conversion.h"
#include "mpc/protocols/mac_check/batch_checking.h"
#include "mpc/dabit/dabit.h"

using namespace qst::mpc::sharing;

// === Run Secure Computation (Share Conversion) ===
void run_party(qst::MPCParty<mpz_class>& party) {
    party.setup_communication();

    auto circuits = qst::mpc::MPCContext<mpz_class>::create_circuit(
        GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23,
        1,
        party.get_io(),
        &party.get_thread_pool(),
        party.get_id());

    for (const auto& circuit : circuits) {
        circuit.mpc_gc->preprocess();
        party.get_mpc_context().register_circuit(circuit);
    }
    std::vector<bool> input_share(23,false);
    auto f = party.get_mpc_context().get_registered_circuit(GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23);
    auto output_share = qst::mpc::protocols::gc::circuit::run(f, &input_share, &input_share);
    if (party.get_id()==1)
        sleep(1);
    for (int i=0; i<23;i++)
        std::cout << output_share[i];
    std::cout << std::endl;
}


#define ITER 1
int main() {
    for (int iter = 0; iter < ITER; iter++) {
        constexpr int bit_length = 23;
        constexpr int number_of_parties = 2;

        Number<mpz_class> modulus{8380417};
        Number<mpz_class> secret{12};

        // Initialize parties
        qst::MPCParty party_1(1, "127.0.0.1", 12346, qst::mpc::MPCContext<mpz_class>{});
        qst::MPCParty party_2(2, "127.0.0.1", 12346, qst::mpc::MPCContext<mpz_class>{});


        std::thread thread_party_1(run_party, std::ref(party_1));
        std::thread thread_party_2(run_party, std::ref(party_2));

        thread_party_1.join();
        thread_party_2.join();

    }

    return 0;
}

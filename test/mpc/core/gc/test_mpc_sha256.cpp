#include "party.h"
#include "types.h"
#include "mpc/protocols/gc/circuit.h"

void run_party(qst::Party *party) {
    party->setup_communication();
    party->get_mpc_core().register_gc_circuit(qst::mpc::protocols::gc::circuit::FUNCTIONS::SHA256_512_0_256,
    party->get_io(), party->get_id(), 1);

    /* Note: Input wires have been boolean shared among the parties */
    std::vector<bool> input1{};
    std::vector<bool> input2{};

    if (party->get_id() != 1) {
        auto zero = qst::types::Data::zero_data(64);
        input1 = zero.as_bool_vector_le();
    }else {
        auto data = qst::types::Data{"kiarash", 7};
        input1 = data.encode_sha256_chunk();
    }

    auto circuit_output = run(*party,
        qst::mpc::protocols::gc::circuit::FUNCTIONS::SHA256_512_0_256, &input1);

    if (party->get_id() == 2)
        sleep(1);

    std::cout << "Party " << party->get_id() << ": ";
    for (const auto &bit: circuit_output)
        std::cout << bit;

    std::cout << std::endl;
}



int main() {
    qst::Party party_1(1, "127.0.0.1", 12346);
    qst::Party party_2(2, "127.0.0.1", 12346);

    std::thread thread_party_1(run_party, &party_1);
    std::thread thread_party_2(run_party, &party_2);

    thread_party_1.join();
    thread_party_2.join();
}

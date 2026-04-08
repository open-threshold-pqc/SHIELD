// #include "party.h"
// #include "mpc/protocols/gc/circuit.h"
//
// void run_party(qst::Party *party) {
//     party->setup_communication();
//     party->get_mpc_core().register_gc_circuit(qst::mpc::protocols::gc::circuit::FUNCTIONS::UNSIGNED_ADD_32_32_33,
//     party->get_io(), party->get_id(), 3);
//
//     /* Note: Input wires have been boolean shared among the parties */
//     std::vector<bool> input1{};
//     std::vector<bool> input2{};
//     for (int i = 0; i < 32; i++) {
//      input1.push_back(false);
//      input2.push_back(false);
//     }
//     if (party->get_id() == 1)
//         input1[30] = true;
//
//     auto circuit_output = qst::mpc::protocols::gc::circuit::run(*party,
//         qst::mpc::protocols::gc::circuit::FUNCTIONS::UNSIGNED_ADD_32_32_33, &input1, &input2);
//
//     if (party->get_id() == 2)
//         sleep(1);
//
//     std::cout << "Party " << party->get_id() << ": ";
//     for (const auto &bit: circuit_output)
//      std::cout << bit;
//
//     std::cout << std::endl;
// }
//
//
//
// int main() {
//     qst::Party party_1(1, "127.0.0.1", 12346);
//     qst::Party party_2(2, "127.0.0.1", 12346);
//
//     std::thread thread_party_1(run_party, &party_1);
//     std::thread thread_party_2(run_party, &party_2);
//
//     thread_party_1.join();
//     thread_party_2.join();
// }

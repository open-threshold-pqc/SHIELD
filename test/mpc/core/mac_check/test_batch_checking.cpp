#include "mpc/protocols/share_multiplication/share_multiplication.h"
#include "mpc/beaver_triple/beaver_triple.h"
#include "mpc/protocols/mac_check/batch_checking.h"
// #include "mpc/dabit/dabit.h"
#include <gtest/gtest.h>

#include <chrono>

using namespace qst::mpc::protocols;
using namespace qst::mpc::sharing;

void run_party(qst::MPCParty<>& party,
                const ArithmeticSharing<>& s1,
                const ArithmeticSharing<>& s2,
                ArithmeticSharing<>& result) {
    party.setup_communication();
    result = ShareMultiplication<>::multiply_arithmetic_shares(party, s1, s2);
}

void run_party_batch_checking(qst::MPCParty<>& party,
                const Number<>& modulus,
                bool& result) {
    party.setup_communication();
    result = BatchChecking<>::perform_batch_checking(party, modulus);
}
                

TEST(ShareMultiplicationTest, MultiplicationCorrectness) {
    Number<> modulus(97);
    Number<> global_mac_key(42);


    qst::MPCParty party1{1, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};
    qst::MPCParty party2{2, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};

    qst::mpc::BeaverTriple beaver_triplet(modulus, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    auto beaver_triple_shares = beaver_triplet.get_shares();
    party1.get_mpc_context().add_beaver_triple(beaver_triple_shares[0]);
    party2.get_mpc_context().add_beaver_triple(beaver_triple_shares[1]);

    auto global_mac_key_shares = qst::mpc::sharing::ArithmeticSharing<>::generate_random_global_key_shares(modulus, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    party1.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[0]);
    party2.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[1]);

    // add agreed random values
    int num_agreed_random_values = 2;
    for (int i = 0; i < num_agreed_random_values; ++i) {
        Number<> agreed_random_value = Number<>::random(modulus);
        party1.get_mpc_context().add_agreed_random_value(agreed_random_value);
        party2.get_mpc_context().add_agreed_random_value(agreed_random_value);
    }

    
    Number<> secret1(100);
    Number<> secret2(200);
    std::vector<qst::mpc::sharing::ArithmeticSharing<>> secret_shares_1 = qst::mpc::sharing::ArithmeticSharing<>::generate_random_shares(modulus, secret1, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    std::vector<qst::mpc::sharing::ArithmeticSharing<>> secret_shares_2 = qst::mpc::sharing::ArithmeticSharing<>::generate_random_shares(modulus, secret2, global_mac_key, QST_NUM_OF_MPC_PARTIES);

    ArithmeticSharing<> result_party_1(secret_shares_1[0]);
    ArithmeticSharing<> result_party_2(result_party_1);
    thread t1(run_party, std::ref(party1), secret_shares_1[0], secret_shares_2[0], std::ref(result_party_1));
    thread t2(run_party, std::ref(party2), secret_shares_1[1], secret_shares_2[1], std::ref(result_party_2));
    t1.join();
    t2.join();

    ArithmeticSharing<> final_result = result_party_1 + result_party_2;
    Number<> expected_share_result = (secret1 * secret2) % modulus;
    Number<> expected_mac_share = (expected_share_result * global_mac_key) % modulus;
    
    EXPECT_EQ(final_result.get_share(), expected_share_result);
    EXPECT_EQ(final_result.get_mac_share(), expected_mac_share);

    // Perform batch checking
    bool batch_checking_result_party_1 = false;
    bool batch_checking_result_party_2 = false;
    std::thread t3(run_party_batch_checking, std::ref(party1), std::ref(modulus), std::ref(batch_checking_result_party_1));
    std::thread t4(run_party_batch_checking, std::ref(party2), std::ref(modulus), std::ref(batch_checking_result_party_2));
    t3.join();
    t4.join();
    EXPECT_TRUE(batch_checking_result_party_1);
    EXPECT_TRUE(batch_checking_result_party_2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
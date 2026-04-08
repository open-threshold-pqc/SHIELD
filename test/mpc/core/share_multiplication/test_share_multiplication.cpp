#include "mpc/protocols/share_multiplication/share_multiplication.h"
#include "mpc/beaver_triple/beaver_triple.h"
// #include "mpc/dabit/dabit.h"
#include <gtest/gtest.h>

#include <chrono>

using namespace otpqc::mpc::protocols;
using namespace otpqc::mpc::sharing;
using namespace otpqc::math;

void run_party(otpqc::MPCParty<>& party,
                const ArithmeticSharing<>& s1,
                const ArithmeticSharing<>& s2,
                ArithmeticSharing<>& result) {
    party.setup_communication();
    auto start = std::chrono::high_resolution_clock::now();
    result = ShareMultiplication<>::multiply_arithmetic_shares(party, s1, s2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Party " << party.get_id() << " multiplication time: " << duration << " nanoseconds\n";
}
                

TEST(ShareMultiplicationTest, MultiplicationCorrectness) {
    Number<> modulus(8380417);
    Number<> global_mac_key(1234567);


    otpqc::MPCParty party1{1, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};
    otpqc::MPCParty party2{2, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};

    otpqc::mpc::BeaverTriple beaver_triplet(modulus, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    auto beaver_triple_shares = beaver_triplet.get_shares();
    party1.get_mpc_context().add_beaver_triple(beaver_triple_shares[0]);
    party2.get_mpc_context().add_beaver_triple(beaver_triple_shares[1]);

    auto global_mac_key_shares = otpqc::mpc::sharing::ArithmeticSharing<>::generate_random_global_key_shares(modulus, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    party1.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[0]);
    party2.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[1]);

    
    Number<> secret1(100);
    Number<> secret2(200);
    std::vector<otpqc::mpc::sharing::ArithmeticSharing<>> secret_shares_1 = otpqc::mpc::sharing::ArithmeticSharing<>::generate_random_shares(modulus, secret1, global_mac_key, QST_NUM_OF_MPC_PARTIES);
    std::vector<otpqc::mpc::sharing::ArithmeticSharing<>> secret_shares_2 = otpqc::mpc::sharing::ArithmeticSharing<>::generate_random_shares(modulus, secret2, global_mac_key, QST_NUM_OF_MPC_PARTIES);

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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
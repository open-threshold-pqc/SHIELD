#include <gtest/gtest.h>
#include "mpc/beaver_triple/beaver_triple.h"


TEST(BeaverTripleTest, SharesRecoverCorrectly) {
    using dt = QST_UNDERLYING_NUMERIC_TYPE;
    Number<> modulus(97);
    Number<> global_mac_key(42);
    int share_count = 5;
    qst::mpc::BeaverTriple beaver_triple(modulus, global_mac_key, share_count);
    const auto &shares = beaver_triple.get_shares();
    ASSERT_EQ(shares.size(), share_count);

    // Sum all the a, b, c shares
    Number<> a_sum(0), b_sum(0), c_sum(0);
    Number<> a_mac_sum(0), b_mac_sum(0), c_mac_sum(0);
    for (const auto &share: shares) {
        a_sum = (a_sum + share.a_share) % modulus;
        b_sum = (b_sum + share.b_share) % modulus;
        c_sum = (c_sum + share.c_share) % modulus;
        a_mac_sum = (a_mac_sum + share.a_mac_share) % modulus;
        b_mac_sum = (b_mac_sum + share.b_mac_share) % modulus;
        c_mac_sum = (c_mac_sum + share.c_mac_share) % modulus;
    }
    // Check that sumC is equal to sumA * sumB mod modulus
    Number<> expected_c_sum = (a_sum * b_sum) % modulus;
    ASSERT_EQ(c_sum, expected_c_sum);
    // Check MAC shares match number * global_mac_key
    auto expected_a_mac_sum = (a_sum * global_mac_key) % modulus;
    auto expected_b_mac_sum = (b_sum * global_mac_key) % modulus;
    auto expected_c_mac_sum = (c_sum * global_mac_key) % modulus;
    ASSERT_EQ(a_mac_sum, expected_a_mac_sum);
    ASSERT_EQ(b_mac_sum, expected_b_mac_sum);
    ASSERT_EQ(c_mac_sum, expected_c_mac_sum);
}


TEST(BeaverTripleTest, RandomnessCheck) {
    Number<> modulus(97);
    Number<> global_mac_key(42);
    std::set<Number<> > a_sums;
    std::set<Number<> > b_sums;

    for (int i = 0; i < 10; i++) {
        qst::mpc::BeaverTriple triple(modulus, global_mac_key, 5);
        const auto &shares = triple.get_shares();
        Number<> a_sum(0), b_sum(0);
        for (const auto &share: shares) {
            a_sum = (a_sum + share.a_share) % modulus;
            b_sum = (b_sum + share.b_share) % modulus;
        }
        a_sums.insert(a_sum);
        b_sums.insert(b_sum);
    }

    // Naive check for variability
    ASSERT_GT(a_sums.size(), 1);
    ASSERT_GT(b_sums.size(), 1);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

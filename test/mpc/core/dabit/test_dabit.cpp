#include <gtest/gtest.h>
#include "mpc/dabit/dabit.h"

using namespace otpqc::mpc;
using namespace otpqc::math;

using NumberType = int32_t; // adjust if needed

// Mock or helper to create Number<T> from raw value
Number<NumberType> make_number(NumberType val) {
    return Number<NumberType>(val);
}

// Helper to generate a modulus larger than bit_length bits
Number<NumberType> generate_modulus(int bit_length) {
    // For simplicity: (1 << bit_length) + 1 as modulus, fits bit_length+1 bits
    return Number<NumberType>((NumberType(1) << bit_length) + 1);
}


// Test basic construction and get_dabit_shares properties
TEST(DabitTest, ConstructAndGetDabitShares) {
    Number<> modulus{9};
    Number<> key{3};
    constexpr int bit_length{4};
    constexpr int share_count{2};

    Dabit<> dabit(modulus, key, bit_length, share_count);

    auto shares = dabit.get_dabit_shares();

    EXPECT_EQ((int)shares.size(), share_count);

    for (const auto &share: shares) {
        EXPECT_EQ(share.length, bit_length);
        EXPECT_EQ((int)share.a_shares.size(), bit_length);
        EXPECT_EQ((int)share.b_shares.size(), bit_length);

        // Check that arithmetic shares and boolean shares have compatible values
        for (int i = 0; i < bit_length; ++i) {
            // Check arithmetic share modulus matches input modulus
            EXPECT_EQ(share.a_shares[i].get_modulus(), modulus);

            // Boolean shares bit length must be 1
            EXPECT_EQ(share.b_shares[i].get_length(), 1);

            // Boolean share is either zero or one (check is_zero or is_one)
            EXPECT_TRUE(share.b_shares[i].is_zero() || share.b_shares[i].is_one());
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

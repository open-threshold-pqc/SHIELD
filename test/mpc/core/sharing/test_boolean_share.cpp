#include "mpc/sharing/boolean_sharing.h"
#include <gtest/gtest.h>

using namespace otpqc::mpc::sharing;
using namespace otpqc::math;

using NumberType = uint32_t; // assuming QST_UNDERLYING_NUMERIC_TYPE is uint32_t for testing

// Helper to create Number<T> with specific value (assuming Number<T> supports this constructor)
Number<NumberType> make_number(NumberType val) {
    return Number<NumberType>(val);
}

TEST(BooleanSharingTest, Constructor_InvalidBitLength_Throws) {
    EXPECT_THROW(BooleanSharing<NumberType> share(0), std::invalid_argument);
    EXPECT_THROW(BooleanSharing<NumberType> share(-1), std::invalid_argument);
}

TEST(BooleanSharingTest, Constructor_NumberBitLengthCheck) {
    NumberType val = 3; // binary 11, true bit length = 2
    int bit_length = 1; // smaller than true bit length

    Number<NumberType> number = make_number(val);
    EXPECT_THROW(BooleanSharing<NumberType> share(number, bit_length), std::invalid_argument);

    bit_length = 33; // greater than sizeof(T)*8 for uint32_t(32 bits)
    if constexpr (sizeof(NumberType) == 4) {
        EXPECT_THROW(BooleanSharing<NumberType> share(number, bit_length), std::invalid_argument);
    }

    // valid case
    bit_length = 2;
    EXPECT_NO_THROW(BooleanSharing<NumberType> share(number, bit_length));
}

TEST(BooleanSharingTest, Constructor_BitsVector) {
    std::vector<bool> bits = {true, false, true}; // valid
    EXPECT_NO_THROW(BooleanSharing<NumberType> share(bits));

    std::vector<bool> empty_bits;
    EXPECT_THROW(BooleanSharing<NumberType> share(empty_bits), std::invalid_argument);
}

TEST(BooleanSharingTest, CopyMoveConstructorsAndAssignments) {
    BooleanSharing<NumberType> original(3);
    BooleanSharing<NumberType> copy = original;
    BooleanSharing<NumberType> moved = std::move(copy);

    BooleanSharing<NumberType> assign(3);
    assign = original;

    BooleanSharing<NumberType> move_assign(3);
    move_assign = std::move(assign);

    SUCCEED(); // if no exceptions, test passes
}

TEST(BooleanSharingTest, GettersAndFlags) {
    Number<NumberType> zero_num = make_number(0);
    Number<NumberType> one_num = make_number(1);
    Number<NumberType> val_num = make_number(5);

    BooleanSharing<NumberType> zero_share(zero_num, 3);
    BooleanSharing<NumberType> one_share(one_num, 3);
    BooleanSharing<NumberType> val_share(val_num, 3);

    EXPECT_TRUE(zero_share.is_zero());
    EXPECT_FALSE(zero_share.is_one());

    EXPECT_FALSE(one_share.is_zero());
    EXPECT_TRUE(one_share.is_one());

    EXPECT_FALSE(val_share.is_zero());
    EXPECT_FALSE(val_share.is_one());

    EXPECT_EQ(zero_share.get_length(), 3);
    EXPECT_EQ(one_share.get_length(), 3);
    EXPECT_EQ(val_share.get_length(), 3);

    EXPECT_EQ(zero_share.get_share(), zero_num);
}

TEST(BooleanSharingTest, BitsLEandBE) {
    Number<NumberType> num = make_number(0b101); // 5
    BooleanSharing<NumberType> share(num, 3);

    auto bits_le = share.bits_le();
    ASSERT_EQ(bits_le.size(), 3);
    EXPECT_EQ(bits_le[0], true);
    EXPECT_EQ(bits_le[1], false);
    EXPECT_EQ(bits_le[2], true);

    auto bits_be = share.bits_be();
    ASSERT_EQ(bits_be.size(), 3);
    EXPECT_EQ(bits_be[0], true);
    EXPECT_EQ(bits_be[1], false);
    EXPECT_EQ(bits_be[2], true);
}

TEST(BooleanSharingTest, GenerateBooleanShares_Valid) {
    Number<NumberType> number = make_number(7); // 111
    int bit_length = 3;
    int count = 5;

    auto shares = BooleanSharing<NumberType>::generate_random_shares(number, bit_length, count);
    EXPECT_EQ((int)shares.size(), count);

    // XOR all shares should be equal to the original number
    Number<NumberType> xor_sum{};
    for (const auto &share: shares) {
        xor_sum ^= share.get_share();
    }
    EXPECT_EQ(xor_sum, number);
}

TEST(BooleanSharingTest, GenerateBooleanShares_InvalidBitLength) {
    Number<NumberType> number = make_number(7); // true bit length is 3
    int bit_length = 2; // less than true bit length

    EXPECT_THROW(BooleanSharing<NumberType>::generate_random_shares(number, bit_length, 3), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

#include "math/bits.h"


using namespace otpqc::math;

TEST(BitsToBytesBE, EmptyInput) {
    std::vector<bool> bits = {};
    auto bytes = bits_to_bytes_be(bits);
    EXPECT_TRUE(bytes.empty());
}

TEST(BitsToBytesBE, SingleByteMSBPadding) {
    std::vector<bool> bits = {1, 1, 0, 0, 1}; // Should pad to: 00011001 = 0x19
    auto bytes = bits_to_bytes_be(bits);
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x19);
}

TEST(BitsToBytesBE, MultipleBytesMSBPadding) {
    std::vector<bool> bits = {
        1, 0, 1, 1, 1, 0, 0, 1, // byte 1 = 0xB9
        1, 1, 0 // byte 2 = 110xxxxx -> padded to 11000000 = 0xC0
    };
    // byte1: 0 0 0 0 0 1 0 1
    // byte2: 1 1 0 0 1 1 1 0
    auto bytes = bits_to_bytes_be(bits);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x5);
    EXPECT_EQ(bytes[1], 206);
}

TEST(BitsToBytesLE, ReverseOfBE) {
    std::vector<bool> bits = {1, 1, 0, 0, 1}; // Padded to 0x19
    auto bytes_le = bits_to_bytes_le(bits);
    ASSERT_EQ(bytes_le.size(), 1);
    EXPECT_EQ(bytes_le[0], 0x19); // Reversing 1 byte is the same
}

TEST(TrueBitLength, BuiltinUnsigned) {
    EXPECT_EQ(true_bit_length(uint8_t{0b00000000}), 1);
    EXPECT_EQ(true_bit_length(uint8_t{0b00000001}), 1);
    EXPECT_EQ(true_bit_length(uint8_t{0b10000000}), 8);
    EXPECT_EQ(true_bit_length(uint16_t{0b1000000000000000}), 16);
}

TEST(TrueBitLength, BuiltinSignedPositive) {
    EXPECT_EQ(true_bit_length(int8_t{127}), 7);
    EXPECT_EQ(true_bit_length(int16_t{255}), 8);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

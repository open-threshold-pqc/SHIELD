#include "math/number.h"

#include <gtest/gtest.h>
#include <vector>


using otpqc::math::Number;

template<typename T>
class NumberTest : public ::testing::Test {
};

using IntegralTypes = ::testing::Types<int32_t, uint32_t, mpz_class>;
TYPED_TEST_SUITE(NumberTest, IntegralTypes);

// Helper for creating vector from bytes
static std::vector<uint8_t> to_bytes(const std::vector<uint8_t> &v) { return v; }

TYPED_TEST(NumberTest, DefaultConstructorAndGetValue) {
    Number<TypeParam> n;
    EXPECT_EQ(n.get_value(), TypeParam(0));
}

TYPED_TEST(NumberTest, ConstructFromValue) {
    Number<TypeParam> n1(TypeParam(42));
    EXPECT_EQ(n1.get_value(), TypeParam(42));

    // Copy constructor
    Number<TypeParam> n2(n1);
    EXPECT_EQ(n2.get_value(), TypeParam(42));
}

TYPED_TEST(NumberTest, ConstructFromConvertible) {
    if constexpr (otpqc::math::GmpNumeric<TypeParam>) {
        Number<TypeParam> n(std::string("12345678901234567890"));
        EXPECT_GT(n.get_value(), TypeParam(0));
        EXPECT_EQ(n.get_value(), TypeParam("12345678901234567890"));
    } else {
        Number<TypeParam> n(100);
        EXPECT_EQ(n.get_value(), TypeParam(100));
    }
}

TYPED_TEST(NumberTest, ConstructFromBytes) {
    if constexpr (otpqc::math::PrimitiveNumeric<TypeParam>) {
        // bytes fitting exactly sizeof(TypeParam)
        std::vector<uint8_t> bytes(sizeof(TypeParam), 0);
        bytes.back() = 0xFF; // last byte = 255
        Number<TypeParam> n(bytes);
        if constexpr (sizeof(TypeParam) == 1) {
            EXPECT_EQ(n.get_value(), static_cast<TypeParam>(255));
        } else {
            EXPECT_GT(n.get_value(), 0);
            EXPECT_EQ(n.get_value(), TypeParam(0xFF));
        }
        // Exception: bytes too large
        std::vector<uint8_t> too_big(sizeof(TypeParam) + 1, 1);
        EXPECT_THROW(Number<TypeParam> bad(too_big), std::invalid_argument);
    } else if constexpr (otpqc::math::GmpNumeric<TypeParam>) {
        // For GMP just test non-empty vector, expect no throw
        std::vector<uint8_t> bytes = {1, 2, 3, 4};
        Number<TypeParam> n(bytes);
        EXPECT_GT(n.get_value(), 0);
    }
}

TYPED_TEST(NumberTest, ArithmeticOperators) {
    Number<TypeParam> a(10), b(3);

    auto c = a + b;
    EXPECT_EQ(c.get_value(), TypeParam(13));

    c = a - b;
    EXPECT_EQ(c.get_value(), TypeParam(7));

    c = a * b;
    EXPECT_EQ(c.get_value(), TypeParam(30));

    c = a / b;
    EXPECT_EQ(c.get_value(), TypeParam(3));

    // Modulo: test positive remainder
    c = a % b;
    EXPECT_EQ(c.get_value(), TypeParam(1));

    if constexpr (!std::is_unsigned_v<TypeParam>) {
        // Modulo: test negative remainder correction
        Number<TypeParam> neg_a(-10);
        c = neg_a % b;
        EXPECT_GE(c.get_value(), 0); // remainder should be corrected to positive
        EXPECT_EQ(c.get_value(), TypeParam(2)); // -10 % 3 = 2
    }
}

TYPED_TEST(NumberTest, BitwiseXorOperators) {
    Number<TypeParam> a(5), b(3);
    auto c = a ^ b;
    EXPECT_EQ(c.get_value(), TypeParam(5 ^ 3));

    Number<TypeParam> d(1);
    d ^= b;
    EXPECT_EQ(d.get_value(), TypeParam(1 ^ 3));
}

TYPED_TEST(NumberTest, CompoundAssignmentOperators) {
    Number<TypeParam> a(10), b(4);
    a += b;
    EXPECT_EQ(a.get_value(), TypeParam(14));

    a -= b;
    EXPECT_EQ(a.get_value(), TypeParam(10));

    a *= b;
    EXPECT_EQ(a.get_value(), TypeParam(40));

    a /= b;
    EXPECT_EQ(a.get_value(), TypeParam(10));

    a ^= b;
    EXPECT_EQ(a.get_value(), TypeParam(10 ^ 4));
}

TYPED_TEST(NumberTest, LeftShiftOperator) {
    Number<TypeParam> a(1);
    auto b = a << 3;
    EXPECT_EQ(b.get_value(), TypeParam(1 << 3));
}

TYPED_TEST(NumberTest, MultiplyWithPrimitive) {
    Number<TypeParam> a(2);
    auto b = a * 3;
    EXPECT_EQ(b.get_value(), TypeParam(6));
}

TYPED_TEST(NumberTest, ComparisonOperators) {
    Number<TypeParam> a(5), b(7);

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= 5);
    EXPECT_TRUE(b >= 7);
    EXPECT_TRUE(a == 5);
    EXPECT_TRUE(a != b);

    EXPECT_TRUE(5 == a);
    EXPECT_TRUE(7 > a);
    EXPECT_TRUE(5 <= a);
    EXPECT_TRUE(7 >= a);
}

TYPED_TEST(NumberTest, ExplicitCastOperator) {
    Number<TypeParam> a(42);
    TypeParam val = static_cast<TypeParam>(a);
    EXPECT_EQ(val, 42);

    if constexpr (!otpqc::math::GmpNumeric<TypeParam>) {
        Number<int32_t> b = static_cast<Number<int32_t>>(a);
        EXPECT_EQ(static_cast<int32_t>(b.get_value()), static_cast<int32_t>(a.get_value()));
    }
}

TYPED_TEST(NumberTest, LinearizedAndBits) {
    Number<TypeParam> a(0x1234);
    auto bytes = a.linearized();
    EXPECT_FALSE(bytes.empty());

    auto bits_le = a.bits_le();
    EXPECT_FALSE(bits_le.empty());

    auto bits_be = a.bits_be();
    EXPECT_EQ(bits_be.size(), bits_le.size());

    if constexpr (std::is_signed_v<TypeParam>) {
        auto bits_le_se = a.bits_le_se(16);
        EXPECT_EQ(bits_le_se.size(), 16);

        auto bits_be_se = a.bits_be_se(16);
        EXPECT_EQ(bits_be_se.size(), 16);

        EXPECT_THROW(a.bits_le_se(8), std::invalid_argument);
        EXPECT_THROW(a.bits_be_se(8), std::invalid_argument);
    }

    auto bits_le_ze = a.bits_le_ze(16);
    EXPECT_EQ(bits_le_ze.size(), 16);
    auto bits_be_ze = a.bits_be_ze(16);
    EXPECT_EQ(bits_be_ze.size(), 16);

    EXPECT_THROW(a.bits_le_ze(8), std::invalid_argument);
    EXPECT_THROW(a.bits_be_ze(8), std::invalid_argument);
}

TYPED_TEST(NumberTest, StaticRandomModulus) {
    Number<TypeParam> mod(100);
    auto r = Number<TypeParam>::random(mod);
    EXPECT_GE(r.get_value(), 0);
    EXPECT_LT(r.get_value(), mod.get_value());

    // test modulus <= 0 throws
    Number<TypeParam> zero_mod(0);
    EXPECT_THROW(Number<TypeParam>::random(zero_mod), std::invalid_argument);
}

TYPED_TEST(NumberTest, StaticRandomBitLength) {
    // test positive bit_length generates number
    auto r = Number<TypeParam>::random(4);
    (void) r; // just ensure no exceptions

    // test negative or zero bit_length throws
    EXPECT_THROW(Number<TypeParam>::random(0), std::invalid_argument);
    EXPECT_THROW(Number<TypeParam>::random(-1), std::invalid_argument);

    // test bit_length > type bits throws for primitives
    if constexpr (otpqc::math::PrimitiveIntegral<TypeParam>) {
        int bits = sizeof(TypeParam) * 8;
        EXPECT_THROW(Number<TypeParam>::random(bits + 1), std::invalid_argument);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

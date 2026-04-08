#include "mpc/sharing/arithmetic_sharing.h"
#include "math/number.h"

#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>

using namespace otpqc::mpc::sharing;
using namespace otpqc::math;

template<typename T>
class ArithmeticSharingTest : public ::testing::Test {};

using TestTypes = ::testing::Types<int32_t, mpz_class>;
TYPED_TEST_SUITE(ArithmeticSharingTest, TestTypes);

TYPED_TEST(ArithmeticSharingTest, ConstructorValidAndInvalid) {
    Number<TypeParam> modulus(17);
    Number<TypeParam> share(5);
    Number<TypeParam> mac(10);

    // Valid construction
    EXPECT_NO_THROW({
        ArithmeticSharing<TypeParam> s(modulus, share, mac);
        EXPECT_EQ(s.get_modulus(), modulus);
        EXPECT_EQ(s.get_share(), share);
        EXPECT_EQ(s.get_mac_share(), mac);
    });

    // Invalid modulus <= 0
    Number<TypeParam> zero_mod(0);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(zero_mod, share, mac), std::invalid_argument);

    Number<TypeParam> neg_mod(-1);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(neg_mod, share, mac), std::invalid_argument);

    // share < 0
    Number<TypeParam> neg_share(-1);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(modulus, neg_share, mac), std::invalid_argument);

    // share >= modulus
    Number<TypeParam> large_share(17);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(modulus, large_share, mac), std::invalid_argument);

    // mac < 0
    Number<TypeParam> neg_mac(-2);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(modulus, share, neg_mac), std::invalid_argument);

    // mac >= modulus
    Number<TypeParam> large_mac(20);
    EXPECT_THROW(ArithmeticSharing<TypeParam> s(modulus, share, large_mac), std::invalid_argument);
}

TYPED_TEST(ArithmeticSharingTest, CopyAndMoveOperations) {
    Number<TypeParam> modulus(19);
    Number<TypeParam> share(7);
    Number<TypeParam> mac(3);

    ArithmeticSharing<TypeParam> original(modulus, share, mac);

    // Copy constructor
    ArithmeticSharing<TypeParam> copy(original);
    EXPECT_EQ(copy.get_modulus(), modulus);
    EXPECT_EQ(copy.get_share(), share);
    EXPECT_EQ(copy.get_mac_share(), mac);

    // Move constructor
    ArithmeticSharing<TypeParam> moved(std::move(copy));
    EXPECT_EQ(moved.get_modulus(), modulus);
    EXPECT_EQ(moved.get_share(), share);
    EXPECT_EQ(moved.get_mac_share(), mac);

    // Copy assignment
    ArithmeticSharing<TypeParam> assign(modulus, Number<TypeParam>(1), Number<TypeParam>(1));
    assign = moved;
    EXPECT_EQ(assign.get_modulus(), modulus);
    EXPECT_EQ(assign.get_share(), share);
    EXPECT_EQ(assign.get_mac_share(), mac);

    // Move assignment
    ArithmeticSharing<TypeParam> move_assign(modulus, Number<TypeParam>(1), Number<TypeParam>(1));
    move_assign = std::move(assign);
    EXPECT_EQ(move_assign.get_modulus(), modulus);
    EXPECT_EQ(move_assign.get_share(), share);
    EXPECT_EQ(move_assign.get_mac_share(), mac);
}

TYPED_TEST(ArithmeticSharingTest, AdditionAndSubtractionOperators) {
    Number<TypeParam> modulus(23);
    ArithmeticSharing<TypeParam> a(modulus, Number<TypeParam>(10), Number<TypeParam>(7));
    ArithmeticSharing<TypeParam> b(modulus, Number<TypeParam>(20), Number<TypeParam>(15));

    // operator+
    ArithmeticSharing<TypeParam> c = a + b;
    // (10+20) % 23 = 7, (7+15) % 23 = 22
    EXPECT_EQ(c.get_share(), Number<TypeParam>(7));
    EXPECT_EQ(c.get_mac_share(), Number<TypeParam>(22));

    // operator+(T number and ArithmeticSharing)
    Number<TypeParam> number(3);
    c = number + a;
    // (3+10) % 23 = 13, (3+7) % 23 = 10
    EXPECT_EQ(c.get_share(), Number<TypeParam>(13));
    EXPECT_EQ(c.get_mac_share(), Number<TypeParam>(10));
    // operator+(ArithmeticSharing and T number)
    c = a + number;
    // (10+3) % 23 = 13, (7+3) % 23 = 10
    EXPECT_EQ(c.get_share(), Number<TypeParam>(13));
    EXPECT_EQ(c.get_mac_share(), Number<TypeParam>(10));

    // operator+=
    ArithmeticSharing<TypeParam> d = a;
    d += b;
    EXPECT_EQ(d.get_share(), Number<TypeParam>(7));
    EXPECT_EQ(d.get_mac_share(), Number<TypeParam>(22));

    // operator+= with T number
    d += number;
    EXPECT_EQ(d.get_share(), Number<TypeParam>(10));
    EXPECT_EQ(d.get_mac_share(), Number<TypeParam>(2));

    // operator-
    ArithmeticSharing<TypeParam> e = b - a;
    // (20-10) % 23 = 10, (15-7) % 23 = 8
    EXPECT_EQ(e.get_share(), Number<TypeParam>(10));
    EXPECT_EQ(e.get_mac_share(), Number<TypeParam>(8));

    // operator-(T number and ArithmeticSharing)
    e = number - e;
    // (3-10) % 23 = 16, (3-8) % 23 = 18
    EXPECT_EQ(e.get_share(), Number<TypeParam>(16));
    EXPECT_EQ(e.get_mac_share(), Number<TypeParam>(18));
    // operator-(ArithmeticSharing and T number)
    e = e - number;
    // (16-3) % 23 = 13, (18-3) % 23 = 15
    EXPECT_EQ(e.get_share(), Number<TypeParam>(13));
    EXPECT_EQ(e.get_mac_share(), Number<TypeParam>(15));
    // operator-= with T number
    e -= number;
    EXPECT_EQ(e.get_share(), Number<TypeParam>(10));
    EXPECT_EQ(e.get_mac_share(), Number<TypeParam>(12));

    // operator-=
    ArithmeticSharing<TypeParam> f = b;
    f -= a;
    EXPECT_EQ(f.get_share(), Number<TypeParam>(10));
    EXPECT_EQ(f.get_mac_share(), Number<TypeParam>(8));
}

TYPED_TEST(ArithmeticSharingTest, MultiplicationOperators) {
    Number<TypeParam> modulus(31);
    ArithmeticSharing<TypeParam> a(modulus, Number<TypeParam>(7), Number<TypeParam>(5));

    // operator*(U number)
    auto b = a * 3;
    // (7*3)%31=21, (5*3)%31=15
    EXPECT_EQ(b.get_share(), Number<TypeParam>(21));
    EXPECT_EQ(b.get_mac_share(), Number<TypeParam>(15));

    // operator*=(U number)
    ArithmeticSharing<TypeParam> c = a;
    c *= 4;
    // (7*4)%31=28, (5*4)%31=20
    EXPECT_EQ(c.get_share(), Number<TypeParam>(28));
    EXPECT_EQ(c.get_mac_share(), Number<TypeParam>(20));

    // operator*(T number and ArithmeticSharing)
    Number<TypeParam> number(2);
    auto d = number * a;
    // (2*7)%31=14, (2*5)%31=10
    EXPECT_EQ(d.get_share(), Number<TypeParam>(14));
    EXPECT_EQ(d.get_mac_share(), Number<TypeParam>(10));
    // operator*(ArithmeticSharing and T number)
    d = a * number;
    // (7*2)%31=14, (5*2)%31=10
    EXPECT_EQ(d.get_share(), Number<TypeParam>(14));
    EXPECT_EQ(d.get_mac_share(), Number<TypeParam>(10));
    // operator*=(T number)
    ArithmeticSharing<TypeParam> e = a;
    e *= number;
    // (7*2)%31=14, (5*2)%31=10
    EXPECT_EQ(e.get_share(), Number<TypeParam>(14));
    EXPECT_EQ(e.get_mac_share(), Number<TypeParam>(10));
}

TYPED_TEST(ArithmeticSharingTest, LeftShiftOperator) {
    Number<TypeParam> modulus(17);
    ArithmeticSharing<TypeParam> a(modulus, Number<TypeParam>(3), Number<TypeParam>(5));

    // Left shift by 2: (3 << 2)%17=12, (5 << 2)%17=3
    auto b = a << 2;
    EXPECT_EQ(b.get_share(), Number<TypeParam>((3 << 2) % 17));
    EXPECT_EQ(b.get_mac_share(), Number<TypeParam>((5 << 2) % 17));
}

TYPED_TEST(ArithmeticSharingTest, GenerateArithmeticShares) {
    Number<TypeParam> modulus(101);
    Number<TypeParam> number(50);
    Number<TypeParam> key(7);
    int count = 5;
    Number<TypeParam> mac((number * key) % modulus);

    auto shares = ArithmeticSharing<TypeParam>::generate_random_shares(modulus, number, key, count);

    ASSERT_EQ(shares.size(), size_t(count));
    // Sum of shares modulo modulus equals number
    Number<TypeParam> sum_share(0);
    Number<TypeParam> sum_mac(0);
    for (const auto& share : shares) {
        EXPECT_EQ(share.get_modulus(), modulus);
        // EXPECT_EQ(share.get_mac_share(), key);
        // sum = (sum + share.get_share()) % modulus;
        sum_mac = (sum_mac + share.get_mac_share()) % modulus;
        sum_share = (sum_share + share.get_share()) % modulus;
    }
    EXPECT_EQ(sum_share, number);
    EXPECT_EQ(sum_mac, mac);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

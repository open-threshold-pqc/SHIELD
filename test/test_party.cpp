#include <gtest/gtest.h>
#include <thread>
#include "party.h"

using namespace otpqc::math;

void run_party1(otpqc::MPCParty<> &party, const Number<> &input_number) {
    party.setup_communication();
    auto io = party.get_io();
    auto num_bytes = input_number.linearized();
    int byte_count = static_cast<int>(num_bytes.size());
    io->send_data(2, &byte_count, sizeof(int));
    io->send_data(2, num_bytes.data(), byte_count);
    io->flush();
}

void run_party2(otpqc::MPCParty<> &party, Number<> &out_number) {
    party.setup_communication();
    auto io = party.get_io();
    int byte_count{};
    io->recv_data(1, &byte_count, sizeof(int));
    uint8_t recv_buffer[500];
    io->recv_data(1, recv_buffer, byte_count);
    std::vector<uint8_t> recv_bytes(recv_buffer, recv_buffer + byte_count);
    out_number = Number<>(recv_bytes);
}


void gmp_run_party1(otpqc::MPCParty<> &party, const Number<mpz_class> &input_number) {
    party.setup_communication();
    auto io = party.get_io();
    auto num_bytes = input_number.linearized();
    int byte_count = static_cast<int>(num_bytes.size());
    io->send_data(2, &byte_count, sizeof(int));
    io->send_data(2, num_bytes.data(), byte_count);
    io->flush();
}

void gmp_run_party2(otpqc::MPCParty<> &party, Number<mpz_class> &out_number) {
    party.setup_communication();
    auto io = party.get_io();
    int byte_count{};
    io->recv_data(1, &byte_count, sizeof(int));
    uint8_t recv_buffer[500];
    io->recv_data(1, recv_buffer, byte_count);
    std::vector<uint8_t> recv_bytes(recv_buffer, recv_buffer + byte_count);
    out_number = Number<mpz_class>(recv_bytes);
}


TEST(MPCNetworkTest, NumberTransferBetweenParties) {
    otpqc::MPCParty party1{1, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};
    otpqc::MPCParty party2{2, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};

    Number<> input_number{123456789};
    Number<> output_number{};

    std::thread thread1(run_party1, std::ref(party1), std::ref(input_number));
    std::thread thread2(run_party2, std::ref(party2), std::ref(output_number));
    thread1.join();
    thread2.join();

    EXPECT_EQ(output_number, input_number);
}

TEST(MPCNetworkTest, GMPNumberTransferBetweenParties) {
    otpqc::MPCParty party1{1, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};
    otpqc::MPCParty party2{2, "127.0.0.1", 12345, otpqc::mpc::MPCContext<>{}};

    Number<mpz_class> input_number{"712638612783618723687126127836233"};
    Number<mpz_class> output_number{};

    std::thread thread1(gmp_run_party1, std::ref(party1), std::ref(input_number));
    std::thread thread2(gmp_run_party2, std::ref(party2), std::ref(output_number));
    thread1.join();
    thread2.join();

    EXPECT_EQ(output_number, input_number);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

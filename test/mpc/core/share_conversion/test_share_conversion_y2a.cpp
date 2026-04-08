#include "mpc/protocols/share_conversion/share_conversion.h"
#include "mpc/dabit/dabit.h"
#include <chrono>

using namespace otpqc::mpc::sharing;
using namespace otpqc::math;

void run_party(otpqc::MPCParty<long> &party,
               BooleanSharing<long> &input_share,
               ArithmeticSharing<long> &output_share) {
    party.setup_communication();
    output_share = otpqc::mpc::protocols::ShareConversion<long>::y2a(party, input_share);
}

#define ITER 1
int main() {

    for (int i=0;i < ITER ; i++) {
        Number<long> modulus{8380417};
        Number<long> key{4295};
        Number<long> secret{1234};
        int bit_length{23};
        int number_of_parties{2};

        otpqc::MPCParty party_1(1, "127.0.0.1", 12346, otpqc::mpc::MPCContext<long>{});
        otpqc::MPCParty party_2(2, "127.0.0.1", 12346, otpqc::mpc::MPCContext<long>{});

        /* Dealer generates dabits (1 here) */
        const otpqc::mpc::Dabit dabit(modulus, key, bit_length, number_of_parties);
        auto dabit_shares = dabit.get_dabit_shares();

        auto boolean_shares = BooleanSharing<long>::generate_random_shares(
            secret, bit_length, number_of_parties);

        /* Load the dabits for each party */
        party_1.get_mpc_context().add_dabit(dabit_shares[0]);
        party_2.get_mpc_context().add_dabit(dabit_shares[1]);

        // Set MAC keys 
        auto global_mac_key_shares = ArithmeticSharing<long>::generate_random_global_key_shares(modulus, key, number_of_parties);
        party_1.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[0]);
        party_2.get_mpc_context().set_global_mac_key_share(global_mac_key_shares[1]);

        /* Run the clients */
        ArithmeticSharing<long> output_share_1{modulus, Number<long>{3}, Number<long>{1}};
        ArithmeticSharing<long> output_share_2{modulus, Number<long>{3}, Number<long>{2}};
        std::thread thread_party_1(run_party, std::ref(party_1), std::ref(boolean_shares[0]), std::ref(output_share_1));
        std::thread thread_party_2(run_party, std::ref(party_2), std::ref(boolean_shares[1]), std::ref(output_share_2));


        thread_party_1.join();
        thread_party_2.join();

        auto final_result = output_share_1 + output_share_2;
        if (final_result.get_share() != secret) {
            std::cerr << "Test failed: Expected " << secret.get_value() << ", got " << final_result.get_share().get_value() << std::endl;
        } else {
            std::cout << "Test passed: Secret is " << final_result.get_share().get_value() << std::endl;
            if (final_result.get_mac_share() != (key * secret) % modulus) {
                std::cerr << "Test failed: MAC share mismatch" << std::endl;
                std::cout << "Expected MAC share: " << (key * secret) % modulus << std::endl;
                std::cout << "Got MAC share: " << final_result.get_mac_share().get_value() << std::endl;
            } else {
                std::cout << "MAC share is correct: " << final_result.get_mac_share().get_value() << std::endl;
            }
        }
    }
}

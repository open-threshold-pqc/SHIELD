#ifndef SHARE_CONVERSION_H
#define SHARE_CONVERSION_H

#include "mpc/protocols/gc/circuit_exec.h"
#include "mpc/sharing/arithmetic_sharing.h"
#include "mpc/sharing/boolean_sharing.h"
#include "math/number.h"
#include "party.h"


/**
* \brief Contains various MPC protocols such as Garbled Circuit, Share Conversion, Share Multiplication
*/
namespace otpqc::mpc::protocols {
    using GC_FUNCTION_CODE = otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

    /**
     * \brief Share conversion class implementing conversions between various shares (e.g., Arithmetic and Yao)
     *
     * \subsection Tutorial Tutorial
     * Following tutorial shows how to perform share conversion in QST.
     *
     * \subsubsection a2y Arithmetic to Yao (A2Y)
     * \include tutorial_share_conversion_a2y.cpp
     *
     * \subsubsection y2a Yao to Arithmetic (Y2A)
     * \include tutorial_share_conversion_y2a.cpp
     */

    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class ShareConversion {

    public:
        /**
         * \brief This function converts Arithmetic sharing to Yao sharing (A2Y)
         * \param party MPC Party
         * \param share Arithmetic share to be converted
         * \return Boolean share
         */
        static sharing::BooleanSharing<T> a2y(MPCParty<T> &party, const sharing::ArithmeticSharing<T> &share) {
            /* Take the next daBit to use for the conversion */
            const auto [dabit_a_shares, dabit_b_shares, dabit_bit_length, dabit_modulus] = party.get_mpc_context().get_next_dabit();

            /*
             *
             * Locally compute r_i = Sigma (2^i * [r_i]_A)
             *                      then
             * Locally compute x_i - r_i and send to party 1 to construct the value x-r
             *
             */
            /* Create a zeroed share (temp share with zero share/mac) for r_i and add all the arithmetic shares of it as above */
            sharing::ArithmeticSharing<T> r_share{share.get_modulus(), otpqc::math::Number<T>{}, otpqc::math::Number<T>{}};

            for (int i = 0; i < dabit_bit_length; i++)
                r_share += dabit_a_shares[i] << i;

            /* Compute x_i - r_i */
            auto x_min_r_share{share - r_share};

            std::vector<bool> addition_circuit_input_1(dabit_bit_length);
            std::vector<bool> addition_circuit_input_2(dabit_bit_length);

            auto io = party.get_io();

            if (party.get_id() != 1) {
                /* Sending [[ x - r ]] shares to party 1*/
                auto share_byte_array = x_min_r_share.get_share().linearized();
                const int share_byte_len{static_cast<int>(share_byte_array.size())};

                io->send_data(1, &share_byte_len, sizeof(int)); //todo optimization of sending 4 bytes for length?
                io->send_data(1, share_byte_array.data(), share_byte_len);
                io->flush();

                int byte_length;
                std::vector<uint8_t> x_min_byte_array;
                io->recv_data(1, &byte_length, sizeof(int));
                x_min_byte_array.resize(byte_length);
                io->recv_data(1, x_min_byte_array.data(), byte_length);
                otpqc::math::Number<T> x_min_r_value{x_min_byte_array};

                party.get_mpc_context().add_arithmetic_partial_opening_value_mac(x_min_r_value, x_min_r_share.get_mac_share());

                /* Each party (!=1) participates in the addition circuit with inputs (0, [r_i]_b) */
                for (int i = 0; i < dabit_bit_length; i++)
                    addition_circuit_input_1[i] = false;

                /* daBit is already little endian */
                for (int i = 0; i < dabit_bit_length; i++)
                    addition_circuit_input_2[i] = (dabit_b_shares[dabit_bit_length - 1 - i].get_share() == 1);
            } else {
                /* Create an accumulator to add shares received from other parties */
                sharing::ArithmeticSharing x_min_r_value = x_min_r_share;

                std::vector<uint8_t> byte_buffer(100); //todo optimization

                /* Receiving [[x-r]] share from all other parties to construct the x-r */
                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; i++) {
                    int share_byte_len;
                    io->recv_data(i, &share_byte_len, sizeof(int));
                    io->recv_data(i, byte_buffer.data(), share_byte_len);
                    byte_buffer.resize(share_byte_len);

                    x_min_r_value += sharing::ArithmeticSharing{
                        share.get_modulus(), otpqc::math::Number<T>{byte_buffer}, otpqc::math::Number<T>{}
                    };
                }

                /* Adding the partial share/mac for later batch checking */
                party.get_mpc_context().add_arithmetic_partial_opening_value_mac(x_min_r_value.get_share(), x_min_r_share.get_mac_share());

                /* Sending x-r value to other parties */
                auto xminr_bytes = x_min_r_value.get_share().linearized();
                int xminr_byte_len {static_cast<int>(xminr_bytes.size())};
                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; i++) {
                    io->send_data(i, &xminr_byte_len, sizeof(int));
                    io->send_data(i, xminr_bytes.data(), xminr_byte_len);
                    io->flush();
                }

                /* Party 1 participates in the addition circuit with inputs (x_r, [r_i]_b) */
                addition_circuit_input_1 = x_min_r_value.get_share().bits_be_ze(dabit_bit_length);

                for (int i = 0; i < dabit_bit_length; i++)
                    addition_circuit_input_2[i] = (dabit_b_shares[dabit_bit_length - 1 - i].get_share() == 1);
            }

            /* Run MPC Circuit addition */
            std::vector<bool> circuit_output; //todo Write this better
            if (dabit_bit_length == 23) {
                auto circuit = party.get_mpc_context().get_registered_circuit(
                    GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_8380417_23_23_23);
                circuit_output = gc::circuit::run(circuit, &addition_circuit_input_1, &addition_circuit_input_2);
            } else if (dabit_bit_length == 32) {
                auto circuit = party.get_mpc_context().get_registered_circuit(
                    GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_4294967295_32_32_32);
                circuit_output = gc::circuit::run(circuit, &addition_circuit_input_1, &addition_circuit_input_2);
            } else if (dabit_bit_length == 256) {
                auto circuit = party.get_mpc_context().get_registered_circuit(
                    GC_FUNCTION_CODE::UNSIGNED_ADD_MOD_PRIME_256_256_256);
                circuit_output = gc::circuit::run(circuit, &addition_circuit_input_1, &addition_circuit_input_2);
            } else
                throw std::runtime_error("[MPC:A2Y] Unsupported daBit length");

            sharing::BooleanSharing<T> result{circuit_output};

            return result;
        }


        /**
         * \brief This function converts Yao sharing to Arithmetic sharing (Y2A)
         * \param party MPC Party
         * \param share Boolean (Yao) share to be converted
         * \return Arithmetic share
         */
        static sharing::ArithmeticSharing<T> y2a(MPCParty<T> &party, const sharing::BooleanSharing<T> &share) {
            /* Take the next daBit to use for the conversion */
            auto [dabit_a_shares, dabit_b_shares, dabit_bit_length, dabit_modulus] = party.get_mpc_context().get_next_dabit();


            /* Convert the boolean share into a vector of bits (bools) stored as little endian */
            auto input_share_bits = share.bits_le();

            /*
             *
             * We first compute v_i = x_i XOR r_i
             *              then
             * Open the value of v_i to all other parties (Send to party 1 to reconstruct v and party 1 will
             * send v to all other parties)
             *
             */
            /* Computing v shares */
            std::vector<bool> v_shares(dabit_bit_length);
            for (int i = 0; i < dabit_bit_length; i++)
                v_shares[i] = input_share_bits[i] ^ (dabit_b_shares[i].get_share() == 1);


            /* Open the value v_i to all the parties */
            auto io = party.get_io();
            std::vector<bool> v_value = v_shares;

            if (party.get_id() == 1) {
                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; i++) {
                    char v_shares_bits_string[512]; //todo Optimize this with bits to bytes and bytes to bits
                    int v_shares_bits_string_length;

                    io->recv_data(i, &v_shares_bits_string_length, sizeof(int));
                    io->recv_data(i, v_shares_bits_string, v_shares_bits_string_length);

                    std::vector<bool> received_v_shares_bits{};
                    for (int j = 0; j < v_shares_bits_string_length; j++)
                        received_v_shares_bits.push_back(v_shares_bits_string[j] == '1');

                    /* Party 1 XORs the received bits with its own share to construct the v */
                    for (int j = 0; j < dabit_bit_length; j++)
                        v_value[j] = v_value[j] ^ received_v_shares_bits[j];
                }
            } else {
                /* Convert the vector of bools to string */
                std::string v_shares_bits_string{};
                for (int j = 0; j < dabit_bit_length; j++)
                    v_shares_bits_string += v_shares[j] ? "1" : "0";

                const int v_shares_bits_string_length = v_shares_bits_string.length();
                io->send_data(1, &v_shares_bits_string_length, sizeof(int));
                io->send_data(1, v_shares_bits_string.c_str(), v_shares_bits_string_length);
                io->flush();
            }

            /* Send the constructed v to all other parties */
            if (party.get_id() != 1) {
                char v_bits_string[512];
                int v_bits_string_length;
                io->recv_data(1, &v_bits_string_length, sizeof(int));
                io->recv_data(1, v_bits_string, v_bits_string_length);

                for (int i = 0; i < v_bits_string_length; i++)
                    v_value[i] = (v_bits_string[i] == '1');
            } else {
                /* Convert vector of bools to string */
                std::string v_bits_string{};
                for (int j = 0; j < dabit_bit_length; j++)
                    v_bits_string += v_value[j] ? '1' : '0';

                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; i++) {
                    int v_bits_string_length = v_bits_string.length();
                    io->send_data(i, &v_bits_string_length, sizeof(int));
                    io->send_data(i, v_bits_string.c_str(), v_bits_string_length);
                    io->flush();
                }
            }

            /* Compute x_i = v_i + r_i - 2 * vi * r_i locally */
            std::vector<sharing::ArithmeticSharing<T>> input_new_arithmetic_shares;
            input_new_arithmetic_shares.reserve(dabit_bit_length);

            auto global_mac_key_share = party.get_mpc_context().get_global_mac_key_share();

            for (int i = 0; i < dabit_bit_length; i++) {
                int vi{v_value[i]};

                if (party.get_id() == 1) {
                    input_new_arithmetic_shares.emplace_back(
                        (vi + dabit_a_shares[i] -= vi * dabit_a_shares[i]) -= vi * dabit_a_shares[i]);
                    // process the mac of newly added share
                    auto &last_share = input_new_arithmetic_shares.back();
                    last_share.set_mac_share((last_share.get_mac_share() - otpqc::math::Number<T>(vi) + otpqc::math::Number<T>(vi) * global_mac_key_share) % dabit_modulus);
                } 
                else {
                    input_new_arithmetic_shares.emplace_back(
                        (dabit_a_shares[i] -= vi * dabit_a_shares[i]) -= vi * dabit_a_shares[i]);
                    // process the mac of newly added share
                    auto &last_share = input_new_arithmetic_shares.back();
                    last_share.set_mac_share((last_share.get_mac_share() + otpqc::math::Number<T>(vi) * global_mac_key_share) % dabit_modulus);
                }
            }

            /* Compute the final share as [x] = Sigma (2^i * [x_i]) */
            sharing::ArithmeticSharing final_share{dabit_modulus, otpqc::math::Number<T> {}, otpqc::math::Number<T> {}};
            for (int i = 0; i < dabit_bit_length; i++)
                final_share += input_new_arithmetic_shares[i] << i;

            return final_share;
    }
};

}

#endif

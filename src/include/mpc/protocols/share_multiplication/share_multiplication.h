#ifndef SHARE_MULTIPLICATION_H
#define SHARE_MULTIPLICATION_H

#include "party.h"
#include "mpc/sharing/arithmetic_sharing.h"

/**
* \brief Contains various MPC protocols such as Garbled Circuit, Share Conversion, Share Multiplication
*/
namespace otpqc::mpc::protocols {
    /**
     * @brief Share multiplication class (Arithmetic x Arithmetic)
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class ShareMultiplication {
    public:
        /**
         * \brief This function multiplies two given arithmetic shares (multiplication is non-linear operation)
         * \param party MPC Party
         * \param share1 Arithmetic share 1 to be multiplied
         * \param share2 Arithmetic share 2 to be multiplied
         * \return New arithmetic share which is multiplication of the two given shares
         */
        static sharing::ArithmeticSharing<T> multiply_arithmetic_shares(otpqc::MPCParty<T> &party,
                                                             const sharing::ArithmeticSharing<T> &share1,
                                                             const sharing::ArithmeticSharing<T> &share2) {
            auto beaver_triple = party.get_mpc_context().get_next_beaver_triple();
            otpqc::math::Number<T> modulus(share1.get_modulus());
            otpqc::math::Number<T> d, e, d_mac, e_mac;
            d = (share1.get_share() - beaver_triple.a_share) % modulus;
            e = (share2.get_share() - beaver_triple.b_share) % modulus;
            d_mac = (share1.get_mac_share() - beaver_triple.a_mac_share) % modulus;
            e_mac = (share2.get_mac_share() - beaver_triple.b_mac_share) % modulus;
            // Broadcast d and e to all parties
            auto io = party.get_io();

            if (party.get_id() != 1) {
                auto d_bytes = d.linearized();
                auto e_bytes = e.linearized();
                int d_length = d_bytes.size();
                int e_length = e_bytes.size();
                io->send_data(1, &d_length, sizeof(int));
                io->send_data(1, d_bytes.data(), d_length);
                io->send_data(1, &e_length, sizeof(int));
                io->send_data(1, e_bytes.data(), e_length);
                io->flush();
            }
            std::vector<uint8_t> recv_buffer;
            int length = 0;
            if (party.get_id() == 1) {
                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; ++i) {
                    io->recv_data(i, &length, sizeof(int));
                    recv_buffer.resize(length);
                    io->recv_data(i, recv_buffer.data(), length);
                    d = (d + otpqc::math::Number<T>(recv_buffer)) % modulus;
                    io->recv_data(i, &length, sizeof(int));
                    recv_buffer.resize(length);
                    io->recv_data(i, recv_buffer.data(), length);
                    e = (e + otpqc::math::Number<T>(recv_buffer)) % modulus;
                }
                // Broadcast d and e to all parties
                auto d_bytes = d.linearized();
                auto e_bytes = e.linearized();
                int d_length = d_bytes.size();
                int e_length = e_bytes.size();
                for (int i = 2; i <= QST_NUM_OF_MPC_PARTIES; ++i) {
                    io->send_data(i, &d_length, sizeof(int));
                    io->send_data(i, d_bytes.data(), d_length);
                    io->send_data(i, &e_length, sizeof(int));
                    io->send_data(i, e_bytes.data(), e_length);
                    io->flush();
                }
            }

            if (party.get_id() != 1) {
                io->recv_data(1, &length, sizeof(int));
                recv_buffer.resize(length);
                io->recv_data(1, recv_buffer.data(), length);
                d = otpqc::math::Number<T>(recv_buffer);
                io->recv_data(1, &length, sizeof(int));
                recv_buffer.resize(length);
                io->recv_data(1, recv_buffer.data(), length);
                e = otpqc::math::Number<T>(recv_buffer);
            }
            // add d and e with mac shares of d_mac and e_mac to the party
            party.get_mpc_context().add_arithmetic_partial_opening_value_mac(d, d_mac);
            party.get_mpc_context().add_arithmetic_partial_opening_value_mac(e, e_mac);
            // Compute z = c + a * e + b * d + d * e
            otpqc::math::Number<T> z = beaver_triple.c_share;
            otpqc::math::Number<T> a_e_mul = (beaver_triple.a_share * e) % modulus;
            otpqc::math::Number<T> b_d_mul = (beaver_triple.b_share * d) % modulus;
            z = (z + a_e_mul + b_d_mul) % modulus;
             otpqc::math::Number<T> d_e_mul = (d * e) % modulus;
            if (party.get_id() == 1) {
                z = (z + d_e_mul) % modulus;
            }
            otpqc::math::Number<T> z_mac = beaver_triple.c_mac_share;
            otpqc::math::Number<T> a_e_mul_mac = (beaver_triple.a_mac_share * e) % modulus;
            otpqc::math::Number<T> b_d_mul_mac = (beaver_triple.b_mac_share * d) % modulus;
            otpqc::math::Number<T> d_e_mul_mac = (d_e_mul * party.get_mpc_context().get_global_mac_key_share()) % modulus;
            z_mac = (z_mac + a_e_mul_mac + b_d_mul_mac + d_e_mul_mac) % modulus;

            return otpqc::mpc::sharing::ArithmeticSharing<T>(modulus, z, z_mac);
        }

                                                             
    };
}

#endif

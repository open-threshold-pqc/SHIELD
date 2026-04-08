#ifndef BATCH_CHECKING_H
#define BATCH_CHECKING_H

#include "party.h"
/**
 * \brief Batch checking class for MPC protocols
 */
namespace otpqc::mpc::protocols {
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class BatchChecking {
    public:
        /**
         * \brief This function performs batch checking of arithmetic shares
         * \param party MPC Party
         * \return True if the batch checking is successful, false otherwise
         */
        static bool perform_batch_checking(otpqc::MPCParty<T> &party, const otpqc::math::Number<T> &modulus) {
            std::vector<std::pair<otpqc::math::Number<T>, otpqc::math::Number<T> > > partial_opening_values_mac =
                    party.get_mpc_context().get_arithmetic_partial_opening_values_mac();

            std::vector<otpqc::math::Number<T> > agreed_random_values =
                    party.get_mpc_context().get_agreed_random_values(partial_opening_values_mac.size());

            assert(partial_opening_values_mac.size() == agreed_random_values.size()
                && "[BatchChecking:] Partial opening values and agreed random values must have the same size");
            otpqc::math::Number<T> sigma{0};

            auto global_mac_key_share = party.get_mpc_context().get_global_mac_key_share();

            for (int i = 0; i < partial_opening_values_mac.size(); ++i) {
                auto &pair = partial_opening_values_mac[i];
                auto &value = partial_opening_values_mac[i].first;
                auto &mac = partial_opening_values_mac[i].second;

                sigma = (sigma + (agreed_random_values[i] *
                                  (mac - (value * party.get_mpc_context().get_global_mac_key_share()) % modulus)) %
                         modulus) % modulus;
            }

            int length;
            auto io = party.get_io();
            for (int i = 1; i <= QST_NUM_OF_MPC_PARTIES; ++i) {
                if (i == party.get_id()) continue;
                auto sigma_bytes = sigma.linearized();
                length = sigma_bytes.size();
                io->send_data(i, &length, sizeof(int));
                io->send_data(i, sigma_bytes.data(), length);
                io->flush();
            }
            std::vector<uint8_t> recv_buffer;
            for (int i = 1; i <= QST_NUM_OF_MPC_PARTIES; ++i) {
                if (i == party.get_id()) continue;
                io->recv_data(i, &length, sizeof(int));
                recv_buffer.resize(length);
                io->recv_data(i, recv_buffer.data(), length);
                sigma = (sigma + otpqc::math::Number<T>(recv_buffer)) % modulus;
            }
            return sigma == 0;
        }
    };
}


#endif

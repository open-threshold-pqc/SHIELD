#ifndef PARTY_H
#define PARTY_H

#include "mpc/protocols/gc/emp-mpc/netmp.h"
#include "mpc/mpc_context.h"
#include <string>


/**
 * Contains various fundamental classes
 */
namespace otpqc {
    /**
     * \brief MPC user class
     *
     * \subsection Tutorial Tutorial
     * Following tutorial shows how to work with the user (party) in QST.
     * \include tutorial_party.cpp
     */
    class Party {
    public:
        Party(int id, const std::string &ip, int port);

        ~Party();

        [[nodiscard]] int get_id() const;

        [[nodiscard]] std::string get_ip() const;

        [[nodiscard]] int get_port() const;

        [[nodiscard]] NetIOMP<QST_NUM_OF_MPC_PARTIES> *get_io() const;

        /**
         * \brief Starts the communication channel for NetIOMP
         */
        void setup_communication();

    private:

        int m_id;

        std::string m_ip;

        int m_port;

        NetIOMP<QST_NUM_OF_MPC_PARTIES> *m_net_io{nullptr};
    };

    /**
     * \brief Specialized MPC party derived from the generic Party class
     * @tparam T Underlying data type that MPC party will be using for constructing all MPC data over
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class MPCParty: public Party{
    public:
        MPCParty(const int id, const std::string &ip, const int port, mpc::MPCContext<T> mpc_core):
            Party(id, ip, port), m_mpc_ctx(mpc_core){
        }

        mpc::MPCContext<T> &get_mpc_context() {
            return m_mpc_ctx;
        }

        [[nodiscard]] ThreadPool& get_thread_pool() {
            return m_pool;
        }
    private:
        mpc::MPCContext<T> m_mpc_ctx;
        ThreadPool m_pool {2 * (QST_NUM_OF_MPC_PARTIES - 1) + 2};
    };
}


#endif
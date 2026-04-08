#include "party.h"


namespace otpqc {
    Party::Party(const int id, const std::string &ip, const int port): m_id(id), m_ip(ip), m_port(port) {
    }

    void Party::setup_communication() {
        m_net_io = static_cast<NetIOMP<QST_NUM_OF_MPC_PARTIES> *>(::operator new[](
            2 * sizeof(NetIOMP<QST_NUM_OF_MPC_PARTIES>)));

        new(&m_net_io[0]) NetIOMP<QST_NUM_OF_MPC_PARTIES>(m_id, m_port);
#ifdef LOCALHOST
        new(&m_net_io[1]) NetIOMP<QST_NUM_OF_MPC_PARTIES>(
            m_id, m_port + 2 * (QST_NUM_OF_MPC_PARTIES + 1) * (QST_NUM_OF_MPC_PARTIES + 1) + 1);
#else
        new (&m_net_io[1]) NetIOMP<QST_NUM_OF_MPC_PARTIES>(m_id, m_port+2*(QST_NUM_OF_MPC_PARTIES+1));
#endif
    }

    Party::~Party() {
        if (m_net_io != nullptr) {
            m_net_io[0].~NetIOMP<QST_NUM_OF_MPC_PARTIES>();
            m_net_io[1].~NetIOMP<QST_NUM_OF_MPC_PARTIES>();
            ::operator delete[](m_net_io);
        }
    }


    int Party::get_id() const {
        return m_id;
    }

    std::string Party::get_ip() const {
        return m_ip;
    }

    int Party::get_port() const {
        return m_port;
    }

    NetIOMP<QST_NUM_OF_MPC_PARTIES> *Party::get_io() const {
        return m_net_io;
    }
}

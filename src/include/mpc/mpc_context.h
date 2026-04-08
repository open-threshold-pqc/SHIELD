#ifndef MPC_H
#define MPC_H

#include "mpc/protocols/gc/function_codes.h"
#include "mpc/protocols/gc/function_context.h"
#include "mpc/beaver_triple/beaver_triple.h"
#include "mpc/dabit/dabit.h"
#include "math/number.h"
#include <deque>
#include <map>


namespace otpqc::mpc {
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    class MPCContext {
        using GCFunctionContext = otpqc::mpc::protocols::gc::circuit::GCFunctionContext;
        using GCFunctionCode = otpqc::mpc::protocols::gc::circuit::GC_FUNCTION_CODE;

    public:
        MPCContext() = default;


        static std::vector<GCFunctionContext> create_circuit(GCFunctionCode operation, const int count,
                                                             NetIOMP<QST_NUM_OF_MPC_PARTIES> *net_io,
                                                             ThreadPool *thread_pool,
                                                             const int mpc_party_id) {
            std::vector<GCFunctionContext> circuits;
            circuits.reserve(count);

            std::string circuit_file_name{};
            if (operation == GCFunctionCode::SHA256_512_0_256)
                circuit_file_name = "SHA2/sha256_512_0_256.txt";
            else if (operation == GCFunctionCode::UNSIGNED_ADD_MOD_8380417_23_23_23)
                circuit_file_name = "Arithmetic/23-bit/unsigned_adder_mod_8380417_23_23_23.txt";
            else if (operation == GCFunctionCode::UNSIGNED_ADD_MOD_4294967295_32_32_32)
                circuit_file_name = "Arithmetic/32-bit/unsigned_adder_mod_4294967295_32_32_32.txt";
            else if (operation == GCFunctionCode::UNSIGNED_ADD_MOD_PRIME_256_256_256)
                circuit_file_name = "Arithmetic/256-bit/unsigned_adder_mod_prime_256_256_256.txt";
            else if (operation == GCFunctionCode::UNSIGNED_LESS_THAN_256_256_1)
                circuit_file_name = "Comparison/unsigned_less_than_256_256_1.txt";
            else if (operation == GCFunctionCode::KECCAK_F_PERMUTATION_1600_1600)
                circuit_file_name = "Keccak/keccak_f_permutation_1600_1600.txt";
            else if (operation == GCFunctionCode::KECCAK_F_PERMUTATION_1600_1600_CHAIN_5)
                circuit_file_name = "Keccak/keccak_f_permutation_1600_1600_chain_5.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_DECOMPOSE)
                circuit_file_name = "Dilithium/Decompose/dilithium2_decompose.txt";
            else if (operation == GCFunctionCode::DILITHIUM35_DECOMPOSE)
                circuit_file_name = "Dilithium/Decompose/dilithium35_decompose.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_SAMPLE_IN_BALL)
                circuit_file_name = "Dilithium/PollyChallenge/dilithium2_sample_in_ball.txt";
            else if (operation == GCFunctionCode::DILITHIUM3_SAMPLE_IN_BALL)
                circuit_file_name = "Dilithium/PollyChallenge/dilithium3_sample_in_ball.txt";
            else if (operation == GCFunctionCode::DILITHIUM5_SAMPLE_IN_BALL)
                circuit_file_name = "Dilithium/PollyChallenge/dilithium5_sample_in_ball.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_CHECK_NORM_GAMMA1_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium2_check_norm_gamma1_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM3_CHECK_NORM_GAMMA1_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium3_check_norm_gamma1_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM5_CHECK_NORM_GAMMA1_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium5_check_norm_gamma1_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_CHECK_NORM_GAMMA2_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium2_check_norm_gamma2_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM3_CHECK_NORM_GAMMA2_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium3_check_norm_gamma2_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM5_CHECK_NORM_GAMMA2_BETA)
                circuit_file_name = "Dilithium/CheckNorm/dilithium5_check_norm_gamma2_beta.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_CHECK_NORM_GAMMA2)
                circuit_file_name = "Dilithium/CheckNorm/dilithium2_check_norm_gamma2.txt";
            else if (operation == GCFunctionCode::DILITHIUM3_CHECK_NORM_GAMMA2)
                circuit_file_name = "Dilithium/CheckNorm/dilithium3_check_norm_gamma2.txt";
            else if (operation == GCFunctionCode::DILITHIUM5_CHECK_NORM_GAMMA2)
                circuit_file_name = "Dilithium/CheckNorm/dilithium5_check_norm_gamma2.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_MAKE_HINT)
                circuit_file_name = "Dilithium/MakeHint/dilithium2_make_hint.txt";
            else if (operation == GCFunctionCode::DILITHIUM35_MAKE_HINT)
                circuit_file_name = "Dilithium/MakeHint/dilithium35_make_hint.txt";
            else if (operation == GCFunctionCode::DILITHIUM2_HINT_ONE_COMPARISON)
                circuit_file_name = "Dilithium/HintOneComparison/dilithium2_hint_one_comparison.txt";
            else if (operation == GCFunctionCode::DILITHIUM3_HINT_ONE_COMPARISON)
                circuit_file_name = "Dilithium/HintOneComparison/dilithium3_hint_one_comparison.txt";
            else if (operation == GCFunctionCode::DILITHIUM5_HINT_ONE_COMPARISON)
                circuit_file_name = "Dilithium/HintOneComparison/dilithium5_hint_one_comparison.txt";
            else
                throw std::invalid_argument("[MPC::GC::create_circuit] Given operation is not supported");

            for (int i = 0; i < count; i++) {
                const auto circuit_bf = new BristolFormat(
                    (std::string(GC_BRISTOL_CIRCUITS_BASE_PATH) + "/" + circuit_file_name).c_str());


                circuits.emplace_back(operation, new CMPC{net_io, thread_pool, mpc_party_id, circuit_bf},
                                      circuit_bf->n1,
                                      circuit_bf->n2, circuit_bf->n3);
            }
            return circuits;
        }

        void register_circuit(const GCFunctionContext &function) {
            m_functions[function.operation].emplace_back(function);
        }

        GCFunctionContext get_registered_circuit(const GCFunctionCode code) {
            const auto it = m_functions.find(code);
            if (it == m_functions.end() || it->second.empty()) {
                throw std::runtime_error(
                    "[MPCCore.get_function] No available GCFunction for " + GC_FUNCTION_CODE_STRING(code));
            }

            const GCFunctionContext func = it->second.front();
            it->second.pop_front();
            return func;
        }

        void add_dabit(const DabitShares<T> &dabit) {
            m_dabits.emplace_back(dabit);
        }

        DabitShares<T> get_next_dabit() {
            if (m_dabits.empty())
                throw std::runtime_error("[MPCCore.daBit] No more daBits available");

            DabitShares<T> next = std::move(m_dabits.front());
            m_dabits.pop_front();
            return next;
        }

        void add_beaver_triple(const BeaverTripleShares<T> &triple) {
            m_beaver_triples.emplace_back(triple);
        }

        BeaverTripleShares<T> get_next_beaver_triple() {
            if (m_beaver_triples.empty())
                throw std::runtime_error("[MPCCore.beaver_triple] No more Beaver Triples available");

            BeaverTripleShares<T> next = std::move(m_beaver_triples.front());
            m_beaver_triples.pop_front();
            return next;
        }

        void set_global_mac_key_share(const otpqc::math::Number<T> &key_share) {
            m_global_mac_key_share = key_share;
        }

        otpqc::math::Number<T> get_global_mac_key_share() const {
            return m_global_mac_key_share;
        }

        void add_arithmetic_partial_opening_value_mac(const otpqc::math::Number<T> &value,
                                                      const otpqc::math::Number<T> &mac) {
            m_arithmetic_partial_opening_values_mac.emplace_back(value, mac);
        }

        std::vector<std::pair<otpqc::math::Number<T>, otpqc::math::Number<T> > >
        get_arithmetic_partial_opening_values_mac() const {
            return m_arithmetic_partial_opening_values_mac;
        }

        void add_agreed_random_value(const otpqc::math::Number<T> &value) {
            agreed_random_values.emplace_back(value);
        }

        /**
         * \brief Get the agreed random values
         * \param num_randomo_values Number of random values to retrieve
         * \return Vector of agreed random values
         */
        std::vector<otpqc::math::Number<T> > get_agreed_random_values(int num_randomo_values) {
            if (num_randomo_values > agreed_random_values.size()) {
                throw std::runtime_error(
                    "[MPCCore.get_agreed_random_values] Not enough agreed random values available");
            }
            auto begin = agreed_random_values.begin();
            std::vector<otpqc::math::Number<T> > result(begin, begin + num_randomo_values);
            agreed_random_values.erase(begin, begin + num_randomo_values);
            return result;
        }

    private:
        std::deque<DabitShares<T> > m_dabits{};
        std::deque<BeaverTripleShares<T> > m_beaver_triples{};
        otpqc::math::Number<T> m_global_mac_key_share{};
        std::vector<std::pair<otpqc::math::Number<T>, otpqc::math::Number<T> > > m_arithmetic_partial_opening_values_mac{};
        std::vector<otpqc::math::Number<T> > agreed_random_values{};

        std::map<GCFunctionCode, std::deque<GCFunctionContext> > m_functions{};
    };
}

#endif

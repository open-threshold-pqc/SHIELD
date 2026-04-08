#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include <tuple>

#include "ml_dsa/threshold/randombytes.h"
#include "ml_dsa/threshold/packing.h"
#include "ml_dsa/threshold/threshold_keygen.h"
#include "ml_dsa/threshold/threshold_sign.h"
#include "ml_dsa/threshold/threshold_preprocess.h"
#include "math/number.h"


inline constexpr const char *COLOR_GREEN = "\033[1;32m";
inline constexpr const char *COLOR_RED = "\033[1;31m";
inline constexpr const char *COLOR_YELLOW = "\033[1;33m";
inline constexpr const char *COLOR_RESET = "\033[0m";

int main() {
    using namespace otpqc::threshold_signatures::dilithium::poly;

    std::cout << COLOR_GREEN << "[Threshold Dilithium Test]" << COLOR_RESET << "\n";
    std::cout << " - Parties      : " << QST_NUM_OF_MPC_PARTIES << "\n";
    std::cout << " - Mode         : " << DILITHIUM_MODE << "\n";

    constexpr std::size_t CTX_LENGTH = 14;
    std::array<uint8_t, CTX_LENGTH> ctx{};
    snprintf(reinterpret_cast<char *>(ctx.data()), CTX_LENGTH, "test_dilithium");

    constexpr std::size_t MESSAGE_LENGTH{59};
    std::array<uint8_t, MESSAGE_LENGTH + DILITHIUM_CRYPTO_BYTES> message{};


    const otpqc::math::Number<> modulus{DILITHIUM_Q};
    const otpqc::math::Number<> global_mac_key{123456};

    /* Run keygen and get the unpacked shares of the private key and the actual public key */
    auto [rho,
                key_shares,
                tr, t0_shares,
                s1_shares,
                s2_shares,
                pk] =
            otpqc::threshold_signatures::dilithium::keygen(modulus, global_mac_key);

    /* Perform A = ExpandA(rho) and distribute shares of the matrix as matrix of PolyCoeffConstantShare */
    polyvecl mat[DILITHIUM_K];
    polyvec_matrix_expand(mat, rho.get());

    PolynomialMatrixShare<> A(DILITHIUM_K);
    for (int i = 0; i < DILITHIUM_K; ++i) {
        A[i].resize(DILITHIUM_L);
        for (int j = 0; j < DILITHIUM_L; ++j) {
            for (int coeff: mat[i].vec[j].coeffs) {
                A[i][j].emplace_back(utils::PolyyCoeff_int32_to_Number(coeff));
            }
        }
    }

    /* Creating QST_NUM_OF_MPC_PARTIES parties (threads) */
    std::vector<std::unique_ptr<otpqc::MPCParty<> > > parties;
    parties.reserve(QST_NUM_OF_MPC_PARTIES);
    for (int pid = 1; pid <= QST_NUM_OF_MPC_PARTIES; ++pid)
        parties.emplace_back(std::make_unique<otpqc::MPCParty<> >(pid, "127.0.0.1", 12346, otpqc::mpc::MPCContext<>()));


    /* Run the MPC processing (daBit generation, beaver triples generation, and Garbled Circuit preprocessing) */
    otpqc::threshold_signatures::dilithium::mpc_preprocess(parties, modulus, global_mac_key);


#if defined(DILITHIUM_RANDOMIZED_SIGNING)
    std::array<uint8_t, DILITHIUM_RNDBYTES> rnd_buffer;
    randombytes(rnd_buffer.data(), DILITHIUM_RNDBYTES);
    const auto rnd_shares{
        otpqc::threshold_signatures::utils::boolean_distribute_byte_array(rnd_buffer.data(), DILITHIUM_RNDBYTES,
                                                                        QST_NUM_OF_MPC_PARTIES)
    };
#else
    std::vector<std::unique_ptr<uint8_t[]> > rnd_shares;
    for (int i = 0; i < QST_NUM_OF_MPC_PARTIES; ++i) {
        auto buf = std::make_unique<uint8_t[]>(DILITHIUM_RNDBYTES);
        std::fill_n(buf.get(), DILITHIUM_RNDBYTES, 0);
        rnd_shares.push_back(std::move(buf));
    }
#endif

    std::vector<std::unique_ptr<uint8_t[]> > packed_signature_shares(QST_NUM_OF_MPC_PARTIES);
    std::vector<size_t> signature_len(QST_NUM_OF_MPC_PARTIES);
    for (auto &buf: packed_signature_shares) {
        buf = std::make_unique<uint8_t[]>(MESSAGE_LENGTH + DILITHIUM_CRYPTO_BYTES);
        std::fill_n(buf.get(), MESSAGE_LENGTH + DILITHIUM_CRYPTO_BYTES, 0);
    }
    std::vector<std::thread> threads;

    /*
    * Set nonce:
    *   Dilithium2 = 11
    *   Dilithium3 = 2
    *   Dilithium5 = 1
     */
    int nonce;

    if constexpr (QST_ML_DSA_MODE == 2)
        nonce = 11;
    else if constexpr (QST_ML_DSA_MODE == 3)
        nonce = 2;
    else
        nonce = 1;

    /*
    * Batch checking results
    */
   std::array<bool, QST_NUM_OF_MPC_PARTIES> res_batch_check{};
    for (int p = 0; p < QST_NUM_OF_MPC_PARTIES; ++p) {
        threads.emplace_back([&, p] {
            otpqc::threshold_signatures::dilithium::sign(
                *parties[p], packed_signature_shares[p].get(), &signature_len[p],
                message.data(), MESSAGE_LENGTH, ctx.data(), CTX_LENGTH,
                tr.get(), key_shares[p].get(), rnd_shares[p].get(),
                A, s1_shares[p], s2_shares[p], t0_shares[p], nonce, res_batch_check[p]);
        });
    }
    for (auto &t: threads) t.join();
    if (std::any_of(res_batch_check.begin(), res_batch_check.end(), [](bool res) { return !res; })) {
        std::cerr << COLOR_RED << "Batch check failed" << COLOR_RESET << "\n";
    }

    std::array<uint8_t, MESSAGE_LENGTH + DILITHIUM_CRYPTO_BYTES> reconstructed_message{};
    size_t reconstructed_message_len{};

    int ret = crypto_sign_open(reconstructed_message.data(), &reconstructed_message_len,
                               packed_signature_shares[0].get(), signature_len[0],
                               ctx.data(), CTX_LENGTH, pk.get());

    if (ret) {
        std::cerr << COLOR_RED << "Verification failed" << COLOR_RESET << "\n";
        return -1;
    }

    if (signature_len[0] != MESSAGE_LENGTH + DILITHIUM_CRYPTO_BYTES) {
        std::cerr << COLOR_RED << "Signed message lengths wrong" << COLOR_RESET << "\n";
        return -1;
    }

    if (reconstructed_message_len != MESSAGE_LENGTH) {
        std::cerr << COLOR_RED << "Message lengths wrong" << COLOR_RESET << "\n";
        return -1;
    }

    for (int i = 0; i < MESSAGE_LENGTH; ++i) {
        if (reconstructed_message[i] != message[i]) {
            fprintf(stderr, "Messages don't match\n");
            return -1;
        }
    }

    std::cout << COLOR_YELLOW << "\n[Dilithium Parameter Summary]\n" << COLOR_RESET;
    std::cout << "  " << COLOR_GREEN << "CRYPTO_PUBLICKEYBYTES" << COLOR_RESET << "  = " <<
            DILITHIUM_CRYPTO_PUBLICKEYBYTES << "\n";
    std::cout << "  " << COLOR_GREEN << "CRYPTO_SECRETKEYBYTES" << COLOR_RESET << "  = " <<
            DILITHIUM_CRYPTO_SECRETKEYBYTES << "\n";
    std::cout << "  " << COLOR_GREEN << "CRYPTO_BYTES" << COLOR_RESET << "           = " << DILITHIUM_CRYPTO_BYTES <<
            "\n";

    std::cout << COLOR_GREEN << "\nThreshold Dilithium test passed successfully!\n" << COLOR_RESET;
    return 0;
}

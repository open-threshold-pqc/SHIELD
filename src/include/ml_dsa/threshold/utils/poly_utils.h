#ifndef POLY_UTILS_H
#define POLY_UTILS_H

#include <vector>

#include "math/number.h"
#include "mpc/sharing/boolean_sharing.h"
#include "mpc/sharing/arithmetic_sharing.h"
#include "../reduce.h"
#include "../polyvec.h"
#include "ml_dsa/threshold/params.h"
#include "ml_dsa/threshold/threshold_poly.h"

namespace otpqc::threshold_signatures::dilithium::poly::utils {
    /**
     * \brief Converts a int_32_t values to a Number<> values without the montgomery and balanced modulus
     * \param value Integer value to be converted
     * \return Number<> value
     */
    inline math::Number<> PolyyCoeff_int32_to_Number(const int32_t val) {
        int32_t val_tmp = val;
        if (val_tmp < 0) {
            val_tmp += DILITHIUM_Q;
        }
        return math::Number<>(val_tmp);
    }

    /**
     * \brief Converts polyvecl in the dilithium to the PolynomialVectorShare with all positive elements.
     * \param v Pointer to the polyvecl structure
     * \return PolynomialVectorShare representing the coefficients of the polyvecl
     */
    inline PolynomialVectorShare<> polyvecl_to_polynomial_vector_share(const polyvecl *v) {
        PolynomialVectorShare<> vec;
        vec.reserve(DILITHIUM_L);

        for (int i = 0; i < DILITHIUM_L; ++i) {
            PolynomialShare<> poly_share;
            poly_share.reserve(DILITHIUM_N);
            for (int j = 0; j < DILITHIUM_N; ++j) {
                int coeff = v->vec[i].coeffs[j];
                // coeff = coeff < 0 ? coeff + DILITHIUM_Q : coeff; // Ensure coefficients are positive
                poly_share.emplace_back(PolyyCoeff_int32_to_Number(coeff));
            }
            vec.emplace_back(std::move(poly_share));
        }
        return vec;
    }

    /**
     * \brief Converts polyveck in the dilithium to the PolynomialVectorShare with all positive elements.
     * \param v Pointer to the polyveck structure
     * \return PolynomialVectorShare representing the coefficients of the polyveck
     */
    inline PolynomialVectorShare<> polyveck_to_polynomial_vector_share(const polyveck *v) {
        PolynomialVectorShare<> vec;
        vec.reserve(DILITHIUM_K);
        for (int i = 0; i < DILITHIUM_K; ++i) {
            PolynomialShare<> poly_share;
            poly_share.reserve(DILITHIUM_N);
            for (int j = 0; j < DILITHIUM_N; ++j) {
                int coeff = v->vec[i].coeffs[j];
                // coeff = coeff < 0 ? coeff + DILITHIUM_Q : coeff; // Ensure coefficients are positive
                poly_share.emplace_back(PolyyCoeff_int32_to_Number(coeff));
            }
            vec.emplace_back(std::move(poly_share));
        }
        return vec;
    }

    /**
     * \brief Check the equality of one PolynomialVectorShare with polyvec(including polyveck and polyvecl)
     * \param vec PolynomialVectorShare to be checked
     * \param polyvec Pointer to the polyvecl or polyveck structure
     * \return True if the PolynomialVectorShare is equal to the polyvec, false otherwise
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    bool polynomial_vector_share_equal_polyvec(
        const PolynomialVectorShare<> &vec, const void *polyvec) {
        size_t poly_vec_size = vec.size();

        if (poly_vec_size != DILITHIUM_L && poly_vec_size != DILITHIUM_K) {
            return false; // Size mismatch
        }

        const auto check_coeffs = [&](const auto *v) {
            for (size_t i = 0; i < poly_vec_size; ++i) {
                const auto &poly_share = vec[i];
                for (size_t j = 0; j < poly_share.size(); ++j) {
                    if (std::holds_alternative<PolyCoeffConstantShare<T> >(poly_share[j])) {
                        auto coeff_share = std::get<PolyCoeffConstantShare<T> >(poly_share[j]);
                        auto coeff_value = PolyyCoeff_int32_to_Number(v->vec[i].coeffs[j]);
                        coeff_value = coeff_value % otpqc::math::Number<>(DILITHIUM_Q);
                        // Ensure coefficients are reduced modulo DILITHIUM_Q
                        if (coeff_share.get_value() != coeff_value.get_value()) {
                            std::cout << "[polynomial_vector_share_equal_polyvec]Coefficient mismatch at index (" << i
                                    << ", " << j << "): "
                                    << coeff_share.get_value() << " != " << coeff_value.get_value() << std::endl;
                            return false; // Coefficient mismatch
                        }
                    }
                }
            }
            return true;
        };

        if (poly_vec_size == DILITHIUM_L) {
            return check_coeffs(static_cast<const polyvecl *>(polyvec));
        }

        return check_coeffs(static_cast<const polyveck *>(polyvec));
    }

    /**
     * \brief Converts a PolynomialVectorShare to arithmetic shares distributed among multiple parties
     * \param poly_vec_share The PolynomialVectorShare to distribute
     * \param dilithium_modulus The modulus for arithmetic sharing (typically DILITHIUM_Q)
     * \param sharing_global_key The global MAC key for arithmetic sharing
     * \param num_parties Number of MPC parties to distribute shares among (default: QST_NUM_OF_MPC_PARTIES)
     * \return Vector of PolynomialVectorShare, one for each party
     */
    inline std::vector<PolynomialVectorShare<> > polynomial_vector_to_arithmetic_shares(
        const PolynomialVectorShare<> &poly_vec_share,
        const math::Number<> &dilithium_modulus,
        const math::Number<> &sharing_global_key,
        const int num_parties = QST_NUM_OF_MPC_PARTIES) {
        std::vector<PolynomialVectorShare<> > distributed_shares(num_parties);

        // Initialize each party's PolynomialVectorShare with the same size as input
        for (auto &party_share: distributed_shares) {
            party_share.resize(poly_vec_share.size());
        }

        // Iterate through each polynomial in the vector
        for (size_t i = 0; i < poly_vec_share.size(); ++i) {
            const auto &poly_share = poly_vec_share[i];

            // Iterate through each coefficient in the polynomial
            for (size_t j = 0; j < poly_share.size(); ++j) {
                const auto &coeff_share = poly_share[j];

                // Extract the coefficient value from the share variant
                auto coeff_value = std::get<PolyCoeffConstantShare<> >(coeff_share);

                // Generate arithmetic shares for this coefficient
                auto new_coeff_shares = otpqc::mpc::sharing::ArithmeticSharing<>::generate_random_shares(
                    dilithium_modulus,
                    coeff_value,
                    sharing_global_key,
                    num_parties
                );

                // Distribute to each party
                for (int k = 0; k < num_parties; ++k) {
                    auto share_obj = otpqc::mpc::sharing::ArithmeticSharing<>{
                        dilithium_modulus,
                        new_coeff_shares[k].get_share(),
                        new_coeff_shares[k].get_mac_share()
                    };

                    distributed_shares[k][i].emplace_back(share_obj);
                }
            }
        }

        return distributed_shares;
    }

    /**
     * \brief Check the equality of the reconstruction from the std::vector<PolynomialVectorShare<> > with the olynomialVectorShare<>
     * \param poly_vec_share PolynomialVectorShare to be checked
     * \param reconstructed_shares Vector of PolynomialVectorShare representing the reconstructed shares
     * \return True if the reconstructed shares are equal to the poly_vec_share, false otherwise
     */
    static bool polynomial_vector_share_equal_reconstruction(
        const PolynomialVectorShare<> &poly_vec_share,
        const std::vector<PolynomialVectorShare<> > &reconstructed_shares,
        const otpqc::math::Number<> &global_mac_key) {
        if (reconstructed_shares.size() != QST_NUM_OF_MPC_PARTIES)
            return false; // Size mismatch
        PolynomialVectorShare<> reconstructed_poly_value;
        reconstructed_poly_value.resize(poly_vec_share.size());
        for (int k = 0; k < QST_NUM_OF_MPC_PARTIES; ++k) {
            for (int i = 0; i < reconstructed_poly_value.size(); ++i) {
                reconstructed_poly_value[i].resize(poly_vec_share[i].size());
                for (int j = 0; j < poly_vec_share[i].size(); ++j) {
                    if (k == 0) {
                        // Initialize the first party's share
                        reconstructed_poly_value[i][j] = reconstructed_shares[k][i][j];
                    } else {
                        // Combine shares from other parties
                        if (std::holds_alternative<PolyCoeffMPCArithShare<> >(reconstructed_poly_value[i][j]) &&
                            std::holds_alternative<PolyCoeffMPCArithShare<> >(reconstructed_shares[k][i][j])) {
                            auto current_share = std::get<PolyCoeffMPCArithShare<> >(reconstructed_poly_value[i][j]);
                            auto new_share = std::get<PolyCoeffMPCArithShare<> >(reconstructed_shares[k][i][j]);
                            // Combine the shares by adding them
                            reconstructed_poly_value[i][j] = PolyCoeffMPCArithShare<>{
                                current_share + new_share
                            };
                        }
                    }
                }
            }
        }
        // Now compare the reconstructed polynomial vector share with the original
        for (int i = 0; i < poly_vec_share.size(); ++i) {
            for (int j = 0; j < poly_vec_share[i].size(); ++j) {
                if (std::holds_alternative<PolyCoeffConstantShare<> >(poly_vec_share[i][j])) {
                    auto original_coeff = std::get<PolyCoeffConstantShare<> >(poly_vec_share[i][j]);
                    auto reconstructed_coeff = std::get<PolyCoeffMPCArithShare<> >(reconstructed_poly_value[i][j]);
                    if (original_coeff.get_value() != reconstructed_coeff.get_share().get_value()) {
                        std::cout << "Coefficient mismatch at index (" << i << ", " << j << "): "
                                << original_coeff.get_value() << " != "
                                << reconstructed_coeff.get_share().get_value() << std::endl;
                        return false; // Coefficient mismatch
                    }
                    // check the mac component
                    otpqc::math::Number<> expected_mac = (original_coeff * global_mac_key) % otpqc::math::Number<>(
                                                           DILITHIUM_Q);
                    if (reconstructed_coeff.get_mac_share().get_value() != expected_mac.get_value()) {
                        std::cout << "MAC mismatch at index (" << i << ", " << j << "): "
                                << reconstructed_coeff.get_mac_share().get_value() << " != "
                                << expected_mac.get_value() << std::endl;
                        return false; // MAC mismatch
                    }
                }
            }
        }
        return true; // All coefficients match
    }
}
#endif

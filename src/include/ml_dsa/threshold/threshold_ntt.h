#ifndef THRESHOLD_NTT_H
#define THRESHOLD_NTT_H

#include "ml_dsa/threshold/threshold_poly.h"
#include "ml_dsa/threshold/params.h"
#include "math/number.h"

namespace otpqc::threshold_signatures::dilithium::threshold_NTT{
    
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    static const otpqc::math::Number<T> F{8347681}; // Inverse of N modulo Q for DILITHIUM_N=256
    
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    static const otpqc::math::Number<T> ZETAS[DILITHIUM_N] = {
        otpqc::math::Number<T>{0},        otpqc::math::Number<T>{4808194},   otpqc::math::Number<T>{3765607},   otpqc::math::Number<T>{3761513},   otpqc::math::Number<T>{5178923},   otpqc::math::Number<T>{5496691},    otpqc::math::Number<T>{5234739},  otpqc::math::Number<T>{5178987}, 
        otpqc::math::Number<T>{7778734},  otpqc::math::Number<T>{3542485},   otpqc::math::Number<T>{2682288},   otpqc::math::Number<T>{2129892},   otpqc::math::Number<T>{3764867},   otpqc::math::Number<T>{7375178},    otpqc::math::Number<T>{557458},   otpqc::math::Number<T>{7159240}, 
        otpqc::math::Number<T>{5010068},  otpqc::math::Number<T>{4317364},   otpqc::math::Number<T>{2663378},   otpqc::math::Number<T>{6705802},   otpqc::math::Number<T>{4855975},   otpqc::math::Number<T>{7946292},    otpqc::math::Number<T>{676590},   otpqc::math::Number<T>{7044481}, 
        otpqc::math::Number<T>{5152541},  otpqc::math::Number<T>{1714295},   otpqc::math::Number<T>{2453983},   otpqc::math::Number<T>{1460718},   otpqc::math::Number<T>{7737789},   otpqc::math::Number<T>{4795319},    otpqc::math::Number<T>{2815639},  otpqc::math::Number<T>{2283733}, 
        otpqc::math::Number<T>{3602218},  otpqc::math::Number<T>{3182878},   otpqc::math::Number<T>{2740543},   otpqc::math::Number<T>{4793971},   otpqc::math::Number<T>{5269599},   otpqc::math::Number<T>{2101410},    otpqc::math::Number<T>{3704823},  otpqc::math::Number<T>{1159875}, 
        otpqc::math::Number<T>{394148},   otpqc::math::Number<T>{928749},    otpqc::math::Number<T>{1095468},   otpqc::math::Number<T>{4874037},   otpqc::math::Number<T>{2071829},   otpqc::math::Number<T>{4361428},    otpqc::math::Number<T>{3241972},  otpqc::math::Number<T>{2156050}, 
        otpqc::math::Number<T>{3415069},  otpqc::math::Number<T>{1759347},   otpqc::math::Number<T>{7562881},   otpqc::math::Number<T>{4805951},   otpqc::math::Number<T>{3756790},   otpqc::math::Number<T>{6444618},    otpqc::math::Number<T>{6663429},  otpqc::math::Number<T>{4430364}, 
        otpqc::math::Number<T>{5483103},  otpqc::math::Number<T>{3192354},   otpqc::math::Number<T>{556856},    otpqc::math::Number<T>{3870317},   otpqc::math::Number<T>{2917338},   otpqc::math::Number<T>{1853806},    otpqc::math::Number<T>{3345963},  otpqc::math::Number<T>{1858416}, 
        otpqc::math::Number<T>{3073009},  otpqc::math::Number<T>{1277625},   otpqc::math::Number<T>{5744944},   otpqc::math::Number<T>{3852015},   otpqc::math::Number<T>{4183372},   otpqc::math::Number<T>{5157610},    otpqc::math::Number<T>{5258977},  otpqc::math::Number<T>{8106357}, 
        otpqc::math::Number<T>{2508980},  otpqc::math::Number<T>{2028118},   otpqc::math::Number<T>{1937570},   otpqc::math::Number<T>{4564692},   otpqc::math::Number<T>{2811291},   otpqc::math::Number<T>{5396636},    otpqc::math::Number<T>{7270901},  otpqc::math::Number<T>{4158088}, 
        otpqc::math::Number<T>{1528066},  otpqc::math::Number<T>{482649},    otpqc::math::Number<T>{1148858},   otpqc::math::Number<T>{5418153},   otpqc::math::Number<T>{7814814},   otpqc::math::Number<T>{169688},     otpqc::math::Number<T>{2462444},  otpqc::math::Number<T>{5046034}, 
        otpqc::math::Number<T>{4213992},  otpqc::math::Number<T>{4892034},   otpqc::math::Number<T>{1987814},   otpqc::math::Number<T>{5183169},   otpqc::math::Number<T>{1736313},   otpqc::math::Number<T>{235407},     otpqc::math::Number<T>{5130263},  otpqc::math::Number<T>{3258457}, 
        otpqc::math::Number<T>{5801164},  otpqc::math::Number<T>{1787943},   otpqc::math::Number<T>{5989328},   otpqc::math::Number<T>{6125690},   otpqc::math::Number<T>{3482206},   otpqc::math::Number<T>{4197502},    otpqc::math::Number<T>{7080401},  otpqc::math::Number<T>{6018354}, 
        otpqc::math::Number<T>{7062739},  otpqc::math::Number<T>{2461387},   otpqc::math::Number<T>{3035980},   otpqc::math::Number<T>{621164},    otpqc::math::Number<T>{3901472},   otpqc::math::Number<T>{7153756},    otpqc::math::Number<T>{2925816},  otpqc::math::Number<T>{3374250}, 
        otpqc::math::Number<T>{1356448},  otpqc::math::Number<T>{5604662},   otpqc::math::Number<T>{2683270},   otpqc::math::Number<T>{5601629},   otpqc::math::Number<T>{4912752},   otpqc::math::Number<T>{2312838},    otpqc::math::Number<T>{7727142},  otpqc::math::Number<T>{7921254}, 
        otpqc::math::Number<T>{348812},   otpqc::math::Number<T>{8052569},   otpqc::math::Number<T>{1011223},   otpqc::math::Number<T>{6026202},   otpqc::math::Number<T>{4561790},   otpqc::math::Number<T>{6458164},    otpqc::math::Number<T>{6143691},  otpqc::math::Number<T>{1744507}, 
        otpqc::math::Number<T>{1753},     otpqc::math::Number<T>{6444997},   otpqc::math::Number<T>{5720892},   otpqc::math::Number<T>{6924527},   otpqc::math::Number<T>{2660408},   otpqc::math::Number<T>{6600190},    otpqc::math::Number<T>{8321269},  otpqc::math::Number<T>{2772600}, 
        otpqc::math::Number<T>{1182243},  otpqc::math::Number<T>{87208},     otpqc::math::Number<T>{636927},    otpqc::math::Number<T>{4415111},   otpqc::math::Number<T>{4423672},   otpqc::math::Number<T>{6084020},    otpqc::math::Number<T>{5095502},  otpqc::math::Number<T>{4663471}, 
        otpqc::math::Number<T>{8352605},  otpqc::math::Number<T>{822541},    otpqc::math::Number<T>{1009365},   otpqc::math::Number<T>{5926272},   otpqc::math::Number<T>{6400920},   otpqc::math::Number<T>{1596822},    otpqc::math::Number<T>{4423473},  otpqc::math::Number<T>{4620952}, 
        otpqc::math::Number<T>{6695264},  otpqc::math::Number<T>{4969849},   otpqc::math::Number<T>{2678278},   otpqc::math::Number<T>{4611469},   otpqc::math::Number<T>{4829411},   otpqc::math::Number<T>{635956},     otpqc::math::Number<T>{8129971},  otpqc::math::Number<T>{5925040}, 
        otpqc::math::Number<T>{4234153},  otpqc::math::Number<T>{6607829},   otpqc::math::Number<T>{2192938},   otpqc::math::Number<T>{6653329},   otpqc::math::Number<T>{2387513},   otpqc::math::Number<T>{4768667},    otpqc::math::Number<T>{8111961},  otpqc::math::Number<T>{5199961}, 
        otpqc::math::Number<T>{3747250},  otpqc::math::Number<T>{2296099},   otpqc::math::Number<T>{1239911},   otpqc::math::Number<T>{4541938},   otpqc::math::Number<T>{3195676},   otpqc::math::Number<T>{2642980},    otpqc::math::Number<T>{1254190},  otpqc::math::Number<T>{8368000}, 
        otpqc::math::Number<T>{2998219},  otpqc::math::Number<T>{141835},    otpqc::math::Number<T>{8291116},   otpqc::math::Number<T>{2513018},   otpqc::math::Number<T>{7025525},   otpqc::math::Number<T>{613238},     otpqc::math::Number<T>{7070156},  otpqc::math::Number<T>{6161950}, 
        otpqc::math::Number<T>{7921677},  otpqc::math::Number<T>{6458423},   otpqc::math::Number<T>{4040196},   otpqc::math::Number<T>{4908348},   otpqc::math::Number<T>{2039144},   otpqc::math::Number<T>{6500539},    otpqc::math::Number<T>{7561656},  otpqc::math::Number<T>{6201452}, 
        otpqc::math::Number<T>{6757063},  otpqc::math::Number<T>{2105286},   otpqc::math::Number<T>{6006015},   otpqc::math::Number<T>{6346610},   otpqc::math::Number<T>{586241},    otpqc::math::Number<T>{7200804},    otpqc::math::Number<T>{527981},   otpqc::math::Number<T>{5637006}, 
        otpqc::math::Number<T>{6903432},  otpqc::math::Number<T>{1994046},   otpqc::math::Number<T>{2491325},   otpqc::math::Number<T>{6987258},   otpqc::math::Number<T>{507927},    otpqc::math::Number<T>{7192532},    otpqc::math::Number<T>{7655613},  otpqc::math::Number<T>{6545891}, 
        otpqc::math::Number<T>{5346675},  otpqc::math::Number<T>{8041997},   otpqc::math::Number<T>{2647994},   otpqc::math::Number<T>{3009748},   otpqc::math::Number<T>{5767564},   otpqc::math::Number<T>{4148469},    otpqc::math::Number<T>{749577},   otpqc::math::Number<T>{4357667}, 
        otpqc::math::Number<T>{3980599},  otpqc::math::Number<T>{2569011},   otpqc::math::Number<T>{6764887},   otpqc::math::Number<T>{1723229},   otpqc::math::Number<T>{1665318},   otpqc::math::Number<T>{2028038},    otpqc::math::Number<T>{1163598},  otpqc::math::Number<T>{5011144}, 
        otpqc::math::Number<T>{3994671},  otpqc::math::Number<T>{8368538},   otpqc::math::Number<T>{7009900},   otpqc::math::Number<T>{3020393},   otpqc::math::Number<T>{3363542},   otpqc::math::Number<T>{214880},     otpqc::math::Number<T>{545376},   otpqc::math::Number<T>{7609976}, 
        otpqc::math::Number<T>{3105558},  otpqc::math::Number<T>{7277073},   otpqc::math::Number<T>{508145},    otpqc::math::Number<T>{7826699},   otpqc::math::Number<T>{860144},    otpqc::math::Number<T>{3430436},    otpqc::math::Number<T>{140244},   otpqc::math::Number<T>{6866265}, 
        otpqc::math::Number<T>{6195333},  otpqc::math::Number<T>{3123762},   otpqc::math::Number<T>{2358373},   otpqc::math::Number<T>{6187330},   otpqc::math::Number<T>{5365997},   otpqc::math::Number<T>{6663603},    otpqc::math::Number<T>{2926054},  otpqc::math::Number<T>{7987710}, 
        otpqc::math::Number<T>{8077412},  otpqc::math::Number<T>{3531229},   otpqc::math::Number<T>{4405932},   otpqc::math::Number<T>{4606686},   otpqc::math::Number<T>{1900052},   otpqc::math::Number<T>{7598542},    otpqc::math::Number<T>{1054478},  otpqc::math::Number<T>{7648983}
    };
    /*************************************************
     * Name:        threshold_ntt
     *
     * Description: Forward Number Theoretic Transform (NTT) for threshold cryptography,
     *              operating in-place on polynomial shares. This function performs the
     *              NTT transformation on MPC (Multi-Party Computation) arithmetic shares,
     *              enabling secure computation of polynomial operations. Output polynomial coefficients are in bit-reversed order.
     *
     * Arguments:   PolynomialShare<T> &a: input/output polynomial with MPC arithmetic shares
     *              Template parameter T: underlying numeric type for qst::math::Number<T>
     *
     * Throws:      std::invalid_argument if input contains constant shares instead of MPC shares
     *
     * Note:        Uses precomputed ZETAS roots of unity for DILITHIUM_N=256 and DILITHIUM_Q
     **************************************************/
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_ntt(poly::PolynomialShare<T> &a) {
        using namespace poly; //todo

        if (std::holds_alternative<poly::PolyCoeffConstantShare<T>>(a[0])) {
            throw std::invalid_argument("[NTT] Input coefficients must be MPC shares, not constant shares.");
        }
        unsigned int len, start, j, k;
        poly::CoefficientShare<T> t;
        k = 0;
        for(len = 128; len > 0; len >>= 1) {
            for(start = 0; start < DILITHIUM_N; start = j + len) {
                poly::CoefficientShare<T> zeta{ZETAS<T>[++k]};
                for(j = start; j < start + len; ++j) {
                    t = zeta * a[j + len];
                    a[j + len] = a[j] - t;
                    a[j] = a[j] + t;
                }
            }
        }
    }


    /*************************************************
     * Name:        threshold_invntt
     *
     * Description: Inverse Number Theoretic Transform (INTT) for threshold cryptography,
     *              operating in-place on polynomial shares. This function performs the
     *              inverse NTT transformation on MPC (Multi-Party Computation) arithmetic 
     *              shares, converting from frequency domain back to coefficient domain.
     *              No modular reductions are performed after additions/subtractions during
     *              the main loop. Input coefficients should be smaller than DILITHIUM_Q in
     *              absolute value. Output coefficients are normalized and smaller than 
     *              DILITHIUM_Q in absolute value.
     *
     * Arguments:   PolynomialShare<T> &a: input/output polynomial with MPC arithmetic shares
     *              Template parameter T: underlying numeric type for qst::math::Number<T>
     *
     * Throws:      std::invalid_argument if input contains constant shares instead of MPC shares
     *
     * Note:        Applies normalization factor F (inverse of N modulo Q) to complete the
     *              inverse transformation. Uses negated ZETAS values for inverse operation.
     **************************************************/
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_invntt(poly::PolynomialShare<T> &a) {
        using namespace poly; //todo

        if (std::holds_alternative<poly::PolyCoeffConstantShare<T>>(a[0])) {
            throw std::invalid_argument("[INVNTT] Input coefficients must be MPC shares, not constant shares.");
        }
        unsigned int start, len, j, k;
        poly::CoefficientShare<T> t;
        
        k = DILITHIUM_N;
        for(len = 1; len < DILITHIUM_N; len <<= 1) {
            for(start = 0; start < DILITHIUM_N; start = j + len) {
                poly::CoefficientShare<T> neg_zeta = poly::CoefficientShare<T>{otpqc::math::Number<T>{0}} - poly::CoefficientShare<T>{ZETAS<T>[--k]};
                for(j = start; j < start + len; ++j) {
                    t = a[j];
                    a[j] = t + a[j + len];
                    a[j + len] = t - a[j + len];
                    a[j + len] = neg_zeta * a[j + len];
                }
            }
        }
        for (j = 0; j < DILITHIUM_N; ++j) {
            poly::CoefficientShare<T> f{F<T>};
            a[j] = f * a[j];
        }
    }
    /**
     * \brief Applies the Number Theoretic Transform (NTT) to each polynomial in a vector of polynomial shares.
     *
     * This template function iterates through a PolynomialVectorShare and applies the threshold_ntt function
     * to each individual polynomial share.
     *
     * \param a A reference to the PolynomialVectorShare to be transformed.
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_ntt(poly::PolynomialVectorShare<T> &a) {
        for (auto &poly : a)
            threshold_ntt(poly);
    }

    /**
     * \brief Applies the Inverse Number Theoretic Transform (INTT) to each polynomial in a vector of polynomial shares.
     *
     * This template function iterates through a PolynomialVectorShare and applies the threshold_invntt function
     * to each individual polynomial share.
     *
     * \param a A reference to the PolynomialVectorShare to be transformed.
     */
    template<otpqc::math::IntegralNumeric T = QST_UNDERLYING_NUMERIC_TYPE>
    void threshold_invntt(poly::PolynomialVectorShare<T> &a) {
        for (auto &poly : a) {
            threshold_invntt(poly);
        }
    }

}

#endif
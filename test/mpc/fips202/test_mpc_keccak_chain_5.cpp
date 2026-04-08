#include "mpc/protocols/gc/circuit_exec.h"
#include "mpc/sharing/boolean_sharing.h"
#include <gtest/gtest.h>
#include <thread>
#include "party.h"


#define NROUNDS 24
#define ROL(a, offset) ((a << offset) ^ (a >> (64-offset)))


/* Keccak round constants */
const uint64_t KeccakF_RoundConstants[NROUNDS] = {
    (uint64_t) 0x0000000000000001ULL,
    (uint64_t) 0x0000000000008082ULL,
    (uint64_t) 0x800000000000808aULL,
    (uint64_t) 0x8000000080008000ULL,
    (uint64_t) 0x000000000000808bULL,
    (uint64_t) 0x0000000080000001ULL,
    (uint64_t) 0x8000000080008081ULL,
    (uint64_t) 0x8000000000008009ULL,
    (uint64_t) 0x000000000000008aULL,
    (uint64_t) 0x0000000000000088ULL,
    (uint64_t) 0x0000000080008009ULL,
    (uint64_t) 0x000000008000000aULL,
    (uint64_t) 0x000000008000808bULL,
    (uint64_t) 0x800000000000008bULL,
    (uint64_t) 0x8000000000008089ULL,
    (uint64_t) 0x8000000000008003ULL,
    (uint64_t) 0x8000000000008002ULL,
    (uint64_t) 0x8000000000000080ULL,
    (uint64_t) 0x000000000000800aULL,
    (uint64_t) 0x800000008000000aULL,
    (uint64_t) 0x8000000080008081ULL,
    (uint64_t) 0x8000000000008080ULL,
    (uint64_t) 0x0000000080000001ULL,
    (uint64_t) 0x8000000080008008ULL
};

/*************************************************
* Name:        KeccakF1600_StatePermute
*
* Description: The Keccak F1600 Permutation
*
* Arguments:   - uint64_t *state: pointer to input/output Keccak state
**************************************************/
static void KeccakF1600_StatePermute(uint64_t state[25]) {
    int round;

    uint64_t Aba, Abe, Abi, Abo, Abu;
    uint64_t Aga, Age, Agi, Ago, Agu;
    uint64_t Aka, Ake, Aki, Ako, Aku;
    uint64_t Ama, Ame, Ami, Amo, Amu;
    uint64_t Asa, Ase, Asi, Aso, Asu;
    uint64_t BCa, BCe, BCi, BCo, BCu;
    uint64_t Da, De, Di, Do, Du;
    uint64_t Eba, Ebe, Ebi, Ebo, Ebu;
    uint64_t Ega, Ege, Egi, Ego, Egu;
    uint64_t Eka, Eke, Eki, Eko, Eku;
    uint64_t Ema, Eme, Emi, Emo, Emu;
    uint64_t Esa, Ese, Esi, Eso, Esu;

    //copyFromState(A, state)
    Aba = state[0];
    Abe = state[1];
    Abi = state[2];
    Abo = state[3];
    Abu = state[4];
    Aga = state[5];
    Age = state[6];
    Agi = state[7];
    Ago = state[8];
    Agu = state[9];
    Aka = state[10];
    Ake = state[11];
    Aki = state[12];
    Ako = state[13];
    Aku = state[14];
    Ama = state[15];
    Ame = state[16];
    Ami = state[17];
    Amo = state[18];
    Amu = state[19];
    Asa = state[20];
    Ase = state[21];
    Asi = state[22];
    Aso = state[23];
    Asu = state[24];

    for (round = 0; round < NROUNDS; round += 2) {
        //    prepareTheta
        BCa = Aba ^ Aga ^ Aka ^ Ama ^ Asa;
        BCe = Abe ^ Age ^ Ake ^ Ame ^ Ase;
        BCi = Abi ^ Agi ^ Aki ^ Ami ^ Asi;
        BCo = Abo ^ Ago ^ Ako ^ Amo ^ Aso;
        BCu = Abu ^ Agu ^ Aku ^ Amu ^ Asu;

        //thetaRhoPiChiIotaPrepareTheta(round, A, E)
        Da = BCu ^ ROL(BCe, 1);
        De = BCa ^ ROL(BCi, 1);
        Di = BCe ^ ROL(BCo, 1);
        Do = BCi ^ ROL(BCu, 1);
        Du = BCo ^ ROL(BCa, 1);

        Aba ^= Da;
        BCa = Aba;
        Age ^= De;
        BCe = ROL(Age, 44);
        Aki ^= Di;
        BCi = ROL(Aki, 43);
        Amo ^= Do;
        BCo = ROL(Amo, 21);
        Asu ^= Du;
        BCu = ROL(Asu, 14);
        Eba = BCa ^ ((~BCe) & BCi);
        Eba ^= (uint64_t) KeccakF_RoundConstants[round];
        Ebe = BCe ^ ((~BCi) & BCo);
        Ebi = BCi ^ ((~BCo) & BCu);
        Ebo = BCo ^ ((~BCu) & BCa);
        Ebu = BCu ^ ((~BCa) & BCe);

        Abo ^= Do;
        BCa = ROL(Abo, 28);
        Agu ^= Du;
        BCe = ROL(Agu, 20);
        Aka ^= Da;
        BCi = ROL(Aka, 3);
        Ame ^= De;
        BCo = ROL(Ame, 45);
        Asi ^= Di;
        BCu = ROL(Asi, 61);
        Ega = BCa ^ ((~BCe) & BCi);
        Ege = BCe ^ ((~BCi) & BCo);
        Egi = BCi ^ ((~BCo) & BCu);
        Ego = BCo ^ ((~BCu) & BCa);
        Egu = BCu ^ ((~BCa) & BCe);

        Abe ^= De;
        BCa = ROL(Abe, 1);
        Agi ^= Di;
        BCe = ROL(Agi, 6);
        Ako ^= Do;
        BCi = ROL(Ako, 25);
        Amu ^= Du;
        BCo = ROL(Amu, 8);
        Asa ^= Da;
        BCu = ROL(Asa, 18);
        Eka = BCa ^ ((~BCe) & BCi);
        Eke = BCe ^ ((~BCi) & BCo);
        Eki = BCi ^ ((~BCo) & BCu);
        Eko = BCo ^ ((~BCu) & BCa);
        Eku = BCu ^ ((~BCa) & BCe);

        Abu ^= Du;
        BCa = ROL(Abu, 27);
        Aga ^= Da;
        BCe = ROL(Aga, 36);
        Ake ^= De;
        BCi = ROL(Ake, 10);
        Ami ^= Di;
        BCo = ROL(Ami, 15);
        Aso ^= Do;
        BCu = ROL(Aso, 56);
        Ema = BCa ^ ((~BCe) & BCi);
        Eme = BCe ^ ((~BCi) & BCo);
        Emi = BCi ^ ((~BCo) & BCu);
        Emo = BCo ^ ((~BCu) & BCa);
        Emu = BCu ^ ((~BCa) & BCe);

        Abi ^= Di;
        BCa = ROL(Abi, 62);
        Ago ^= Do;
        BCe = ROL(Ago, 55);
        Aku ^= Du;
        BCi = ROL(Aku, 39);
        Ama ^= Da;
        BCo = ROL(Ama, 41);
        Ase ^= De;
        BCu = ROL(Ase, 2);
        Esa = BCa ^ ((~BCe) & BCi);
        Ese = BCe ^ ((~BCi) & BCo);
        Esi = BCi ^ ((~BCo) & BCu);
        Eso = BCo ^ ((~BCu) & BCa);
        Esu = BCu ^ ((~BCa) & BCe);

        //    prepareTheta
        BCa = Eba ^ Ega ^ Eka ^ Ema ^ Esa;
        BCe = Ebe ^ Ege ^ Eke ^ Eme ^ Ese;
        BCi = Ebi ^ Egi ^ Eki ^ Emi ^ Esi;
        BCo = Ebo ^ Ego ^ Eko ^ Emo ^ Eso;
        BCu = Ebu ^ Egu ^ Eku ^ Emu ^ Esu;

        //thetaRhoPiChiIotaPrepareTheta(round+1, E, A)
        Da = BCu ^ ROL(BCe, 1);
        De = BCa ^ ROL(BCi, 1);
        Di = BCe ^ ROL(BCo, 1);
        Do = BCi ^ ROL(BCu, 1);
        Du = BCo ^ ROL(BCa, 1);

        Eba ^= Da;
        BCa = Eba;
        Ege ^= De;
        BCe = ROL(Ege, 44);
        Eki ^= Di;
        BCi = ROL(Eki, 43);
        Emo ^= Do;
        BCo = ROL(Emo, 21);
        Esu ^= Du;
        BCu = ROL(Esu, 14);
        Aba = BCa ^ ((~BCe) & BCi);
        Aba ^= (uint64_t) KeccakF_RoundConstants[round + 1];
        Abe = BCe ^ ((~BCi) & BCo);
        Abi = BCi ^ ((~BCo) & BCu);
        Abo = BCo ^ ((~BCu) & BCa);
        Abu = BCu ^ ((~BCa) & BCe);

        Ebo ^= Do;
        BCa = ROL(Ebo, 28);
        Egu ^= Du;
        BCe = ROL(Egu, 20);
        Eka ^= Da;
        BCi = ROL(Eka, 3);
        Eme ^= De;
        BCo = ROL(Eme, 45);
        Esi ^= Di;
        BCu = ROL(Esi, 61);
        Aga = BCa ^ ((~BCe) & BCi);
        Age = BCe ^ ((~BCi) & BCo);
        Agi = BCi ^ ((~BCo) & BCu);
        Ago = BCo ^ ((~BCu) & BCa);
        Agu = BCu ^ ((~BCa) & BCe);

        Ebe ^= De;
        BCa = ROL(Ebe, 1);
        Egi ^= Di;
        BCe = ROL(Egi, 6);
        Eko ^= Do;
        BCi = ROL(Eko, 25);
        Emu ^= Du;
        BCo = ROL(Emu, 8);
        Esa ^= Da;
        BCu = ROL(Esa, 18);
        Aka = BCa ^ ((~BCe) & BCi);
        Ake = BCe ^ ((~BCi) & BCo);
        Aki = BCi ^ ((~BCo) & BCu);
        Ako = BCo ^ ((~BCu) & BCa);
        Aku = BCu ^ ((~BCa) & BCe);

        Ebu ^= Du;
        BCa = ROL(Ebu, 27);
        Ega ^= Da;
        BCe = ROL(Ega, 36);
        Eke ^= De;
        BCi = ROL(Eke, 10);
        Emi ^= Di;
        BCo = ROL(Emi, 15);
        Eso ^= Do;
        BCu = ROL(Eso, 56);
        Ama = BCa ^ ((~BCe) & BCi);
        Ame = BCe ^ ((~BCi) & BCo);
        Ami = BCi ^ ((~BCo) & BCu);
        Amo = BCo ^ ((~BCu) & BCa);
        Amu = BCu ^ ((~BCa) & BCe);

        Ebi ^= Di;
        BCa = ROL(Ebi, 62);
        Ego ^= Do;
        BCe = ROL(Ego, 55);
        Eku ^= Du;
        BCi = ROL(Eku, 39);
        Ema ^= Da;
        BCo = ROL(Ema, 41);
        Ese ^= De;
        BCu = ROL(Ese, 2);
        Asa = BCa ^ ((~BCe) & BCi);
        Ase = BCe ^ ((~BCi) & BCo);
        Asi = BCi ^ ((~BCo) & BCu);
        Aso = BCo ^ ((~BCu) & BCa);
        Asu = BCu ^ ((~BCa) & BCe);
    }

    //copyToState(state, A)
    state[0] = Aba;
    state[1] = Abe;
    state[2] = Abi;
    state[3] = Abo;
    state[4] = Abu;
    state[5] = Aga;
    state[6] = Age;
    state[7] = Agi;
    state[8] = Ago;
    state[9] = Agu;
    state[10] = Aka;
    state[11] = Ake;
    state[12] = Aki;
    state[13] = Ako;
    state[14] = Aku;
    state[15] = Ama;
    state[16] = Ame;
    state[17] = Ami;
    state[18] = Amo;
    state[19] = Amu;
    state[20] = Asa;
    state[21] = Ase;
    state[22] = Asi;
    state[23] = Aso;
    state[24] = Asu;
}


void run_party(qst::MPCParty<> &party, const std::vector<bool> &input_bits, std::vector<bool> &output_bits) {
    party.setup_communication();

    auto circuits = qst::mpc::MPCContext<>::create_circuit(
        GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600,
        1,
        party.get_io(),
        &party.get_thread_pool(),
        party.get_id());

    for (const auto &circuit: circuits) {
        party.get_mpc_context().register_circuit(circuit);
    }

    auto circuit = party.get_mpc_context().get_registered_circuit(GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600);
    circuit.mpc_gc->preprocess();

    auto result = qst::mpc::protocols::gc::circuit::run(circuit, &input_bits);
    output_bits.insert(output_bits.end(), result.begin(), result.end());
}

TEST(MPCNetworkTest, ZeroStateTest) {
    qst::MPCParty party1{1, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};
    qst::MPCParty party2{2, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};

    Number<mpz_class> number{};
    auto shares = qst::mpc::sharing::BooleanSharing<mpz_class>::generate_random_shares(number, 1600, 2);

    auto party1_bits = shares[0].get_share().bits_be_ze(1600);
    auto party2_bits = shares[1].get_share().bits_be_ze(1600);

    std::vector<bool> party1_output_bits, party2_output_bits;

    std::thread thread1(run_party, std::ref(party1), std::ref(party1_bits), std::ref(party1_output_bits));
    std::thread thread2(run_party, std::ref(party2), std::ref(party2_bits), std::ref(party2_output_bits));
    thread1.join();
    thread2.join();

    std::vector<bool> output_bits(1600);
    for (int i = 0; i < 1600; ++i)
        output_bits[i] = party1_output_bits[i] ^ party2_output_bits[i];


    /* Computing the correct output of the Keccak permutation function */
    uint64_t state[25] = {};
    KeccakF1600_StatePermute(state);
    std::vector<bool> real_bits;
    real_bits.reserve(1600);
    for (unsigned long i: state) {
        for (auto bits = Number{i}.bits_be_ze(64); auto b: bits)
            real_bits.emplace_back(b);
    }

    for (size_t i = 0; i < real_bits.size(); ++i) {
        EXPECT_EQ(output_bits[i], real_bits[i]) << "Mismatch at bit " << i;
    }
}

TEST(MPCNetworkTest, RandomStateTest) {
    qst::MPCParty party1{1, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};
    qst::MPCParty party2{2, "127.0.0.1", 12345, qst::mpc::MPCContext<>{}};

    Number<mpz_class> number{"11046523866547493700194738121961604509657437141950601115727861198714174764042238031049129692587036242041435177724285736589899870108357133994399503065255109520311292167255636711314179973051836737773288590152909382727687675374213062207812994109721873416920091352729493561774334841781547326427652674413397306995144016102514881865697659668352908970583952666272591519669055933427695790920231977319728484290087099811291651043674755457561910336286787430189308022120021395142189966349915477"};
    auto shares = qst::mpc::sharing::BooleanSharing<mpz_class>::generate_random_shares(number, 1600, 2);

    auto party1_bits = shares[0].get_share().bits_be_ze(1600);
    auto party2_bits = shares[1].get_share().bits_be_ze(1600);

    std::vector<bool> party1_output_bits, party2_output_bits;

    std::thread thread1(run_party, std::ref(party1), std::ref(party1_bits), std::ref(party1_output_bits));
    std::thread thread2(run_party, std::ref(party2), std::ref(party2_bits), std::ref(party2_output_bits));
    thread1.join();
    thread2.join();

    std::vector<bool> output_bits(1600);
    for (int i = 0; i < 1600; ++i)
        output_bits[i] = party1_output_bits[i] ^ party2_output_bits[i];


    /* Computing the correct output of the Keccak permutation function */
    uint64_t state[25] = {
        0x3F9A2D9E45C7B102, 0x7AD54C9B0F23E1DA, 0x82B374F1A998DCB4, 0xD119BC8A2349FA11, 0x8C3A7F4D6B012349,
        0x2E68F23B5A019CEF, 0xE7DA3B91F2BC78A4, 0x19C84E7A3A0BDF56, 0xC01479DE0A49B3FE, 0x5AE79FD1CC0D3482,
        0x9B3A5F9D0E812334, 0x137CCFBE4DA87901, 0xAFF13E7D9BC42A05, 0x01FEDCBA98765432, 0xACDC13579BDF2468,
        0x9EADBEEF00112233, 0x1122334455667788, 0x66778899AABBCCDD, 0x0F0E0D0C0B0A0908, 0x1234567890ABCDEF,
        0xDEADBEEFCAFEBABE, 0x0102030405060708, 0x8888777766665555, 0x3333444455556666, 0xAAAAAAAA55555555
    };
    KeccakF1600_StatePermute(state);
    std::vector<bool> real_bits;
    real_bits.reserve(1600);
    for (unsigned long i: state) {
        for (auto bits = Number{i}.bits_be_ze(64); auto b: bits)
            real_bits.emplace_back(b);
    }

    for (size_t i = 0; i < real_bits.size(); ++i) {
        EXPECT_EQ(output_bits[i], real_bits[i]) << "Mismatch at bit " << i;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
}

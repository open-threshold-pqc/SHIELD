#include <stdint.h>
#include <iostream>
#include <chrono>

#include "dilithium/threshold_dilithium/symmetric.h"


#define SHAKE_INPUT_SIZE (38)
#define SHAKE_OUTPUT_SIZE (32)


void print_state(uint64_t state[25]) {
  for (int i = 0; i < 25; i++) {
    std::bitset<64> bits(state[i]);  // Convert each uint64_t to a bitset of 64 bits
    std::cout << "s[" << i << "]: " << bits << std::endl;
  }
  std::cout << "\n";
}


void thread_run_party(qst::Party &party, const uint8_t *input, uint64_t *output) {
  party.setup_communication();
  party.get_mpc_core().register_gc_circuit(
      qst::mpc::protocols::gc::circuit::FUNCTIONS::KECCAK_F_PERMUTATION_1600_1600,
      party.get_io(), party.get_id(), 6);


  // Shared seed
  uint8_t seed[SHAKE_OUTPUT_SIZE] = {0x00};
  uint16_t nonce {};
  if (party.get_id() == 1) {

    seed[0] = 0x01;
    seed[1] = 0xac;

  }

  auto start = std::chrono::high_resolution_clock::now();

  //
  uint8_t buf[100000];
  stream256_state state;
  //
  threshold_dilithium_shake256_stream_init(party, &state, seed, nonce);
  threshold_shake256_squeezeblocks(party, buf, 6, &state);



  auto end = std::chrono::high_resolution_clock::now();
  auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  // Output results
  std::cout << "Execution time: " << duration_ns << " ns (" << duration_us << " µs)" << std::endl;


  // Write the state for checking
  for (int i=0; i<25; i++) {
    output[i] = state.s[i];
  }



}


void run_threshold_shake256(uint8_t *input) {
  qst::Party party1{1, "127.0.0.1", 12345};
  qst::Party party2{2, "127.0.0.1", 12345};


  /* Secret share the input */
  // Input is fixed to all bytes of 0. Hence, we assume that the share of the inputs are two vector 0xff bytes for simplity

  uint8_t input_1[SHAKE_INPUT_SIZE] = {0x00};
  uint8_t input_2[SHAKE_INPUT_SIZE] = {0x00};

  uint64_t output_1[25] = {0};
  uint64_t output_2[25] = {0};

  std::cout << "Running the threads" << std::endl;

  /* Run the clients */
  std::thread thread_party_1(thread_run_party, std::ref(party1), input_1, output_1);
  std::thread thread_party_2(thread_run_party, std::ref(party2), input_2, output_2);

  thread_party_1.join();
  thread_party_2.join();


  uint64_t final_s[25];
  for (int i=0; i<25; i++) {
    final_s[i] = output_1[i] ^ output_2[i];
  }


  print_state(final_s);

  std::cout << "\n";
}



int main() {
  std::cout << "Threshold Shake Stream" << std::endl;

  uint16_t nonce {};
  uint8_t seed[100] = {0x00};
  seed[0] = 0x01;
  seed[1] = 0xac;

  uint8_t output[200000] = {0};

  stream256_state state;
  stream256_init(&state, seed, nonce);
  stream256_squeezeblocks(output, 6, &state);

  std::cout << "ORIGINAL \n";
  print_state(state.s);



  std::cout << "THRESHOLD \n";
  run_threshold_shake256(seed);


  //
  // for (int i = 0; i < 25; i++) {
  //   std::cout << "s[" << i << "]: " << state.s[i] << std::endl;
  // }

  return 0;
}

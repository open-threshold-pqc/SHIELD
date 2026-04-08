#include <stdint.h>
#include <iostream>
#include "dilithium/threshold_dilithium/fips202.h"
#include "party.h"
#include <chrono>
#include "mpc/protocols/gc/circuit.h"

#define SHAKE_INPUT_SIZE (139)
#define SHAKE_OUTPUT_SIZE (2*136)


void print_state(uint64_t state[25]) {
  for (int i = 0; i < 25; i++) {
    std::bitset<64> bits(state[i]);  // Convert each uint64_t to a bitset of 64 bits
    std::cout << "s[" << i << "]: " << bits << std::endl;
  }
  std::cout << "\n";
}

double global_time = 0;

void thread_run_party(qst::Party &party, const uint8_t *input, uint64_t *output) {
  party.setup_communication();
  party.get_mpc_core().register_gc_circuit(
      qst::mpc::protocols::gc::circuit::FUNCTIONS::KECCAK_F_PERMUTATION_1600_1600,
      party.get_io(), party.get_id(), 3);


  uint8_t buff[SHAKE_OUTPUT_SIZE] = {0xff};


  keccak_state state;
  threshold_shake256_init(&state);



  threshold_shake256_absorb(party, &state, input, SHAKE_INPUT_SIZE);
  threshold_shake256_finalize(party, &state);





  // threshold_shake256_squeeze(party, buff, SHAKE_OUTPUT_SIZE, &state);
  std:vector<bool> input1;
  for (int i = 0; i < 1600; i++)
    input1.push_back(false);

  auto start = std::chrono::high_resolution_clock::now();

  auto circuit_output = run(party,
                            qst::mpc::protocols::gc::circuit::FUNCTIONS::KECCAK_F_PERMUTATION_1600_1600, &input1);


  auto end = std::chrono::high_resolution_clock::now();
  auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  if (party.get_id() == 1)
    global_time += duration_us;

  // Output results
  // std::cout << "Execution time: " << duration_ns << " ns (" << duration_us << " µs)" << std::endl;


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

  uint8_t input_1[SHAKE_INPUT_SIZE] = {0xff};
  uint8_t input_2[SHAKE_INPUT_SIZE] = {0xff};

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


  // print_state(final_s);

  // std::cout << "\n";
}



int main() {
  std::cout << "Threshold Shake" << std::endl;
  uint8_t input[SHAKE_INPUT_SIZE] = {0};
  uint8_t output[SHAKE_OUTPUT_SIZE] = {0};

  keccak_state state;
  shake256_init(&state);
  // state.s[0] = 1;
  shake256_absorb(&state, input, SHAKE_INPUT_SIZE);
  shake256_finalize(&state);
  shake256_squeeze(output, SHAKE_OUTPUT_SIZE, &state);

  std::cout << "ORIGINAL \n";
  print_state(state.s);

  std::cout << "THRESHOLD \n";
#define ITER 100
  for (int i=0;i<ITER;i++)
    run_threshold_shake256(input);

  std::cout << "Execution time: " << global_time/ITER << "µs" << std::endl;

  return 0;
}

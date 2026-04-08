#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include "mpc/protocols/gc/emp-mpc/mpc.h"
#include "mpc/protocols/gc/function_context.h"
#include "mpc/protocols/gc/function_codes.h"
#include "math/number.h"
#include <vector>

/**
 * \brief Namespace containing GC circuit execution API and Function Codes
 */
namespace otpqc::mpc::protocols::gc::circuit {
    /**
     * \brief This function is the main online phase of running a GC circuit among multiple parties
     * \param gc_function GC function context variable defining the function
     * \param circuit_input Boolean array of input (bits)
     * \return Vector of boolean values (bits)
     */
    [[nodiscard]] inline std::vector<bool> circuit_exec(const GCFunctionContext &gc_function,
                                                        bool *circuit_input) {
        const std::unique_ptr<bool[]> circuit_output(new bool[gc_function.output_size]);
        memset(circuit_output.get(), false, gc_function.output_size);

        /* Run the MPC */
        gc_function.mpc_gc->online(circuit_input, circuit_output.get(), 1);

        /* Returning the circuit output as a vector of booleans */
        std::vector<bool> output(gc_function.output_size);
        for (int i = 0; i < gc_function.output_size; i++)
            output[i] = circuit_output[i];

        return output;
    }

    /**
     * \brief This function performs preprocessing over the given inputs based on the functionality.
     * \param gc_function GC function context variable defining the function
     * \param input1 Pointer to the vector of first input bits
     * \param input2 Pointer to the vector of second input bits
     * \return Array of bits representing the processed input ready for execution
     */
    inline std::unique_ptr<bool[]> circuit_input_preprocess(const GCFunctionContext &gc_function,
                                                            const std::vector<bool> *input1,
                                                            const std::vector<bool> *input2 = nullptr) {
        /* Create a bool array as the circuit execution input */
        std::unique_ptr<bool[]> execution_input(new bool[gc_function.input1_size + gc_function.input2_size]);
        memset(execution_input.get(), false, gc_function.input1_size + gc_function.input2_size);

        /* Handling the first input */
        if (gc_function.operation == GC_FUNCTION_CODE::SHA256_512_0_256 ||
            gc_function.operation == GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600 ||
            gc_function.operation == GC_FUNCTION_CODE::DILITHIUM2_DECOMPOSE ||
            gc_function.operation == GC_FUNCTION_CODE::DILITHIUM35_DECOMPOSE){
            for (int i = 0; i < gc_function.input1_size; i++)
                execution_input[i] = input1->at(i);
        } else if (gc_function.operation == GC_FUNCTION_CODE::DILITHIUM2_SAMPLE_IN_BALL ||
            gc_function.operation == GC_FUNCTION_CODE::DILITHIUM3_SAMPLE_IN_BALL ||
            gc_function.operation == GC_FUNCTION_CODE::DILITHIUM5_SAMPLE_IN_BALL) {
            /* Swapping each byte in place */
            for (int i = 0; i < gc_function.input1_size; i += 8)
                for (int j = 0; j < 8; j++)
                    execution_input[i+j] = input1->at(i + 8 - 1 - j);
        } else {
            for (int i = 0; i < gc_function.input1_size; i++)
                execution_input[i] = input1->at(gc_function.input1_size - 1 - i);
        }

        /* Handling the second input */
        if (input2) {
            if (input2->size() != gc_function.input2_size)
                throw std::runtime_error(
                    "[MPC::GC::input_preprocess] Input 2's size mismatch with circuit's second input size");
            for (int i = 0; i < gc_function.input2_size; i++)
                execution_input[gc_function.input1_size + i] = input2->at(gc_function.input2_size - 1 - i);
        }

        if (gc_function.operation == GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600) {
            for (int i = 0; i < gc_function.input1_size; i++)
                execution_input[i] = input1->at(i);

            // Reordering the 64 bits
            for (int i = 0; i < 1600; i += 64) {
                for (int j = 0; j < 64 / 2; j++) {
                    const bool temp = execution_input[i + j];
                    execution_input[i + j] = execution_input[i + 64 - j - 1];
                    execution_input[i + 64 - j - 1] = temp;
                }
            }
            // Reordering the 8 Bytes
            for (int i = 0; i < 1600 / 2; i += 64) {
                const int swap_index{1600 - 64 - i};
                for (int j = 0; j < 64; j++) {
                    const bool temp = execution_input[i + j];
                    execution_input[i + j] = execution_input[swap_index + j];
                    execution_input[swap_index + j] = temp;
                }
            }

            for (int i = 0; i < 1600; i += 64) {
                for (int j = 0; j < 64 / 2; j += 8) {
                    const int swap_index = i + 64 - 8 - j; // Index for swapping
                    for (int k = 0; k < 8; k++) {
                        const bool temp = execution_input[i + j + k];
                        execution_input[i + j + k] = execution_input[swap_index + k];
                        execution_input[swap_index + k] = temp;
                    }
                }
            }
        }
        return execution_input;
    }


    /**
     * \brief This function performs postprocessing over the output of a GC function based on the operation.
     * \param gc_function GC function context variable defining the function
     * \param output Pointer to the vector of output bits
     */
    inline void circuit_output_postprocess(const GCFunctionContext &gc_function,
                                           std::vector<bool> &output) {
        // if (gc_function.operation == GC_FUNCTION_CODE::DILITHIUM2_SAMPLE_IN_BALL) {
        //     /* Swapping bytes */
        //     for (int i=0; i< gc_function.output_size / 16; i+=8) {
        //         int last_index = gc_function.output_size - i - 8;
        //         for (int j=0; j<8; j++) {
        //             const bool temp = output[i+j];
        //             output[i+j] = output[last_index + j];
        //             output[last_index + j] = temp;
        //         }
        //     }
        // }
        // else
        //
        if (gc_function.operation != GC_FUNCTION_CODE::SHA256_512_0_256)
            /* Returning the result in big endian format */
            std::reverse(output.begin(), output.end());


        if (gc_function.operation == GC_FUNCTION_CODE::KECCAK_F_PERMUTATION_1600_1600) {
            for (size_t i = 0; i < gc_function.input1_size; i += 64) {
                for (int j = 0; j < 64 / 2; j += 8) {
                    const int swap_index = 64 - 8 - j;
                    // Swap 8-bit segments
                    for (int k = 0; k < 8; ++k) {
                        const bool temp = output[i + j + k];
                        output[i + j + k] = output[i + swap_index + k];
                        output[i + swap_index + k] = temp;
                    }
                }
            }
        }
    }


    /**
     * \brief This function runs the GC functionality
     * @param gc_function GC function context variable that defines the function
     * @param input1 Pointer to the first input bits
     * @param input2 Pointer to the second inputs bits (if any)
     * @return Vector of bits as the result of the circuit
     */
    [[nodiscard]] inline std::vector<bool> run(const GCFunctionContext &gc_function,
                                               const std::vector<bool> *input1,
                                               const std::vector<bool> *input2 = nullptr) {
        if (gc_function.input1_size != input1->size())
            throw std::runtime_error("[MPC::GC::run] Input 1 size is not same as the circuit input 1");

        if (input2 && gc_function.input2_size != input2->size())
            throw std::runtime_error("[MPC::GC::run] Input 2 size is not same as the circuit input 2");

        /* Perform any preprocessing on the inputs of the circuit.
         *
         * Note: This preprocessing is because of our supported circuits. Some circuits need their input bits
         * to be in the big endian format (if inputs are numbers) and some little endian. To avoid delegating this task
         * to the use, we perform such conversions here, and we require users to pass inputs in big endian format.
         */
        const auto execution_input = circuit_input_preprocess(gc_function, input1, input2);

        /* Execute the circuit */
        auto execution_output = circuit_exec(gc_function, execution_input.get());

        /* Perform any postprocessing on the output of the circuit.
         *
         * Note: Similar to Preprocessing
         */
        circuit_output_postprocess(gc_function, execution_output);

        return execution_output;
    }
}


#endif

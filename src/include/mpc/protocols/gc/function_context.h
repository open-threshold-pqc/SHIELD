#ifndef GC_FUNCTION_H
#define GC_FUNCTION_H

#include "mpc/protocols/gc/emp-mpc/mpc.h"
#include "mpc/protocols/gc/function_codes.h"

namespace otpqc::mpc::protocols::gc::circuit {
    /**
     * \brief Context structer describing a GC function with operation, GC implementation class, and input/outputs
     */
    struct GCFunctionContext {
        GC_FUNCTION_CODE operation;
        CMPC<QST_NUM_OF_MPC_PARTIES> *mpc_gc; //todo
        int input1_size;
        int input2_size;
        int output_size;

        GCFunctionContext(const GC_FUNCTION_CODE op, CMPC<QST_NUM_OF_MPC_PARTIES>* gc, const int in1_size,
                          const int in2_size,
                          const int out_size): operation(op), mpc_gc(gc),
                                               input1_size(in1_size), input2_size(in2_size), output_size(out_size) {
        }
    };
}
#endif

cmake_minimum_required(VERSION 3.26)

project(QuantumSafeThreshold LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-O3 -w -maes -msse4.1 -mavx -mavx2)

find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Doxygen REQUIRED)

find_path(GMP_INCLUDE_DIR gmp.h)
find_library(GMP_LIBRARY NAMES gmp)
find_library(GMPXX_LIBRARY NAMES gmpxx)

# Fail early if any not found
if (NOT GMP_INCLUDE_DIR OR NOT GMP_LIBRARY OR NOT GMPXX_LIBRARY)
    message(FATAL_ERROR "Could not find GMP and/or GMPXX")
endif()

# todo remove this line
link_libraries(gmp gmpxx)


####################################
########### Variables ##############
####################################
set(QST_NUM_OF_MPC_PARTIES 2 CACHE STRING "Number of MPC parties")
set(QST_ML_DSA_MODE 2 CACHE STRING "ML-DSA Mode")
set(QST_UNDERLYING_NUMERIC_TYPE "int64_t" CACHE STRING "Underlying numeric type used in MPC")
set(GC_BRISTOL_CIRCUITS_BASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/include/mpc/protocols/gc/circuit/core_circuits")
set(DOXYGEN_CONFIG_FILE ${CMAKE_SOURCE_DIR}/doc/Doxyfile)

######## Output Directories ########
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
set(TEST_OUTPUT_DIR ${OUTPUT_DIR}/test)
set(BENCH_OUTPUT_DIR ${OUTPUT_DIR}/bench)
set(DOC_OUTPUT_DIR ${OUTPUT_DIR}/doc)
file(MAKE_DIRECTORY ${TEST_OUTPUT_DIR})
file(MAKE_DIRECTORY ${BENCH_OUTPUT_DIR})
file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})


add_compile_definitions(
        QST_NUM_OF_MPC_PARTIES=${QST_NUM_OF_MPC_PARTIES}
        QST_ML_DSA_MODE=${QST_ML_DSA_MODE}
        QST_UNDERLYING_NUMERIC_TYPE=${QST_UNDERLYING_NUMERIC_TYPE}
        GC_BRISTOL_CIRCUITS_BASE_PATH="${GC_BRISTOL_CIRCUITS_BASE_PATH}"
)
message(STATUS "======================================")
message(STATUS " QuantumSafeThreshold Configuration")
message(STATUS "--------------------------------------")
message(STATUS " QST_NUM_OF_MPC_PARTIES        = ${QST_NUM_OF_MPC_PARTIES}")
message(STATUS " QST_ML_DSA_MODE               = ${QST_ML_DSA_MODE}")
message(STATUS " QST_UNDERLYING_NUMERIC_TYPE   = ${QST_UNDERLYING_NUMERIC_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER            = ${CMAKE_CXX_COMPILER}")
message(STATUS " CMAKE_BUILD_TYPE              = ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_STANDARD            = ${CMAKE_CXX_STANDARD}")
message(STATUS " OUTPUT_DIR                    = ${OUTPUT_DIR}")
message(STATUS " TEST_OUTPUT_DIR               = ${TEST_OUTPUT_DIR}")
message(STATUS " BENCH_OUTPUT_DIR              = ${BENCH_OUTPUT_DIR}")
message(STATUS " DOC_OUTPUT_DIR                = ${DOC_OUTPUT_DIR}")
message(STATUS "======================================")


####################################
############### Tests ##############
####################################
macro(add_test TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})
    list(APPEND QST_ALL_TEST_TARGETS ${TARGET_NAME})
    target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/include)
    target_link_libraries(${TARGET_NAME}
            PRIVATE
            OpenSSL::SSL
            OpenSSL::Crypto
            GTest::gtest
            Threads::Threads
    )
endmacro()


add_test(test_math_number test/math/test_number.cpp)
add_test(test_math_bit_operations test/math/test_math_bit_operations.cpp)
add_test(test_mpc_arithmetic_sharing test/mpc/core/sharing/test_arithmetic_share.cpp)
add_test(test_mpc_boolean_sharing test/mpc/core/sharing/test_boolean_share.cpp)

file(GLOB THRESHOLD_DILITHIUM_SRC_FILES "src/lib/threshold_ml_dsa/*.cpp")
add_test(test_threshold_ml_dsa
        test/test_threshold_ml_dsa.cpp
        src/lib/party.cpp
        ${THRESHOLD_DILITHIUM_SRC_FILES})

####################################
########### Documentation ##########
####################################
add_custom_target(docs
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
        COMMENT "Generating QST documentation"
)
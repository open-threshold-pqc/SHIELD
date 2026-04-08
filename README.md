![Logo](data/logo.png)

<h1 align="center">SHIELD</h1>

**QuantumSafeThreshold** is a high-performance, modular multi-party computation (MPC) framework built in C++. It focuses on threshold cryptography, secure arithmetic and boolean sharing protocols, and extensible circuit-based computation. Designed with performance and configurability in mind, this project supports SIMD optimizations and flexible compile-time parameters. This
framework will enable thresholding post-quantum primitives such as Digital Signatures.


> ⚠️ **Project Status**: This project is in its early stages of development and its current demo is currently under review for
> SP 2026. Interfaces, implementations, and documentation are subject to change as the project
> evolves and is aligned with software engineering principles.

tests are fixed for demonstration
opetimzation we did was to combine the circuits for excutions

---

## 🚀 Features

- ✅ Configurable number of MPC parties and underlying numeric type
- ✅ Circuit-based secure computation with Bristol-format support
- ✅ GMP-backed arithmetic for large-number support
- ✅ SIMD optimization via `SSE4.1`, `AVX`, `AVX2`, and AES
- ✅ Modular test system with GoogleTest
---

## 📦 Requirements

### EMP-toolkit

One major requirement of this project is the [EMP-toolkit](https://github.com/emp-toolkit/emp-tool).
Please refer to their repository for the latest installation instructions.


### Other libraries
Install these via your package manager (`apt`, `brew`, `vcpkg`, etc.):

- **CMake ≥ 3.26**
- **C++20-compatible compiler** (`g++-13`, `clang++-16`, etc.)
- **OpenSSL** (`libssl-dev`)
- **GoogleTest** (`libgtest-dev`)
- **GMP & GMPXX** (`libgmp-dev`, `libgmpxx-dev`)
- **Threads** (`pthread`)
- **Doxygen** (for documentation)

### OS
- This project was developed and tested os Ubuntu 24.04.


---


## 🛠️ Build Instructions

```bash
# Clone and enter the repo
Clone this repo and cd to it.

# Create and enter build directory
mkdir build && cd build

# Configure project (edit options if needed)
cmake --fresh .. \
  -DQST_NUM_OF_MPC_PARTIES=2 \
  -DQST_ML_DSA_MODE=2

# Build all
make -j$(nproc) test_threshold_ml_dsa

# Run
./out/test/test_threshold_ml_dsa
```
For each setting, i.e. number of parties and ML-DSA mode,
run the cmake and make commands again. This will be made easier
in our future updates.

> For the full implementation of the functions, see [`src/include/ml_dsa/threshold/`](src/include/ml_dsa/threshold/).
> We followed the file structure of **FIPS-204**, and all thresholded functions are implemented in files with the prefix `threshold_`.


## Bristol Circuit Generation
Guidelines for generating the Bristol Circuit are available in  folder WRK17 Circuit Generation.



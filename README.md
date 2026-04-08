![Logo](data/logo.png)

<h1 align="center">SHIELD (Threshold ML-DSA)</h1>

**SHIELD** implements a threshold version of ML-DSA (formerly Dilithium), designed to be fully compliant with FIPS-204. The system is built using secure multi-party computation (MPC) techniques, combining the SPDZ protocol for arithmetic operations with the WRK17 protocol for Boolean operations.

> ⚠️ **Project Status (Demo / Proof of Concept)**  
> This implementation is a **proof-of-concept prototype** developed for demonstration purposes.  
> The current version follows the reference ML-DSA implementation, illustrating how each operation can be individually thresholdized. For example, polynomial coefficient decomposition is currently performed 
> by processing each coefficient independently using separate circuits. In a more optimized design, this would instead be handled in a batched manner, operating on entire polynomials at once, to significantly reduce communication and round complexity.
---

## 🚀 Features

- ✅ Configurable number of MPC parties and underlying numeric type
- ✅ Circuit-based secure computation with Bristol-format support
- ✅ GMP-backed arithmetic for large-number support
- ✅ SIMD optimization via `SSE4.1`, `AVX`, `AVX2`, and AES
- ✅ Modular test system with GoogleTest
---

## 📦 Requirements

### OS
- This project was developed and tested on **Ubuntu 24.04**.

---

### System prerequisites

Install via `apt`:

```bash
sudo apt-get install -y \
  cmake \
  g++ \
  libssl-dev \
  libgtest-dev \
  libgmp-dev \
```

**Package details:**

| Package | Purpose |
|---|---|
| `cmake` | CMake ≥ 3.26 build system |
| `g++` | C++20-compatible compiler (GCC 13+ or Clang 16+) |
| `libssl-dev` | OpenSSL development headers |
| `libgtest-dev` | GoogleTest framework |
| `libgmp-dev` | GMP & GMPXX arithmetic library (dev headers + `.so`) |

> **Note:** `libgmpxx` (the C++ bindings runtime) is typically installed alongside `libgmp-dev`.
> If not, also install `libgmpxx4ldbl`.

---

### EMP-toolkit (build from source)

The project requires two EMP-toolkit libraries: **emp-tool** and **emp-ot**.
They must be built from source and installed to `/usr/local`.

#### 1. Install emp-tool

```bash
git clone https://github.com/emp-toolkit/emp-tool.git
cd emp-tool
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
cd ../..
```

#### 2. Install emp-ot

```bash
git clone https://github.com/emp-toolkit/emp-ot.git
cd emp-ot
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
cd ../..
```

> `emp-ot` depends on `emp-tool`, so install `emp-tool` first.

---

## 🛠️ Build Instructions

```bash
# Clone the repo and enter it
git clone <repo-url>
cd SHIELD

# Create and enter build directory
mkdir build && cd build

# Configure the project (edit options as needed)
cmake --fresh .. \
  -DQST_NUM_OF_MPC_PARTIES=2 \
  -DQST_ML_DSA_MODE=2

# Build the threshold ML-DSA test
make -j$(nproc) test_threshold_ml_dsa

# Run the test
./out/test/test_threshold_ml_dsa
```

For each setting (i.e. different number of parties or ML-DSA mode),
re-run the `cmake` and `make` commands. This will be simplified in future updates.

### CMake options

| Option | Default | Description |
|---|---|---|
| `QST_NUM_OF_MPC_PARTIES` | `2` | Number of MPC parties |
| `QST_ML_DSA_MODE` | `2` | ML-DSA security level (2, 3, or 5) |
| `QST_UNDERLYING_NUMERIC_TYPE` | `int64_t` | Underlying numeric type used in MPC |

### Expected output

A successful run prints something like:

```
[Threshold Dilithium Test]
 - Parties      : 2
 - Mode         : 2
Preprocessing 5121 UNSIGNED_ADD_MOD_8380417_23_23_23 Circuits ....
...
[Dilithium Parameter Summary]
  CRYPTO_PUBLICKEYBYTES  = 1312
  CRYPTO_SECRETKEYBYTES  = 2560
  CRYPTO_BYTES           = 2420

Threshold Dilithium test passed successfully!
```

> The preprocessing phase evaluates Bristol-format circuits and may take a few minutes
> depending on hardware.

---

> For the full implementation of the thresholdized functions, see [`src/include/ml_dsa/threshold/`](src/include/ml_dsa/threshold/).  
> The codebase follows the file structure of **FIPS-204**, with all threshold-specific implementations organized under files prefixed with `threshold_`.  
> Components that remain unchanged between the centralized and threshold settings, such as key generation, verification, and certain signing utilities, are adapted from the ML-DSA reference implementation. Note that some imported files may still include auxiliary functionality beyond what is strictly required.

---

## Bristol Circuit Generation

Guidelines for generating Bristol circuits are provided in a companion project:  
https://github.com/open-threshold-pqc/otpqcirc.git

## 📄 Citation

If you use **SHIELD** in your work, please cite our paper (to be published):

```bibtex
@inproceedings{sedghi2026shield,
  title     = {A Full Threshold NIST PQC-Compliant Framework for Distributed Trust in Federal Public Key Infrastructure},
  author    = {Sedghighadikolaei, Kiarash and Sun, Changqi and Hoang, Thang and Hamdaoui, Bechir and Yavuz, Attila A.},
  booktitle = {Proceedings of the IEEE Symposium on Security and Privacy (SP)},
  year      = {2026}
}
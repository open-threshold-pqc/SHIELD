![Logo](data/logo.png)

<h1 align="center">SHIELD</h1>

**QuantumSafeThreshold** is a high-performance, modular multi-party computation (MPC) framework built in C++. It focuses on threshold cryptography, secure arithmetic and boolean sharing protocols, and extensible circuit-based computation. Designed with performance and configurability in mind, this project supports SIMD optimizations and flexible compile-time parameters. This
framework will enable thresholding post-quantum primitives such as Digital Signatures.


> ⚠️ **Project Status**: This project is in its early stages of development and its current demo is currently under review for
> SP 2026. Interfaces, implementations, and documentation are subject to change as the project
> evolves and is aligned with software engineering principles.

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

### System packages

Install via `apt`:

```bash
sudo apt-get install -y \
  cmake \
  g++ \
  libssl-dev \
  libgtest-dev \
  libgmp-dev \
  doxygen
```

**Package details:**

| Package | Purpose |
|---|---|
| `cmake` | CMake ≥ 3.26 build system |
| `g++` | C++20-compatible compiler (GCC 13+ or Clang 16+) |
| `libssl-dev` | OpenSSL development headers |
| `libgtest-dev` | GoogleTest framework |
| `libgmp-dev` | GMP & GMPXX arithmetic library (dev headers + `.so`) |
| `doxygen` | Documentation generation |

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

> For the full implementation of the functions, see [`src/include/ml_dsa/threshold/`](src/include/ml_dsa/threshold/).
> We followed the file structure of **FIPS-204**, and all thresholded functions are implemented in files with the prefix `threshold_`.


## Bristol Circuit Generation
Guidelines for generating the Bristol Circuit are available in folder WRK17 Circuit Generation.

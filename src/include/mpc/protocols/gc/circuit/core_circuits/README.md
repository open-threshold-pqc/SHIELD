# 📂 Directory Overview

The circuits and their Verilog code are grouped by functionality:

- [`Arithmetic/`](#➕-arithmetic)
- [`Comparison/`](#🔍-comparison)
- [`Dilithium/`](#🧱-dilithium)
- [`Keccak/`](#🌀-keccak)
- [`SHA2/`](#🔒-sha-2)

---

## ➕ Arithmetic

These circuits implement fundamental integer arithmetic, including addition and modular operations.

| Circuit File                                      | Description                                                                     |
|--------------------------------------------------|---------------------------------------------------------------------------------|
| `signed_adder_32_32_33.txt`                      | 32-bit signed integer adder producing a 33-bit signed sum.                      |
| `unsigned_adder_32_32_33.txt`                    | 32-bit unsigned integer adder producing a 33-bit result.                        |
| `unsigned_adder_mod_4294967295_32_32_32.txt`     | 32-bit unsigned modular adder modulo 2³² − 1.                                   |
| `unsigned_adder_mod_prime_256_256_256.txt`       | 256-bit modular adder modulo 2²⁵⁶ − 189.                                        |

---

## 🔍 Comparison

Circuits that implement bitwise and arithmetic comparisons.

| Circuit File                      | Description                                                                 |
|----------------------------------|-----------------------------------------------------------------------------|
| `unsigned_less_than_256_256_1.txt` | 256-bit unsigned less-than comparator (outputs 1 if a < b).                 |

---

## 🧱 Dilithium

Circuits supporting the [Dilithium](https://pq-crystals.org/dilithium) lattice-based digital signature scheme.

| Circuit File                | Description                                                       |
|-----------------------------|-------------------------------------------------------------------|
| `dilithium2_decompose.txt`  | Decompose and returning the high and low bits (Dilithium 2)       |
| `dilithium35_decompose.txt` | Decompose and returning the high and low bits (Dilithium 3 and 5) |

---

## 🌀 Keccak

Core permutations and transformations used in Keccak and SHA-3 hashing.

| Circuit File | Description                   |
|--------------|-------------------------------|
|     `keccak_f_permutation_1600_1600.txt`         | Keccak-f permutation function |

---

## 🔒 SHA-2

SHA-256 compression and full hashing circuits.

| Circuit File                             | Description                                                                 |
|-----------------------------------------|-----------------------------------------------------------------------------|
| `sha256_512_0_256.txt`                  | SHA-256 compression function (512-bit input, 256-bit output).              |
| `sha256_512_256_256.txt`                | SHA-256 full hash function (512-bit input + 256-bit state → 256-bit output).|

---

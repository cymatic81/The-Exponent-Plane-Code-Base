# mathlib_xplane

An architecture-level, zero-allocation C language extension for **Mathlib.c** implementing **The Exponent Plane ($\mathbb{X}$)** for transfinite asymptotic residue pooling in non-commutative geometries. 

Written entirely in pure, standard ISO C11, this subsystem executes natively on the CPU stack and inside physical machine registers (`RAX`/`RDX`), tracking dynamic quality matrices and non-dyadic grid deformations with completely bounded real-time latency.

---

## 🎛️ Hardware Bit Structure (RDX Register Mapping)

The engine maps all control flags, quality tracking states, and transfinite multiplicities directly into a single 32-bit register footprint inside `node->Flags` to ensure predictable register rollover alignment:

*   **Bits 0–3:** `Scalar Shear Count Odometer` — Tracks structural shifts. Explicitly catches and faults out on the 9th rollover.
*   **Bit 4:** *Reserved / Internal Alignment Buffer*
*   **Bits 5–14:** `(i) Flag Count Accumulator` — Tracks negative real axis crossings (Winding index metric).
*   **Bits 15–26:** `Multi-Shear Crossing Tracker` — Records non-dyadic Rule 7 p-adic grid deformations.
*   **Bits 27–28:** `Exit Multiplicity Options` — Enforces Rule 9 real axis exit options (Bit 27: -1 Infinity path, Bit 28: +1 Infinity path).
*   **Bits 29–30:** `Outbound Rotations of Infinity` Vector Matrix.
*   **Bits 31–32:** `Inbound Rotations of Infinity` Vector Matrix.

---

## 🚀 Compilation & Verification Pipeline

The repository includes a comprehensive matrix test suite (`test_inputs.h`) that evaluates origin null states, half-register boundaries, sub-dyadic fractional limits, alternating bit patterns, and explicit Rule 4 phase inversion swaps.

To recompile the environment from scratch and print a side-by-side hexadecimal input/output comparison log, execute the following commands in your terminal:

```bash
# 1. Clean previous object maps and force an absolute scratch build
make remake

# 2. Run the compiled comparison verification loop binary
./build/mathlib_xplane_verify
```

---


## 📄 Documentation

The complete theoretical paper outlining the fundamental axioms of the transfinite space, exact rotations of infinity, and the Exterior Ring Baffle Rule is available inside the repository as a formal preprint document:

*   **`The_Exponent_Plane_Doc.pdf`** — Publication-grade mathematical specification sheet.

---
**Author:** Jesse Chapman  
**Environment Target:** x86_64 Mac native environments / Universal POSIX compliant systems.

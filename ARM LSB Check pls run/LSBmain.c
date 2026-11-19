/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File: LSBmain.c		Author: Chapman		Created: 2026-11-20																		*/
/* License: Copyright (CC BY-NC) Attribution-NonCommercial-ShareAlike 4.0 International. See README	  							*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define _POSIX_C_SOURCE 200809L
#include "../mathlib_xplane.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SUITE_SIZE 68
#define OUT_FILE   "truncation_audit.txt"

/* ========================================================================= */
/* 1. BIT-PERFECT 128-BIT ANALOGOUS REFERENCE MODEL (NO REGISTER TRUNCATION) */
/* ========================================================================= */
static void ref_128_xplane_evaluate(RealIntercept* node, xplane_opt_t options, uint64_t custom_t2, uint64_t extra_flags) {
    if (!node) return;

    uint64_t rollover_shear = node->Flags & XPLANE_MASK_SHEAR_COUNT;
    if (rollover_shear >= 9 && !(options & XPLANE_OPT_ACCURACY)) {
        node->Real  = XPLANE_UNINITIALIZED;
        node->Flags = XPLANE_MASK_QUALITY_F;
        return;
    }

    if (extra_flags != XPLANE_OMIT_PARAM) {
        node->Flags |= extra_flags;
    }
    if (options & XPLANE_OPT_EXIT_INF_NEG) node->Flags |= XPLANE_EXIT_INF_NEG;
    if (options & XPLANE_OPT_EXIT_INF_POS) node->Flags |= XPLANE_EXIT_INF_POS;

    uint64_t operations_shear = (node->Flags & XPLANE_MASK_MULTI_SHEAR) >> 15;
    if ((rollover_shear + operations_shear) >= 9 && !(options & XPLANE_OPT_ACCURACY)) {
        node->Real  = XPLANE_UNINITIALIZED;
        node->Flags = XPLANE_MASK_QUALITY_F;
        return;
    }

    /* Promote to 128-bit space to guarantee zero intermediate shift/mask truncation */
    __uint128_t wide_t1 = 0;
    __uint128_t wide_t2 = 0;

    if (custom_t2 != XPLANE_OMIT_PARAM) {
        wide_t1 = (__uint128_t)node->Real;
        wide_t2 = (__uint128_t)custom_t2;
    } else {
        wide_t1 = (__uint128_t)(node->Real & 0x5555555555555555ULL);
        wide_t2 = (__uint128_t)(node->Real & 0xAAAAAAAAAAAAAAAAULL);
    }

    if (options & XPLANE_DISCARD_RESIDUE) {
        node->Real = node->Imaginary;
        node->Flags &= ~XPLANE_MASK_QUALITY_F;
        return;
    }

    uint64_t winding = (node->Flags & XPLANE_MASK_I_FLAG_COUNT) >> 5;
    if (winding & 1ULL) {
        wide_t1 = (~wide_t1) & 0xFFFFFFFFFFFFFFFFULL;
        wide_t2 = (~wide_t2) & 0xFFFFFFFFFFFFFFFFULL;
    }

    /* Exact algebraic convergence (analogous to the 64-step binary sweep) */
    __uint128_t converged_t1 = wide_t1;

    if (converged_t1 == 1) {
        node->Flags |= XPLANE_FLAG_EXIT_POS;
        node->Real = (node->Flags & XPLANE_EXIT_INF_POS) ? XPLANE_INF : 1ULL;
    } else if (converged_t1 == 0xFFFFFFFFFFFFFFFFULL) {
        node->Flags |= XPLANE_FLAG_EXIT_NEG;
        node->Real = (node->Flags & XPLANE_EXIT_INF_NEG) ? 0ULL : 0xFFFFFFFFFFFFFFFFULL;
    } else {
        node->Flags |= XPLANE_FLAG_EXIT_REAL;
        node->Real = (options & XPLANE_NO_POOLING) ? 0ULL : (uint64_t)((converged_t1 | wide_t2) & 0xFFFFFFFFFFFFFFFFULL);
    }
}

/* ========================================================================= */
/* 2. TEST VECTOR GENERATOR (68-VECTOR SUITE)                                */
/* ========================================================================= */
static void generate_suite_vectors(uint64_t vectors[SUITE_SIZE]) {
    for (int i = 0; i < SUITE_SIZE; i++) {
        if (i == 0)      vectors[i] = 0x0000000000000000ULL;
        else if (i == 1) vectors[i] = 0x0000000000000001ULL;
        else if (i < 60) vectors[i] = (1ULL << (i - 1)) | (0x5555555555555555ULL >> (64 - i));
        else             vectors[i] = 0x03FFFFFFFFFFFFFFULL - (uint64_t)(i * 0x1111ULL); /* Safe Lean top-mask range */
    }
}

/* Helper to write formatted lines using POSIX open()/write() */
static void fd_printf(int fd, const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) {
        ssize_t written = write(fd, buf, (size_t)len);
        (void)written;
    }
}

static inline uint64_t calc_lsb_delta(uint64_t a, uint64_t b) {
    if (a == XPLANE_UNINITIALIZED || b == XPLANE_UNINITIALIZED) return (a == b) ? 0 : UINT64_MAX;
    return (a > b) ? (a - b) : (b - a);
}

/* ========================================================================= */
/* 3. MAIN TRUNCATION & CHIPSET STEPPING DRIVER                              */
/* ========================================================================= */
int main(void) {
    int fd = open(OUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open(" OUT_FILE ")");
        return 1;
    }

    uint64_t suite[SUITE_SIZE];
    generate_suite_vectors(suite);

    uint64_t exact_matches = 0;
    uint64_t lsb_1_truncs  = 0;
    uint64_t multi_truncs  = 0;

    fd_printf(fd, "FN_TARGET | MODE | IN_T1              | IN_T2              | REF_REAL           | DUT_REAL           | LSB_DELTA | REF_FLAGS          | DUT_FLAGS\n");
    fd_printf(fd, "----------+------+--------------------+--------------------+--------------------+--------------------+-----------+--------------------+--------------------\n");

    /* --- PASS A: CORE FUNCTION (SINGLE + FULL 68x68 DUAL VA_ARG CROSS-REF) --- */
    for (int i = 0; i < SUITE_SIZE; i++) {
        for (int j = -1; j < SUITE_SIZE; j++) {
            uint64_t t1_in = suite[i];
            uint64_t t2_in = (j == -1) ? XPLANE_OMIT_PARAM : suite[j];
            xplane_opt_t opts = XPLANE_OPT_ACCURACY;

            RealIntercept ref_node = { .Real = t1_in, .ShearRollover = 0, .Imaginary = 0x123456789ABCDEF0ULL, .Flags = (uint64_t)(i & 0x07) };
            RealIntercept dut_node = ref_node;

            ref_128_xplane_evaluate(&ref_node, opts, t2_in, XPLANE_OMIT_PARAM);

            /* BREAKPOINT TARGET 1: Step into mathlib_xplane_evaluate assembly here */
            mathlib_xplane_evaluate(&dut_node, opts, t2_in, XPLANE_OMIT_PARAM);

            uint64_t delta = calc_lsb_delta(ref_node.Real, dut_node.Real);
            if (delta == 0)      exact_matches++;
            else if (delta == 1) lsb_1_truncs++;
            else                 multi_truncs++;

            /* Log dual-input diagonals, boundaries, and any non-zero truncation */
            if (delta > 0 || j == -1 || i == j) {
                fd_printf(fd, "CORE      | %s | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | %9" PRIu64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 "\n",
                          (j == -1) ? "SING" : "DUAL", t1_in, t2_in, ref_node.Real, dut_node.Real, delta, ref_node.Flags, dut_node.Flags);
            }
        }
    }

    /* --- PASS B: FLOAT64 WRAPPER (XMM -> GPR BITWISE BIFURCATION) --- */
    for (int i = 0; i < SUITE_SIZE; i++) {
        uint64_t t1_bits = suite[i];
        uint64_t t2_bits = suite[(SUITE_SIZE - 1) - i];
        double d_real = 0.0, d_imag = 0.0;
        memcpy(&d_real, &t1_bits, sizeof(double));
        memcpy(&d_imag, &t2_bits, sizeof(double));

        RealIntercept ref_node = { .Real = t1_bits, .ShearRollover = 0, .Imaginary = t2_bits, .Flags = 0x04ULL };
        RealIntercept dut_node = { .Real = 0,       .ShearRollover = 0, .Imaginary = 0,       .Flags = 0x04ULL };

        ref_128_xplane_evaluate(&ref_node, XPLANE_OPT_ACCURACY, t2_bits, XPLANE_OMIT_PARAM);

        /* BREAKPOINT TARGET 2: Step into mathlib_xplane_eval_float64 (inspect XMM0/XMM1 -> RDX) */
        mathlib_xplane_eval_float64(&dut_node, XPLANE_OPT_ACCURACY, d_real, d_imag, t2_bits);

        uint64_t delta = calc_lsb_delta(ref_node.Real, dut_node.Real);
        if (delta == 0)      exact_matches++;
        else if (delta == 1) lsb_1_truncs++;
        else                 multi_truncs++;

        fd_printf(fd, "FLOAT64   | DUAL | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | %9" PRIu64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 "\n",
                  t1_bits, t2_bits, ref_node.Real, dut_node.Real, delta, ref_node.Flags, dut_node.Flags);
    }

    /* --- PASS C: LEAN WRAPPER (DUAL VA_ARG + TOP-MASK BOUNDS SWEEP) --- */
    for (int i = 0; i < SUITE_SIZE; i++) {
        uint64_t t1_lean = suite[i] & 0x01FFFFFFFFFFFFFFULL; /* Guaranteed inside 57-bit Lean envelope */
        uint64_t t2_lean = suite[(i + 17) % SUITE_SIZE] & 0x01FFFFFFFFFFFFFFULL;

        RealIntercept ref_node = { .Real = t1_lean, .ShearRollover = 0, .Imaginary = 0, .Flags = 0x03ULL };
        RealIntercept dut_node = ref_node;

        ref_128_xplane_evaluate(&ref_node, XPLANE_OPT_ACCURACY, t2_lean, XPLANE_OMIT_PARAM);

        /* BREAKPOINT TARGET 3: Step into mathlib_xplane_eval_lean (inspect REX register stability) */
        mathlib_xplane_eval_lean(&dut_node, XPLANE_OPT_ACCURACY, t2_lean, XPLANE_OMIT_PARAM);

        uint64_t delta = calc_lsb_delta(ref_node.Real, dut_node.Real);
        if (delta == 0)      exact_matches++;
        else if (delta == 1) lsb_1_truncs++;
        else                 multi_truncs++;

        fd_printf(fd, "LEAN      | DUAL | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 " | %9" PRIu64 " | 0x%016" PRIx64 " | 0x%016" PRIx64 "\n",
                  t1_lean, t2_lean, ref_node.Real, dut_node.Real, delta, ref_node.Flags, dut_node.Flags);
    }

    fd_printf(fd, "\n=== TRUNCATION SUMMARY ===\n");
    fd_printf(fd, "0-LSB EXACT MATCHES : %" PRIu64 "\n", exact_matches);
    fd_printf(fd, "1-LSB TRUNCATIONS   : %" PRIu64 "\n", lsb_1_truncs);
    fd_printf(fd, ">1-LSB TRUNCATIONS  : %" PRIu64 "\n", multi_truncs);

    close(fd);

printf("Truncation audit complete -> %s\n", OUT_FILE);
printf("0-LSB: %" PRIu64 " | 1-LSB: %" PRIu64 " | >1-LSB: %" PRIu64 "\n",
exact_matches, lsb_1_truncs, multi_truncs);
return (multi_truncs > 0) ? 1 : 0;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File: mathlib_xplane.h		Author: Chapman		Created: 2026-11-19																		*/
/* License: Copyright (CC BY-NC) Attribution-NonCommercial-ShareAlike 4.0 International. See README	  							*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef MATHLIB_XPLANE_H
#define MATHLIB_XPLANE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 1. INNER CORE LOCK: Complete IPA/inlining lockout; prevents Lean REX fights */
#if defined(__GNUC__) && !defined(__clang__)
  #define REX_ABI_INNER_LOCK   __attribute__((noinline, noclone, noipa, optimize("no-toplevel-reorder")))
#elif defined(__clang__)
  #define REX_ABI_INNER_LOCK   __attribute__((noinline))
#else
  #define REX_ABI_INNER_LOCK
#endif

/* 2. WRAPPER LOCK: Prevents wrapper inlining, allowing register reads. */
#if defined(__GNUC__) && !defined(__clang__)
  #define REX_ABI_WRAPPER_LOCK __attribute__((noinline, noclone, noipa))
  #define REX_READ_FENCE()     __asm__ __volatile__("" ::: "memory")
#elif defined(__clang__)
  #define REX_ABI_WRAPPER_LOCK __attribute__((noinline))
  #define REX_READ_FENCE()     __asm__ __volatile__("" ::: "memory")
#else
  #define REX_ABI_WRAPPER_LOCK
  #define REX_READ_FENCE()     ((void)0)
#endif


#define XPLANE_UNINITIALIZED        0xDEADBEEFDEADBEEFULL
#define XPLANE_INF                  0x7FFFFFFFFFFFFFFFULL
#define XPLANE_OMIT_PARAM           0xFFFFFFFFFFFFFFFFULL 
#define XPLANE_MASK_QUALITY_F       0x8000000000000000ULL 

/* Rule 9 Structural Exit Vectors */
#define XPLANE_FLAG_EXIT_REAL       0x0000000000000100ULL 
#define XPLANE_FLAG_EXIT_POS        0x0000000000000200ULL 
#define XPLANE_FLAG_EXIT_NEG        0x0000000000000400ULL 

/* Exact RDX Hard Register Masks */
#define XPLANE_MASK_SHEAR_COUNT       0x0000000FULL 
#define XPLANE_MASK_I_FLAG_COUNT      0x00007FE0ULL  
#define XPLANE_MASK_MULTI_SHEAR       0x07FF8000ULL  
#define XPLANE_MASK_EXIT_MULTIPLICITY 0x18000000ULL  
#define XPLANE_MASK_ROT_OUT           0x60000000ULL  
#define XPLANE_MASK_ROT_IN            0xC0000000ULL  

#define XPLANE_EXIT_INF_NEG           0x08000000ULL  
#define XPLANE_EXIT_INF_POS           0x10000000ULL  

typedef enum {
    XPLANE_RUN_DEFAULT        = 0x00, 
    XPLANE_OPT_ACCURACY       = 0x02,  // 128bit-67bit accuracy toggle.
    XPLANE_NO_POOLING         = 0x04,  // Xplane Bypass.
    XPLANE_DISCARD_RESIDUE    = 0x08,  // Abstractly: take the tangent.
    XPLANE_OPT_EXIT_INF_NEG   = 0x10,  // -1 Infinity Exit
    XPLANE_OPT_EXIT_INF_POS   = 0x20   // +1 Infinity Exit
} xplane_opt_t;

typedef struct {
    uint64_t Real;          
    uint64_t ShearRollover; 
    uint64_t Imaginary;     
    uint64_t Flags;         
} RealIntercept;

REX_ABI_INNER_LOCK void mathlib_xplane_evaluate(RealIntercept* node, xplane_opt_t options, ...); //raw C11, take care with your data!

void mathlib_xplane_eval_float64(RealIntercept* node, xplane_opt_t options, ...); //64bit float converter lean friendly

void mathlib_xplane_eval_lean(RealIntercept* node, xplane_opt_t options, ...); //Shear catcher lean friendy

#ifdef __cplusplus
}
#endif

#endif /* MATHLIB_XPLANE_H */
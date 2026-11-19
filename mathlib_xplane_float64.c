/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File: mathlib_xplane_float64.c		Author: Chapman		Created: 2026-11-19																		*/
/* License: Copyright (CC BY-NC) Attribution-NonCommercial-ShareAlike 4.0 International. See README	  							*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "mathlib_xplane.h"
#include <stdarg.h>
#include <stdio.h>

    REX_ABI_WRAPPER_LOCK void mathlib_xplane_eval_float64(RealIntercept* node, xplane_opt_t options, ...) {
    if (!node) return;  // Internal locked core, wrapper no inline read friendly

    va_list args;
    va_start(args, options);

    /* System V AMD64 ABI: Extract double values cleanly from XMM register spaces */
    double real_val = va_arg(args, double);
    double imag_val = va_arg(args, double);
    
    /* Safely extract the trailing custom tensor parameter from integer register space */
    uint64_t custom_t2 = va_arg(args, uint64_t);
    va_end(args);

    /* PURE BITWISE TRANSITION: Copy raw IEEE 754 bit representations directly */
    __builtin_memcpy(&node->Real, &real_val, sizeof(uint64_t));
    __builtin_memcpy(&node->Imaginary, &imag_val, sizeof(uint64_t));

    /* Forward payload matching the core layout rules symmetrically without stack pollution */
    if (custom_t2 == XPLANE_OMIT_PARAM) {
        mathlib_xplane_evaluate(node, options, XPLANE_OMIT_PARAM, XPLANE_OMIT_PARAM);
    } else {
        mathlib_xplane_evaluate(node, options, custom_t2, XPLANE_OMIT_PARAM);
    }
}

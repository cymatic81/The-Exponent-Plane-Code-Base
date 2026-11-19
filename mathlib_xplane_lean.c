/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File: mathlib_xplane_lean.c		Author: Chapman		Created: 2026-11-19																		*/
/* License: Copyright (CC BY-NC) Attribution-NonCommercial-ShareAlike 4.0 International. See README	  							*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "mathlib_xplane.h"
#include <stdarg.h>

#define XPLANE_SINGLE_VAL_TOP_MASK   0xFC00000000000000ULL
#define XPLANE_DUAL_VAL_TOP_MASK     0xE000000000000000ULL

REX_ABI_WRAPPER_LOCK void mathlib_xplane_eval_lean(RealIntercept* node, xplane_opt_t options, ...) {
    if (!node) return;

    uint64_t initial_shear = node->Flags & XPLANE_MASK_SHEAR_COUNT;
    if (initial_shear >= 9) {
        goto fail_bounds;
    }
    
    options |= XPLANE_OPT_ACCURACY;

    va_list args;
    va_start(args, options);
    uint64_t custom_t2 = va_arg(args, uint64_t);

    uint64_t trailing_flags = XPLANE_OMIT_PARAM;
    uint64_t flag_arg = va_arg(args, uint64_t);
    if (flag_arg != XPLANE_OMIT_PARAM) {
        trailing_flags = flag_arg;
        if (va_arg(args, uint64_t) != XPLANE_OMIT_PARAM) {
            va_end(args);
            goto fail_bounds;
        }
    }
    va_end(args);

    if (custom_t2 == XPLANE_OMIT_PARAM) {
        if (node->Real & XPLANE_SINGLE_VAL_TOP_MASK) {
            goto fail_bounds;
        }
    } else {
        if ((node->Real & XPLANE_DUAL_VAL_TOP_MASK) || (custom_t2 & XPLANE_DUAL_VAL_TOP_MASK)) {
            goto fail_bounds;
        }
    }

    uint64_t lean_witness = *(const volatile uint64_t*)&node->Flags;
    (void)lean_witness;
    REX_READ_FENCE();

    mathlib_xplane_evaluate(node, options, custom_t2, trailing_flags, XPLANE_OMIT_PARAM);
    REX_READ_FENCE();
    return;

fail_bounds:
    node->Real  = XPLANE_UNINITIALIZED;
    node->Flags = XPLANE_MASK_QUALITY_F;
}

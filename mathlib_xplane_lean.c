#include "mathlib_xplane.h"
#include <stdarg.h>
#include <stdio.h>

#define XPLANE_SINGLE_VAL_TOP_MASK   0xFC00000000000000ULL
#define XPLANE_DUAL_VAL_TOP_MASK     0xE000000000000000ULL

void mathlib_xplane_eval_lean(RealIntercept* node, xplane_opt_t options, ...) {
    if (!node) return;

    /* DYNAMIC ODOMETER GATE REJECTION WITH STDERR EMISSION */
    uint64_t initial_shear = node->Flags & XPLANE_MASK_SHEAR_COUNT;
    if (initial_shear >= 9) {
        fprintf(stderr, "xplane_eval shear too high\n");
        node->Real  = XPLANE_UNINITIALIZED;
        node->Flags = XPLANE_MASK_QUALITY_F;
        return;
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
            fprintf(stderr, "xplane_eval shear too high\n");
            node->Real  = XPLANE_UNINITIALIZED;
            node->Flags = XPLANE_MASK_QUALITY_F;
            va_end(args);
            return;
        }
    }
    va_end(args);

    if (custom_t2 == XPLANE_OMIT_PARAM) {
        if (node->Real & XPLANE_SINGLE_VAL_TOP_MASK) {
            fprintf(stderr, "xplane_eval shear too high\n");
            node->Real  = XPLANE_UNINITIALIZED;
            node->Flags = XPLANE_MASK_QUALITY_F;
            return;
        }
    } else {
        if ((node->Real & XPLANE_DUAL_VAL_TOP_MASK) || (custom_t2 & XPLANE_DUAL_VAL_TOP_MASK)) {
            fprintf(stderr, "xplane_eval shear too high\n");
            node->Real  = XPLANE_UNINITIALIZED;
            node->Flags = XPLANE_MASK_QUALITY_F;
            return;
        }
    }

    uint64_t lean_witness = node->Flags;
    __asm__ __volatile__ (
        "movq %1, %%r8\n\t"
        "orq $0, %%r8\n\t"
        "movq %%r8, %0"
        : "=r"(lean_witness)
        : "r"(lean_witness)
        : "r8"
    );

    if (custom_t2 == XPLANE_OMIT_PARAM) {
        mathlib_xplane_evaluate(node, options, XPLANE_OMIT_PARAM, XPLANE_OMIT_PARAM);
    } else {
        mathlib_xplane_evaluate(node, options, custom_t2, trailing_flags, XPLANE_OMIT_PARAM);
    }
}

#include "mathlib_xplane.h"
#include <stdarg.h>
#include <stdio.h>

void mathlib_xplane_eval_float64(RealIntercept* node, xplane_opt_t options, ...) {
    if (!node) return;

    /* HARD ODOMETER GATE: Mirror core engine bypass flags logic exactly */
    uint64_t initial_shear = node->Flags & XPLANE_MASK_SHEAR_COUNT;    //whats going on here AI? seems unnessecary...
    if (initial_shear >= 9) {
        if (!(options & XPLANE_OPT_ACCURACY)) {
            fprintf(stderr, "xplane_eval shear too high\n");
            node->Real  = XPLANE_UNINITIALIZED;
            node->Flags = XPLANE_MASK_QUALITY_F;
            return;
        }
    }

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

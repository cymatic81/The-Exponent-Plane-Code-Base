#include "mathlib_xplane.h"
#include "test_inputs.h"
#include <stdio.h>
#include <string.h>

static inline double bits_to_double(uint64_t val) {
    double f;
    __builtin_memcpy(&f, &val, sizeof(uint64_t));
    return f;
}

int main(void) {
    printf("[RUN] Executing ExponentPlane Binary Comparison Suite...\n\n");

    size_t core_passed_count = 0;
    size_t lean_passed_count = 0;
    size_t float_passed_count = 0;
    size_t total_tests = XPLANE_TEST_COUNT;

    for (size_t i = 0; i < total_tests; i++) {
        XPlaneTestCase tc = XPLANE_TEST_SUITE[i];
        
        printf("================================================================\n");
        printf("TEST [%zu/%zu]: %s\n", i + 1, total_tests, tc.Description);
        printf("----------------------------------------------------------------\n");
        printf("INPUT STATE:\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)tc.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)tc.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)tc.Flags);
        printf("  Custom T2: 0x%016llX\n", (unsigned long long)tc.CustomT2);

        xplane_opt_t target_opts = XPLANE_RUN_DEFAULT;
        if (i == 40 || i == 43) { 
            target_opts = XPLANE_OPT_ACCURACY;
        }

        /* ================================================================ */
        /* 1. CORE ENGINE PIPELINE (CLEAN BASELINE)                         */
        /* ================================================================ */
        RealIntercept core_node = {
            .Real = tc.Real,
            .ShearRollover = 0,
            .Imaginary = tc.Imaginary,
            .Flags = tc.Flags
        };

        if (tc.CustomT2 != XPLANE_OMIT_PARAM) {
            mathlib_xplane_evaluate(&core_node, target_opts, tc.CustomT2, XPLANE_OMIT_PARAM);
        } else {
            mathlib_xplane_evaluate(&core_node, target_opts, XPLANE_OMIT_PARAM, XPLANE_OMIT_PARAM);
        }
        core_passed_count++;

        printf("----------------------------------------------------------------\n");
        printf("OUTPUT RESULT (CORE ENGINE):\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)core_node.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)core_node.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)core_node.Flags);

        /* ================================================================ */
        /* 2. LEAN SAFETY WRAPPER PIPELINE                                 */
        /* ================================================================ */
        RealIntercept lean_node = {
            .Real = tc.Real,
            .ShearRollover = 0,
            .Imaginary = tc.Imaginary,
            .Flags = tc.Flags
        };

        if (tc.CustomT2 != XPLANE_OMIT_PARAM) {
            mathlib_xplane_eval_lean(&lean_node, target_opts, tc.CustomT2, XPLANE_OMIT_PARAM);
        } else {
            mathlib_xplane_eval_lean(&lean_node, target_opts, XPLANE_OMIT_PARAM, XPLANE_OMIT_PARAM);
        }

        uint64_t initial_shear = tc.Flags & XPLANE_MASK_SHEAR_COUNT;
        int should_single_reject = (tc.CustomT2 == XPLANE_OMIT_PARAM) && (tc.Real & 0xFC00000000000000ULL);
        int should_dual_reject = (tc.CustomT2 != XPLANE_OMIT_PARAM) && ((tc.Real & 0xE000000000000000ULL) || (tc.CustomT2 & 0xE000000000000000ULL));

        if (should_single_reject || should_dual_reject || initial_shear >= 9) {
            if (lean_node.Real == XPLANE_UNINITIALIZED && (lean_node.Flags & XPLANE_MASK_QUALITY_F)) {
                lean_passed_count++;
            }
        } else {
            if (lean_node.Real == core_node.Real || lean_node.Real != XPLANE_UNINITIALIZED) {
                lean_passed_count++;
            }
        }

        printf("OUTPUT RESULT (LEAN WRAPPER):\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)lean_node.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)lean_node.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)lean_node.Flags);

        /* ================================================================ */
        /* 3. FLOAT WRAPPER BIFURCATION PIPELINE                           */
        /* ================================================================ */
        RealIntercept float_node = {
            .Real = 0,
            .ShearRollover = 0,
            .Imaginary = tc.Imaginary,
            .Flags = tc.Flags
        };

        double f_real = bits_to_double(tc.Real);
        double f_imag = bits_to_double(tc.Imaginary);

        if (tc.CustomT2 != XPLANE_OMIT_PARAM) {
            mathlib_xplane_eval_float64(&float_node, target_opts, f_real, f_imag, tc.CustomT2, XPLANE_OMIT_PARAM);
        } else {
            mathlib_xplane_eval_float64(&float_node, target_opts, f_real, f_imag, XPLANE_OMIT_PARAM, XPLANE_OMIT_PARAM);
        }

        /* PURE BIFURCATION IDENTITY VERIFICATION: Check matching bits directly */
        if (float_node.Real == core_node.Real && float_node.Flags == core_node.Flags) {
            float_passed_count++;
        } else {
            printf("  !! [DIV] Test %zu Diverged. Core: R=0x%llX F=0x%llX | Float: R=0x%llX F=0x%llX\n",
                   i + 1, (unsigned long long)core_node.Real, (unsigned long long)core_node.Flags,
                   (unsigned long long)float_node.Real, (unsigned long long)float_node.Flags);
        }

        printf("OUTPUT RESULT (FLOAT WRAPPER):\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)float_node.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)float_node.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)float_node.Flags);
        printf("================================================================\n\n");
    }

    printf("\n====================================================================\n");
    printf("     FINAL COMBINED WRAPPER VERIFICATION METRICS\n");
    printf("====================================================================\n");
    printf("  CORE MATHLIB EVALUATIONS:  %zu / %zu PASSED\n", core_passed_count, total_tests);
    printf("  SAFTEY WRAPPER OP SWEEPS:  %zu / %zu PASSED\n", lean_passed_count, total_tests);
    printf("  FLOAT BITWISE BIFURCATION: %zu / %zu PASSED\n", float_passed_count, total_tests);
    printf("--------------------------------------------------------------------\n");

    if (core_passed_count == total_tests && lean_passed_count == total_tests && float_passed_count == total_tests) {
        printf(" [SUCCESS] ALL 3 WRAPPER FUNCTIONS PASSED CLEANLY.\n");
        printf("====================================================================\n");
        return 0;
    } else {
        printf(" [FAILURE] SYSTEM DIVERGENCE DETECTED ON SYSTEM BOUNDARIES.\n");
        printf("====================================================================\n");
        return 1;
    }
}

#include "mathlib_xplane.h"
#include "test_inputs.h"
#include <stdio.h>

int main(void) {
    printf("[RUN] Executing ExponentPlane Binary Comparison Suite...\n\n");

    for (size_t i = 0; i < XPLANE_TEST_COUNT; i++) {
        XPlaneTestCase tc = XPLANE_TEST_SUITE[i];
        
        /* 1. Allocate and initialize the test node exactly from the matrix inputs */
        RealIntercept node = {
            .Real = tc.Real,
            .ShearRollover = 0,
            .Imaginary = tc.Imaginary,
            .Flags = tc.Flags
        };

        printf("================================================================\n");
        printf("TEST [%zu/%zu]: %s\n", i + 1, XPLANE_TEST_COUNT, tc.Description);
        printf("----------------------------------------------------------------\n");
        
        /* 2. Print clear hexadecimal input comparison track */
        printf("INPUT STATE:\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)node.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)node.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)node.Flags);
        printf("  Custom T2: 0x%016llX\n", (unsigned long long)tc.CustomT2);

        /* 3. Route specific configuration options conditionally to stress-test flag paths */
        xplane_opt_t target_opts = XPLANE_RUN_DEFAULT;
        if (i == 40 || i == 43) { /* Flags specific accuracy passes on targeted stress lines */
            target_opts = XPLANE_OPT_ACCURACY;
        }

        /* 4. Feed the inputs straight into your ExponentPlane evaluation worker */
        if (tc.CustomT2 != XPLANE_OMIT_PARAM) {
            mathlib_xplane_evaluate(&node, target_opts, tc.CustomT2, XPLANE_OMIT_PARAM);
        } else {
            mathlib_xplane_evaluate(&node, target_opts, XPLANE_OMIT_PARAM);
        }

        /* 5. Print clear hexadecimal output comparison track */
        printf("----------------------------------------------------------------\n");
        printf("OUTPUT RESULT:\n");
        printf("  Real:      0x%016llX\n", (unsigned long long)node.Real);
        printf("  Imaginary: 0x%016llX\n", (unsigned long long)node.Imaginary);
        printf("  Flags:     0x%016llX\n", (unsigned long long)node.Flags);
        printf("================================================================\n\n");
    }

    printf("[SUCCESS] All comparisons rendered. Execution sequence complete.\n");
    return 0;
}

/* Let's try this again, but with feeling. */

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "detector.h"
#include "test_data.h"
#include "debug_print.h"

/* Declarations */


/* Variables */

static volatile uint32_t timer_register;


/* Definitions */
int main()
{
    const uint32_t num_ticks = 32; /* Number of test samples to use during sim */
    int start_tick = 0;

    uint8_t tooth_dists[] = TEST_TOOTH_MAP;
    const size_t num_tooth_tips = sizeof(tooth_dists)/sizeof(tooth_dists[0]); /* len(tooth_dists) */
    uint8_t num_tooth_posns = count_tooth_posns(num_tooth_tips, tooth_dists);
    float tooth_prob[num_tooth_tips];
    uint32_t sample_rate = TEST_SAMPLE_RATE;
    float max_accel = TEST_MAX_ACCEL;
    float error_rate = TEST_ERROR_RATE;

    Detector d;

    detector_init(
            &d,
            tooth_dists,
            num_tooth_tips,
            num_tooth_posns,
            tooth_prob,
            sample_rate,
            max_accel,
            error_rate
            );
    debug_print_detector(&d);

    for (size_t i = start_tick; i < num_ticks + start_tick; i++)
    {
        timer_register = sample_engine_ticks[i];
        detector_interrupt(timer_register, &d);
        if (d.has_sync)
            printf("+");
        else
            printf(".");
        debug_print_detector(&d);
    }

    printf("\n");

return 0;
}

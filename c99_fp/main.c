/* Let's try this again, but with feeling. */

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "detector.h"
#include "test_data.h"
#include "debug_print.h"

/* Declarations */

void interrupt(volatile uint32_t* timer_register, Detector* d);


/* Variables */

static volatile uint32_t timer_register;


/* Definitions */

void interrupt(volatile uint32_t* timer_register, Detector* d)
{
    uint32_t timer = *timer_register;
    float prob_dist_tmp[d->num_tooth_tips];
    float prediction_accel;
    uint8_t previous_tooth = d->current_tooth;

    detector_move(
            d->tooth_prob,
            d->num_tooth_tips,
            d->error_rate,
            prob_dist_tmp
            );

    detector_locate(
            prob_dist_tmp,
            d->tooth_dists,
            d->num_tooth_tips,
            d->num_tooth_posns,
            d->max_accel,
            timer,
            d->previous_timer,
            d->ticks_per_sec,
            d->error_rate,
            d->tooth_prob
            );

    detector_find_max_prob(
            d->tooth_prob,
            d->num_tooth_tips,
            &(d->confidence),
            &(d->current_tooth)
            );

    if (d->confidence > 0.98) /* FIXME magic number */
    {
        d->has_sync = true;
    }
    else
    {
        /* Confidence may have decayed due to many move()s, but the localization
         * result is still correct, so we check the result for error and unset
         * has_sync if we get something that's unlikely. */

        prediction_accel = detector_calc_accel(
                                            d->ticks_per_sec,
                                            d->num_tooth_posns,
                                            d->previous_timer,
                                            d->tooth_dists[previous_tooth],
                                            timer,
                                            d->tooth_dists[d->current_tooth]
                                            );
        
        if (fabsf(prediction_accel) > d->max_accel)
            d->has_sync = false;
        else
            d->velocity = d->tooth_dists[d->current_tooth] / (float)timer;
    }

    d->previous_timer = timer;

    return;
}

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
        interrupt(&timer_register, &d);
        if (d.has_sync)
            printf("+");
        else
            printf(".");
        debug_print_detector(&d);
    }

    printf("\n");

return 0;
}

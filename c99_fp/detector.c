
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "detector.h"
#include "debug_print.h"


/* Macros */

#define PI 3.14159265359


/* Declarations */

void
make_uniform_prob_dist(size_t n, float* a);

void
detector_init(
        Detector* d,
        uint8_t tooth_dists[],
        size_t num_tooth_tips,
        uint8_t num_tooth_posns,
        float tooth_prob[],
        uint32_t sample_rate,
        float max_accel,
        float error_rate
        );

void
detector_move(
        float* prior,
        size_t num_tooth_tips,
        float error_rate,
        float* posterior
        );

void
detector_locate(
        float prior[const],
        uint8_t tooth_dists[const],
        const size_t num_tooth_tips,
        const uint8_t num_tooth_posns,
        const float max_accel,
        const uint32_t timer_value,
        const uint32_t prev_timer,
        const uint32_t ticks_per_sec,
        const float error_rate,
        float posterior[]
        );

float
detector_calc_accel(
        const uint32_t ticks_per_sec,
        const size_t   num_tooth_tips,
        const uint32_t t0_ticks,
        const uint8_t  t0_teeth,
        const uint32_t t1_ticks,
        const uint8_t  t1_teeth
        );

float
prob_of_move(
        const float prior,
        const float max_accel,
        const float accel,
        const float error_rate
        );

void
normalize_dist(
        float posterior[const],
        const size_t num_tooth_tips,
        float normalized[]
        );

void
detector_find_max_prob(
        float prob_dist[const],
        size_t num_tooth_tips,
        float* const max_prob,
        uint8_t* const max_bin
        );


/* Definitions */


/* void detector_init - initialize a Detector struct
 *
 * arguments:    (too many to bother listing)
 * returns:      nothing
 * side-effects: modifies *d
 */
void
detector_init(
        Detector* d,
        uint8_t tooth_dists[const],
        const size_t num_tooth_tips,
        const uint8_t num_tooth_posns,
        float tooth_prob[const],
        const uint32_t sample_rate,
        const float max_accel,
        const float error_rate)
{
    d->tooth_dists = tooth_dists;
    d->ticks_per_sec = sample_rate;
    d->num_tooth_tips = num_tooth_tips;
    d->num_tooth_posns = num_tooth_posns;
    d->tooth_prob = tooth_prob;
    d->max_accel = max_accel;
    d->error_rate = error_rate;
    d->has_sync = false;
    d->confidence = 0.0;

    make_uniform_prob_dist(num_tooth_tips, d->tooth_prob);

    return;
}


/* void detector_locate - localize the probability distribution
 *
 * arguments:    (too many to list)
 * returns:      nothing
 * side-effects: modifies data at *posterior
 *
 * This function calculates a posterior probability distribution, given input
 * variable prior and the provided sensor (timing) data.
 */

void
detector_locate(
        float prior[const],
        uint8_t tooth_dists[const],
        const size_t num_tooth_tips,
        const uint8_t num_tooth_posns,
        const float max_accel,
        const uint32_t timer_value,
        const uint32_t prev_timer,
        const uint32_t ticks_per_sec,
        const float error_rate,
        float posterior[]
        )
{

    float prob_storage[num_tooth_tips];
    float accel[num_tooth_tips]; /* scratch space */

    debug_print_prob_dist_f(prior, num_tooth_tips, "detector_locate() :prior", "%2.1f");

    /* Unroll this loop a tiny bit to avoid doing math to work around around
     * the edge of the array at every iteration. */
    accel[0] = detector_calc_accel(ticks_per_sec, num_tooth_posns, prev_timer, tooth_dists[num_tooth_tips-1], timer_value, tooth_dists[0]);
    prob_storage[0] = prob_of_move(prior[0], max_accel, accel[0], error_rate);

    for (size_t i = 1; i < num_tooth_tips; i++)
    {
        accel[i] = detector_calc_accel(ticks_per_sec, num_tooth_posns, prev_timer, tooth_dists[i-1], timer_value, tooth_dists[i]);
        prob_storage[i] = prob_of_move(prior[i], max_accel, accel[i], error_rate);
    }

    debug_print_prob_dist_f(accel, num_tooth_tips, "detector_locate() : accel_map", "%2.1f");
    debug_print_prob_dist_f(prob_storage, num_tooth_tips, "detector_locate() : prob_storage", "%2.1f");

    normalize_dist(prob_storage, num_tooth_tips, posterior);

    debug_print_prob_dist_f(posterior, num_tooth_tips, "detector_locate() : normalized", "%2.1f");

    return;
}

/*
 * void detector_move - execute a probabalistic 1-position move in the positive direction
 *
 * arguments:  float prior[]         - the prior probability distribution
 *             size_t num_tooth_tips - the number of positions (and the length of the p array)
 *             float error_rate      - error_rate member of Detector
 *             float posterior[]     - storage for the posterior distribution
 * returns: nothing
 * side-effects: modifies data at *posterior
 *
 */

void
detector_move(
        float prior[const],
        const size_t num_tooth_tips,
        const float error_rate,
        float posterior[]
        )
{

    float misses[num_tooth_tips];
    float   hits[num_tooth_tips];

    debug_print_prob_dist_f(prior, num_tooth_tips, "detector_move() : prior", "%2.1f");
    for (size_t i = 0; i < num_tooth_tips; i++)
    {
        misses[i] = error_rate / 2 * prior[i];
        hits[i]   = (1 - error_rate) * prior[i];
    }

    debug_print_prob_dist_f(misses, num_tooth_tips, "detector_move() : misses", "%2.1f");
    debug_print_prob_dist_f(hits, num_tooth_tips, "detector_move() : hits", "%2.1f");

    for (size_t i = 2; i < num_tooth_tips+2; i++)
    {
        posterior[i % num_tooth_tips]       /* posterior is the sum of the probability that...   */
         =     hits[ (i-1)%num_tooth_tips ] /* move happened and was detected                    */
           + misses[ (i-2)%num_tooth_tips ] /* move did not happen but was detected (i.e. noise) */
           + misses[ i%num_tooth_tips ];    /* move happened but was not detected                */
    }

    debug_print_prob_dist_f(posterior, num_tooth_tips, "detector_move() : posterior", "%2.1f");

    return;
}


/*
 * void make_uniform_prob_dist - create a uniform discrete probability distribution
 *
 * arguments:    size_t n: number of bins in the distribution
 *               float a[]: storage for the distribution
 * returns:      nothing
 * side-effects: modifies data at *a
 *
 */

void
make_uniform_prob_dist(const size_t n, float a[])
{
    float u = 1.0 / (float)n;

    for (size_t i = 0; i < n; i++)
        a[i] = u;
    return;
}


/*
 * float detector_calc_accel  - calculate the acceleration of the engine (see note 1, above)
 *
 * arguments:    uint32_t ticks_per_sec   - position sensor sampel rate in Hz
 *               uint8_t  num_tooth_posns - number of quantized possible positions (both teeth and "gaps")
 *               uint32_t t?_ticks  - number of sample-rate 'ticks', passed between tooth gaps
 *               uint8_t  t?_teeth  - number of tooth positions passed during the period measured
 * returns:      acceleration of the engine from t_0 to t_1 in rads/s^2
 * side-effects: none
 *
 *                                                        v_1 - v_0
 * If you remember your physics, acceleration = dv / dt = ---------. In the usual sense, t_0 is when t=0, so
 *                                                        t_1 - t_0
 * we drop the term entirely and consider t_0 to be the time passed while v = v_0.
 *
 *                        D_1 / t_1 - D_0 / t_0        D_1            D_0
 * Since v = D / t,  a = ----------------------- = ----------  -  -----------.  D, in this case, is specified in terms
 *                                 t_1              t_1 * t_1      t_1 * t_0
 *
 * of fractions of the flywheel, which is measured in flywheel teeth P, so distance in this case is scaled by the number
 * of flywheel teeth e. Therefore, our final formula is
 *
 *          P_1               P_0
 *   a = -----------  -  ---------------, times a scaling factor to convert units to rads/s^2.
 *        e * t_1^2       e * t_1 * t_0
 */

float
detector_calc_accel(
        const uint32_t ticks_per_sec,
        const size_t   num_tooth_posns,
        const uint32_t t0_ticks,
        const uint8_t  t0_teeth,
        const uint32_t t1_ticks,
        const uint8_t  t1_teeth
        )
{
    if (t0_ticks == 0)        /* infinite de/acceleration? */
        return FLT_MIN;       /* sure, if you say so       */
    if (t1_ticks == 0) 
        return FLT_MAX;

    float left_denom, right_denom, left_term, right_term;
    float t0_ticks_f = (float)t0_ticks;
    float t1_ticks_f = (float)t1_ticks;
    float t0_teeth_f = (float)t0_teeth;
    float t1_teeth_f = (float)t1_teeth;

    float unit_conversion = 2.0 * PI * (float)ticks_per_sec * (float)ticks_per_sec;

    left_denom  = t1_ticks_f * t1_ticks_f * (float)num_tooth_posns;
    right_denom = t1_ticks_f * t0_ticks_f * (float)num_tooth_posns;
    left_term   = t1_teeth_f / left_denom;
    right_term  = t0_teeth_f / right_denom;

    return unit_conversion * (left_term - right_term);
}


/*
 * float prob_of_move - calculate the Bayesian probability of the posterior based on accel
 *
 * arguments: float prior      - the prior probability of a location
 *            float max_accel  - the maximum believable acceleration
 *            float accel      - the putatively observed acceleration
 *            float error_rate - the error rate of the "sensor"
 * returns: posterior probability
 * side-effects: none
 *
 * Is the observed acceleration too high to be plausible? If so, return a low
 * probability value based on error_rate. Otherwise, return a high value.
 */

float
prob_of_move(
        const float prior,
        const float max_accel,
        const float accel,
        const float error_rate
        )
{
    if (fabsf(accel) > max_accel)
        return prior * error_rate;
    else
        return prior * (1-error_rate);
}


/*
 * void normalize_dist - normalize the probability distribution to [0,1]
 *
 * arguments: float posterior[]       - array of posterior probabilities to normalize
 *            size_t num_tooth_tips   - number of possible positions
 *            float normalized[]      - storage for the normalized distribution
 * returns: void
 * side-effects: modifies *normalized
 */

void
normalize_dist(
        float posterior[const],
        const size_t num_tooth_tips,
        float normalized[]
        )
{

#ifdef SOFTMAX
    /* Experimental: softmax normalization is very very expensive */
    float sum = 0.0;

    for (size_t i = 0; i < num_tooth_tips; i++)
        sum += expf(posterior[i]);

    for (size_t i = 0; i < num_tooth_tips; i++)
        normalized[i] = expf(posterior[i]) / sum;

#else
    /* This is cheaper and works. */
    float invsum = 0.0;

    for (size_t i = 0; i < num_tooth_tips; i++)
        invsum += posterior[i];

    if (invsum == 0)      /* anything is possible  */
        invsum = FLT_MAX; /* so I remain credulous */

    invsum = 1.0 / invsum;

    for (size_t i = 0; i < num_tooth_tips; i++)
        normalized[i] = posterior[i] * invsum;

#endif
    return;
}


/*
 * void detector_find_max_prob - find the highest probability value in the distribution
 *
 * arguments: float prob_dist[]      - probability distrubution
 *            size_t num_tooth_tips - number of possible locations
 *            float* max_prob        - value of the highest probability bin
 *            uint8_t* max_bin       - index of the highest probability bin
 *  returns: nothing
 *  side-effects: modifies *max_prob and *max_bin
 */

void
detector_find_max_prob(
        float prob_dist[const],
        size_t num_tooth_tips,
        float* const max_prob,
        uint8_t* const max_bin
        )
{
    float curr_max = 0;

    for (size_t i = 0; i < num_tooth_tips; i++)
    {
        if (prob_dist[i] > curr_max)
        {
            curr_max = prob_dist[i];
            *max_prob = prob_dist[i];
            *max_bin = i;
        }
    }
}

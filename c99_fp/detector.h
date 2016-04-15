
typedef struct {
    uint8_t current_tooth;      // = 0
    bool    has_sync;
    bool    phase;
    float   velocity;

    uint8_t  *tooth_dists;      // pointer to array containing tooth distances, e.g. { 2, 1, 1 }
    uint32_t ticks_per_sec;     // sample frequency of the detector's timer, in Hz
    size_t   num_tooth_tips;    // number of actual teeth on the flywheel (e.g. 59 for a 60-1 wheel)
    uint8_t  num_tooth_posns;   // number of places where a tooth could be (e.g. 60 for a 60-1 wheel)

    uint32_t previous_timer;    // assumed 32 bits here, I'll have to check the actual hardware

    float max_accel;            // Maximum acceleration in radians per second squared; this is the value
                                //  above which an engine cannot possibly accelerate, and thus measurements
                                //  appearing to exceed this value are obviously wrong.

    float *tooth_prob;          // pointer to an array containing the prior probability distribution
    float confidence;           // max(tooth_prob)
    float error_rate;           // Used for Bayesian analysis of our input
                                // We don't calculate error_rate dynamically - mostly because if we track it in real-time,
                                //  it can get low enough that we never get sync again. oops. So we use an experimentally
                                //  determined value.

    uint32_t interrupt_counter; // detected_errors / interrupt_counter = error_rate
                                // it's worth noting that at 20kHz interrupts, we can count
                                // up to ~59 hours of run time. so reboot your engine every couple of days.
} Detector;

/* Declarations */


/* Initialize the detector at d */
void
detector_init(
        Detector* d,
        uint8_t tooth_dists[const],
        const size_t num_tooth_tips,
        const uint8_t num_tooth_posns,
        float tooth_prob[const],
        const uint32_t sample_rate,
        const float max_accel,
        const float error_rate);

/* Execute a probabalistic 1-position move */
void
detector_move(
        float prior[const],
        const size_t num_tooth_tips,
        const float error_rate,
        float posterior[]);

/* Perform a probabalistic localization step given sensor input */
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
        float posterior[]);

/* Find the bin with, and the value of, the highest probability */
void
detector_find_max_prob(
        float prob_dist[const],
        const size_t num_tooth_tips,
        float* const max_prob,
        uint8_t* const max_bin);

/* Calculate the acceleration between t0 and t1 in rads/s^2 */
float
detector_calc_accel(
        const uint32_t ticks_per_sec,
        const size_t   num_tooth_posns,
        const uint32_t t0_ticks,
        const uint8_t  t0_teeth,
        const uint32_t t1_ticks,
        const uint8_t  t1_teeth);

/* Count up the number of flywheel divisions (teeth + gaps) */
size_t count_tooth_posns(
        uint8_t num_tooth_tips,
        uint8_t tooth_dists[const]);

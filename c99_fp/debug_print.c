
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "detector.h"

/* Declarations */

void debug_print_detector(const Detector* const d);

void debug_print_prob_dist_f(
        float dist[const], 
        const size_t num_bins,
        const char* name,
        const char* spec);

void debug_print_tooth_map(
        uint8_t map[const],
        const size_t num_bins,
        const char* name,
        const char* spec);


/* Definitions */

/* Pretty-print a Detector structure */
void debug_print_detector(const Detector* const d)
{
#ifdef DEBUG
    printf("Detector <%lx>\n", (uintptr_t)d);
    if (d->has_sync) {
        printf("  has sync on tooth %hhu\n", d->current_tooth);
        printf("  confidence level %2.3f\n", d->confidence);
    }
    debug_print_tooth_map(d->tooth_dists, d->num_tooth_tips, "\ttooth_dists", "%hhu");
    debug_print_prob_dist_f(d->tooth_prob, d->num_tooth_tips, "\ttooth_prob", "%2.3f");
#endif
}


/* Pretty-print a probability distribution */
void debug_print_prob_dist_f(
        float dist[const], 
        const size_t num_bins,
        const char* name,
        const char* spec)
{
#ifdef DEBUG
    printf("%s = [", name);
    printf(spec, dist[0]);
    for (uint8_t i = 1; i < num_bins; i++)
    {
        printf(", ");
        printf(spec, dist[i]*100);
    }
    printf("]\n");
#endif
}

/* Pretty-print a tooth map (read: array) of uint8_t values */
void debug_print_tooth_map(
        uint8_t map[const], 
        const size_t num_bins,
        const char* name,
        const char* spec)
{
#ifdef DEBUG
    printf("%s = [", name);
    printf(spec, map[0]);
    for (uint8_t i = 1; i < num_bins; i++)
    {
        printf(", ");
        printf(spec, map[i]);
    }
    printf("]\n");
#endif
}

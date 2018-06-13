// -*- c++ -*-
#ifndef RNG_H
#define RNG_H

#include <gsl/gsl_rng.h>
#include "pcg/pcg_random.hpp"

extern pcg32 g_rng;

// Setup pcg for use by GSL
typedef struct
{
    pcg32 *rng;
} gsl_pcg_state;

static void gsl_pcg_set(void *state, unsigned long int seed)
{
    ((gsl_pcg_state *)state)->rng = &g_rng;
    (void)seed;			// unused
}

static unsigned long int gsl_pcg_get(void *state)
{
    return (*((gsl_pcg_state *)state)->rng)();
}

static double gsl_pcg_get_double(void *state)
{
    // Means range [0, 1)
    return (*((gsl_pcg_state *)state)->rng)() / 4294967296.;
}


// typedef struct
//   {
//     const char *name;
//     unsigned long int max;
//     unsigned long int min;
//     size_t size;
//     void (*set) (void *state, unsigned long int seed);
//     unsigned long int (*get) (void *state);
//     double (*get_double) (void *state);
//   }
// gsl_rng_type;
static const gsl_rng_type gsl_rng_pcg = {
    "pcg",
    0xffffffffUL,
    0,
    sizeof(gsl_pcg_state),
    &gsl_pcg_set,
    &gsl_pcg_get,
    &gsl_pcg_get_double
};

// Seed RNG using user-provided value.
//   pcg32::state_type is uint64_t.
void setRngSeed(pcg32::state_type seed);

// Return random number in range [0, 1)
double randomNumber();

#endif

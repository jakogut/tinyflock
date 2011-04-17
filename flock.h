#ifndef FLOCK_H_
#define FLOCK_H_

#include "boid.h"
#include "configuration.h"

#define FLOCK_TYPE_RANDOM 0
#define FLOCK_TYPE_CENTERED 1

boid* create_flock(configuration* config);

void destroy_flock(boid* flock);

typedef struct { int thread_id; boid* flock; boid* flock_copy; configuration* config; } flock_update_worker_args;

int flock_update_worker(void* arg);

// Update the location and velocity of each boid in the flock
void flock_update(boid* flock, configuration* config, vector* cursor_pos, int* cursor_interaction);

// Render the flock
void flock_render(boid* flock, configuration* config, SDL_Surface* screen);

void flock_influence(vector* v, boid* flock, boid* b, configuration* config);

inline float rand_range(float min, float max);

#endif

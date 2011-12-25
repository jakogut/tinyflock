#ifndef FLOCK_H_
#define FLOCK_H_

#include "boid.h"
#include "configuration.h"

#include <GL/gl.h>
#include <GL/glu.h>

#define FLOCK_TYPE_RANDOM 0
#define FLOCK_TYPE_CENTERED 1

boid* create_flock(configuration* config);

void destroy_flock(boid* flock);

typedef struct { int* run; int* frame_count; int* update_count; } status_args;
int status_thread(void* arg);

typedef struct { int thread_id; boid* flock; boid* flock_copy; configuration* config; vector* cursor_pos; int* cursor_interaction; } flock_update_worker_args;
int flock_update_worker_thread(void* arg);

// Update the location and velocity of each boid in the flock
typedef struct { int* run; boid* flock; configuration* config; vector* cursor_pos; int* cursor_interaction; int* update_count; } flock_update_args;
int flock_update_thread(void* arg);

// Flock render function pointer type
typedef void (*render_func_t)(boid*, configuration*, SDL_Surface*);

// Flock render functions
void flock_render_software(boid* flock, configuration* config, SDL_Surface* screen);
void flock_render_gl(boid* flock, configuration* config, SDL_Surface* screen);

void flock_influence(vector* v, boid* flock, boid* b, configuration* config);

inline void boid_approach(boid* b, vector* v, float weight);
inline void boid_flee(boid* b, vector* v, float weight);

inline float rand_range(float min, float max);

#endif

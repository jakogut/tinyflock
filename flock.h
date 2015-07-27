#ifndef FLOCK_H_
#define FLOCK_H_

#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include "VLIQ/vliq.h"
#include "configuration.h"

typedef struct
{
	configuration *config;

	vec2_t* location;
	vec2_t* velocity;
	vec2_t* acceleration;
} flock;

flock* flock_create(configuration* config);
void flock_destroy(flock* f);

void flock_randomize_location(flock* f);
void flock_randomize_velocity(flock* f);

typedef struct { int* run; int thread_id; long* ticks; flock* f; vec2_t* cursor_pos; int* cursor_interaction; } flock_update_worker_args;
void* flock_update_worker_thread(void* arg);

void flock_influence(vec2_t* v, flock* f, int boid_id, float max_velocity);

void boid_approach(flock* f, int boid_id, vec2_t v, float weight);
void boid_flee(flock* f, int boid_id, vec2_t v, float weight);

float rand_range(float min, float max);

#endif

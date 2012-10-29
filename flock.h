#ifndef FLOCK_H_
#define FLOCK_H_

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glfw.h>

#include "VLIQ/vliq.h"
#include "configuration.h"

typedef struct
{
	vec3_t* location;
	vec3_t* velocity;
	vec3_t* acceleration;
} flock;

flock* flock_create(configuration* config);
void flock_destroy(flock* f);

void flock_randomize(flock* f, int start, int end);

typedef struct { int* run; int thread_id; int* ticks; flock* f; configuration* config; vec3_t* cursor_pos; int* cursor_interaction; } flock_update_worker_args;
void flock_update_worker_thread(void* arg);

void flock_influence(vec3_t* v, flock* f, int boid_id, configuration* config);

void boid_approach(flock* f, int boid_id, vec3_t v, float weight);
void boid_flee(flock* f, int boid_id, vec3_t v, float weight);

float rand_range(float min, float max);

#endif

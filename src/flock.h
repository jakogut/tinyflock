#ifndef FLOCK_H_
#define FLOCK_H_

#ifdef __WIN32
#include <windef.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <omp.h>

#ifdef ENABLE_ANN
#include <fann.h>
#endif

#include "VLIQ/vliq.h"
#include "configuration.h"

struct flock_snapshot
{
	vec2_t *location, *velocity;
	vec2_t *acceleration;

	struct flock_snapshot *next;
};

struct flock_sample
{
	size_t size;
	int **indices;
	float **distances;
};

struct flock
{
	struct configuration *config;
	struct flock_sample sample;

	struct flock_snapshot *history;
	struct flock_snapshot *history_tail;
	int num_snapshots;

	vec2_t *location;
	vec2_t *velocity;
	vec2_t *acceleration;
};

struct flock* flock_create(struct configuration* config);
void flock_destroy(struct flock* f);

void flock_randomize_location(struct flock* f);
void flock_randomize_velocity(struct flock* f);

struct update_thread_arg {
	int *run;
	long *ticks;
	struct flock *f;

	vec2_t *cursor_pos;
	int *cursor_interaction;
} update_thread_arg;

void *flock_update(void *arg);

void flock_influence(vec2_t* v, struct flock* f, int boid_id, float max_velocity);

#ifdef ENABLE_ANN
void flock_influence_nn(vec2_t* v, struct flock* f, int boid_id, float max_velocity, struct fann *ann);
#endif

void boid_approach(struct flock* f, int boid_id, vec2_t v, float weight);
void boid_flee(struct flock* f, int boid_id, vec2_t v, float weight);

float rand_range(float min, float max);

#endif

#ifndef FLOCK_H_
#define FLOCK_H_

#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include "VLIQ/vliq.h"
#include "configuration.h"

struct flock
{
	struct configuration *config;

	vec2_t* location;
	vec2_t* velocity;
	vec2_t* acceleration;
} flock;

struct flock* flock_create(struct configuration* config);
void flock_destroy(struct flock* f);

void flock_randomize_location(struct flock* f);
void flock_randomize_velocity(struct flock* f);

void flock_update(struct flock *, float);

void flock_influence(vec2_t* v, struct flock* f, int boid_id, float max_velocity);

void boid_approach(struct flock* f, int boid_id, vec2_t v, float weight);
void boid_flee(struct flock* f, int boid_id, vec2_t v, float weight);

float rand_range(float min, float max);

#endif

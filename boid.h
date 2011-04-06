#ifndef BOID_H_
#define BOID_H_

#include <SDL.h>

#include "configuration.h"
#include "vector.h"

typedef struct
{
	// Where the boid is on the screen
	vector location;

	// Whether the boid's speed is increasing or decreased, and in what direction
	vector acceleration;

	// What direction the boid is moving in, and at what speed
	vector velocity;

} boid;

void init_boid(boid* b, float loc_x, float loc_y, float vel_x, float vel_y);

inline void flock_influence(vector* v, boid* flock, boid* b, configuration* config);

#endif

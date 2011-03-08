#ifndef BOID_H_
#define BOID_H_

#include <SDL.h>

#include "flockconfig.h"
#include "vector.h"

typedef struct
{
	SDL_Surface* sprite;

	// Where the boid is on the screen
	vector location;

	// Whether the boid's speed is increasing or decreased, and in what direction
	vector acceleration;

	// What direction the boid is moving in, and at what speed
	vector velocity;

} boid;

void init_boid(boid* b, SDL_Surface* image, int loc_x, int loc_y, int vel_x, int vel_y);

inline void flock_influence(vector* v, boid* flock, boid* b);

inline void flock_limit_velocity(boid* flock, int num_boids, float max_velocity);

#endif

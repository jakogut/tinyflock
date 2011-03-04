#ifndef BOID_H_
#define BOID_H_

#include <SDL/SDL.h>

#include "config.h"
#include "vector.h"

typedef struct
{
	SDL_Surface* sprite;

	// Where the boid is on the screen
	vector* location;

	// Whether the boid's speed is increasing or decreased, and in what direction
	vector* acceleration;

	// What direction the boid is moving in, and at what speed
	vector* velocity;

} boid;

boid* create_boid(SDL_Surface* image, int loc_x, int loc_y, int vel_x, int vel_y);

void destroy_boid(boid* b);

// Avoid crowding flockmates
vector* flock_separate (boid** flock, boid* b);

// Head in the same average direction as flockmates
vector* flock_align(boid** flock, boid* b);

// Head towards the average position of flockmates
vector* flock_cohere(boid** flock, boid* b);

void flock_limit_velocity(boid** flock, int num_boids, float max_velocity);

#endif

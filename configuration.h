#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <SDL/SDL.h>

// Number of worker threads used to update boids
#define NUM_THREADS 4

// Renderer types
#define RENDERER_SOFTWARE 0
#define RENDERER_GL 1

// Default video configuration
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define SCREEN_DEPTH    32

#define FPS 0

#define DRAW_ANCHOR 0

// Default input settings
#define INFLUENCE_RADIUS 75

// Default boid parameters
#define NUM_BOIDS 2048

#define MAX_BOID_VELOCITY 10
#define MIN_BOID_SEPARATION 3
#define MAX_BOID_STEERING_FORCE 0.05
#define NEIGHBORHOOD_RADIUS 25

typedef struct{ unsigned char r, g, b; } color24_t;

typedef struct
{
	SDL_Surface* boid_sprite;
	SDL_Surface* anchor_sprite;

	int num_threads;

	struct
	{
		int renderer;

		int screen_width, screen_height, screen_depth;

		int frames_per_second;

		int draw_anchor;

	} video;

	struct
	{
		float influence_radius;

	} input;

	struct
	{
		int size;

		int max_velocity;
		int min_separation;
		float max_steering_force;
		int neighborhood_radius;

	} flock;

} configuration;

#endif

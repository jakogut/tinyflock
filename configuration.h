#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// Number of worker threads used to update boids
#define NUM_THREADS 4

// Default video configuration
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define SCREEN_DEPTH    32

// Limiting the frame rate to 50 helps free the processor for updates
#define FPS 60

// Default input settings
#define INFLUENCE_RADIUS 150

// Default boid parameters
#define NUM_BOIDS 6144

#define MAX_BOID_VELOCITY 5
#define MIN_BOID_SEPARATION 5
#define MAX_BOID_STEERING_FORCE 0.3
#define NEIGHBORHOOD_RADIUS 30

typedef struct
{
	int num_threads;

	struct
	{
		int screen_width, screen_height, screen_depth;

		int frames_per_second;

	} video;

	struct
	{
		float influence_radius;

	} input;

	struct
	{
		int size;

		float max_velocity;
		float max_steering_force;
		int neighborhood_radius;
		int min_separation;

	} flock;

} configuration;

#endif

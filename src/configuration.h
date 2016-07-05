#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// Default video configuration
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define SCREEN_DEPTH    32

// Limiting the frame rate to 50 helps free the processor for updates
#define FPS 60

// Default input settings
#define INFLUENCE_RADIUS 60
#define INFLUENCE_WEIGHT 80

// Default boid parameters
#define NUM_BOIDS 6144

#define MAX_BOID_VELOCITY 3
#define MIN_BOID_SEPARATION 10
#define MAX_BOID_STEERING_FORCE 0.1
#define NEIGHBORHOOD_RADIUS 30

#define TF_MODE_FLOCK_CONV 0
#define TF_MODE_FLOCK_NN 1
#define TF_MODE_TRAIN 2

#define MAX_EPOCHS 256
#define REPORT_INTERVAL 4
#define DESIRED_ERROR 0.005

struct configuration
{
	int mode;

	char capture_filename[256];

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

	struct
	{
		char trained_net[256];
	} flock_nn;

	struct
	{
		char input[256];
		char output[256];

                int max_epochs;
                int report_interval;
                float desired_error;
                
                char network_dim[256];
	} train;

} configuration;

#endif

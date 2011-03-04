#ifndef CONFIG_H_
#define CONFIG_H_

// Video configuration
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define SCREEN_DEPTH    32

#define FPS 50

// BOID parameters
#define BOID_IMAGE_FILENAME "boid.bmp"
#define NUM_BOIDS 500

#define MAX_BOID_VELOCITY 10
#define MIN_BOID_SEPARATION 5
#define MAX_BOID_STEERING_FORCE 0.2
#define NEIGHBORHOOD_RADIUS 50

#endif

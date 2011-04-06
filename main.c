#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "flock.h"
#include "boid.h"

#include "configuration.h"

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}

// Returns true if key pressed, false otherwise
int key_pressed(SDL_Event* event, int key)
{
	return (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE);
}

// Handle input and events. Return 0 to quit the program, or 1 to keep running.
inline int handle_events(SDL_Event* event)
{
	while(SDL_PollEvent(event))
		if(event->type == SDL_QUIT || key_pressed(event, SDLK_ESCAPE)) return 0;

	return 1;

}

int print_help()
{
	printf("(C) 2011 by Joseph A. Kogut (joseph.kogut@gmail.com)\n"
		"This software is distributed under the MIT license,\n"
		"with no warranty, express or implied. Run this software\n"
		"at your own risk.\n\n"

		"-h / --help\t\tPrint this help message.\n\n"

		"Video configuration\n"
		"------------------------------------------------------------\n"
		"--height\tSpecify screen height in pixels.\n"
		"--width\t\tSpecify screen width in pixels.\n"
		"--depth\t\tSpecify screen depth in bits.\n"
		"--fps\t\tSpecify the maximum number of frames to render\n"
		"\t\tper second.\n\n"

		"Flock configuration\n"
		"------------------------------------------------------------\n"
		"-n / --num-boids\tSpecify the number of boids to create.\n");

	return 0;
}

int main(int argc, char** argv)
{
	// Create a configuration object, and set the values to the defaults
	configuration config;

	config.video.screen_width = SCREEN_WIDTH;
	config.video.screen_height = SCREEN_HEIGHT;
	config.video.screen_depth = SCREEN_DEPTH;
	config.video.frames_per_second = FPS;

	config.flock.num_boids = NUM_BOIDS;
	config.flock.max_boid_velocity = MAX_BOID_VELOCITY;
	config.flock.min_boid_separation = MIN_BOID_SEPARATION;
	config.flock.max_boid_steering_force = MAX_BOID_STEERING_FORCE;
	config.flock.neighborhood_radius = NEIGHBORHOOD_RADIUS;

	// Parse arguments
	int i;
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return print_help();

		if(strcmp(argv[i], "--width") == 0)
			config.video.screen_width = atoi(argv[++i]);

		if(strcmp(argv[i], "--height") == 0)
			config.video.screen_height = atoi(argv[++i]);

		if(strcmp(argv[i], "--depth") == 0)
			config.video.screen_depth = atoi(argv[++i]);

		if(strcmp(argv[i], "--fps") == 0)
			config.video.frames_per_second = atoi(argv[++i]);

		if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--num-boids") == 0)
			config.flock.num_boids = atoi(argv[++i]);
	}

	// Init SDL and create our screen
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Event event;

	SDL_Surface* screen = SDL_SetVideoMode(config.video.screen_width, config.video.screen_height,
					       config.video.screen_depth, SDL_HWSURFACE | SDL_DOUBLEBUF);

	// Load and format our boid image
	SDL_Surface* temp = IMG_Load("boid.png");
	SDL_Surface* boid_image = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	// Create an array of boids
	boid* flock = malloc(sizeof(boid) * config.flock.num_boids);

	srand(time(NULL));

	for(i = 0; i < config.flock.num_boids; i++)
	{
		init_boid(&flock[i], boid_image, rand_range(0, config.video.screen_width), rand_range(0, config.video.screen_height),
					rand_range((0 - config.flock.max_boid_velocity), config.flock.max_boid_velocity),
					rand_range((0 - config.flock.max_boid_velocity), config.flock.max_boid_velocity));
	}

	flock_render_data render_data;

	render_data.run = 1;
	render_data.flock = flock;
	render_data.config = &config;
	render_data.screen = screen;

	pthread_t render_thread;
	pthread_create(&render_thread, NULL, flock_render_pthread, (void*)&render_data);

	// Run while true
	int* run = &render_data.run;

	// If the frame limit is not greater than 0, don't delay between frames at all.
	if(config.video.frames_per_second > 0)
	{
		float delay = (1000 / config.video.frames_per_second);

		while(*run)
		{
			*run = handle_events(&event);
			flock_update(flock, &config);

			SDL_Delay(delay);
		}
	}
	else
	{
		while(*run)
		{
			*run = handle_events(&event);
			flock_update(flock, &config);
		}
	}

	free(flock);

	SDL_FreeSurface(boid_image);
	SDL_Quit();

	return 0;
}

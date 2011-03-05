#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "flockconfig.h"
#include "boid.h"

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}

// Returns true if key pressed, false otherwise
int keyPressed(SDL_Event* event, int key)
{
	return (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE);
}

// Update the positions of all the boids based on their velocity
void update(boid** flock)
{
	vector* separation;
	vector* alignment;
	vector* cohesion;

	vector* random;

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		// Get our influence vectors
		separation = flock_separate(flock, flock[i]);
		alignment = flock_align(flock, flock[i]);
		cohesion = flock_cohere(flock, flock[i]);
		random = create_randomized_vector(0, 1.3);

		// Add our influence vectors to the acceleration vector of the boid
		vector_add(flock[i]->acceleration, separation);
		vector_add(flock[i]->acceleration, alignment);
		vector_add(flock[i]->acceleration, cohesion);

		// Add a little random weighting to the acceleration, to make movements more organic
		vector_mul(flock[i]->acceleration, random);

		vector_add(flock[i]->velocity, flock[i]->acceleration);
		vector_add(flock[i]->location, flock[i]->velocity);

		// Reset the acceleration vectors for the flock
		vector_init(flock[i]->acceleration, 0);

		// Clean up our temporary influence vectors
		destroy_vector(separation);
		destroy_vector(alignment);
		destroy_vector(cohesion);
	}

	// Limit boid velocity to MAX_BOID_VELOCITY as defined in config.h
	flock_limit_velocity(flock, NUM_BOIDS, MAX_BOID_VELOCITY);
}

// Render the boids
void render(boid** flock, SDL_Surface* screen)
{
	SDL_FillRect(screen, NULL, 0xFFFFFF);

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		// If the boid goes off the screen, wrap the location around to the other side
		if(flock[i]->location->x >= SCREEN_WIDTH) flock[i]->location->x = 0;
		else if(flock[i]->location->x <= 0) flock[i]->location->x = SCREEN_WIDTH;

		if(flock[i]->location->y >= SCREEN_HEIGHT) flock[i]->location->y = 0;
		else if(flock[i]->location->y <= 0) flock[i]->location->x = SCREEN_HEIGHT;

		SDL_Rect offset;

		offset.x = flock[i]->location->x;
		offset.y = flock[i]->location->y;

		SDL_BlitSurface(flock[i]->sprite, NULL, screen, &offset);
	}

	SDL_Flip(screen);
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Event event;

	SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_HWSURFACE | SDL_DOUBLEBUF);

	// Load and format our boid image
	SDL_Surface* temp = IMG_Load("boid.png");
	SDL_Surface* boid_image = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	// Create an array of boids
	boid** flock = malloc(sizeof(boid*) * NUM_BOIDS);

	srand(time(NULL));

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		flock[i] = create_boid(boid_image, rand_range(0, SCREEN_WIDTH), rand_range(0, SCREEN_HEIGHT),
					rand_range((0 - MAX_BOID_VELOCITY), MAX_BOID_VELOCITY),
					rand_range((0 - MAX_BOID_VELOCITY), MAX_BOID_VELOCITY));
	}

	// Run while true
	int run = 1;

	while(run)
	{
		update(flock);
		render(flock, screen);

		// Handle keyboard input
		while(SDL_PollEvent(&event))
			if(event.type == SDL_QUIT || keyPressed(&event, SDLK_ESCAPE)) run = 0;

		// If the framerate is not a positive number greater than zero, don't limit it
		#if (FPS > 0)
		SDL_Delay(1000 / FPS);
		#endif
	}

	SDL_FreeSurface(boid_image);

	for(i = 0; i < NUM_BOIDS; i++) destroy_boid(flock[i]);

	SDL_Quit();

	return 0;
}

#include "flock.h"

boid* create_flock(configuration* config)
{
	boid* flock;

	flock = malloc(sizeof(boid) * config->flock.num_boids);

	int i;
	for(i = 0; i < config->flock.num_boids; i++)
	{
		// We add 0.0001 * i to the position of each boid so they don't spawn on top of each other.
		init_boid(&flock[i], (config->video.screen_width / 2) + (0.0001 * i),
				     (config->video.screen_height / 2) + (0.0001 * i),
				     rand_range((0.0f - config->flock.max_boid_velocity), config->flock.max_boid_velocity),
				     rand_range((0.0f - config->flock.max_boid_velocity), config->flock.max_boid_velocity));
	}

	return flock;
}

void destroy_flock(boid* flock)
{
	if(flock) free(flock);

	flock = NULL;
}

void flock_update(boid* flock, configuration* config)
{
	int i;
	for(i = 0; i < config->flock.num_boids; i++)
	{
		// Add our influence vectors to the acceleration vector of the boid
		flock_influence(&flock[i].acceleration, flock, &flock[i], config);

		vector_add(&flock[i].velocity, &flock[i].acceleration);
		vector_add(&flock[i].location, &flock[i].velocity);

		// Reset the acceleration vectors for the flock
		init_vector_scalar(&flock[i].acceleration, 0.0);

		// If the boid goes off the screen, wrap the location around to the other side
		if(flock[i].location.x >= config->video.screen_width)
			flock[i].location.x = 0;
		else if(flock[i].location.x <= 0)
			flock[i].location.x = config->video.screen_width;

		if(flock[i].location.y >= config->video.screen_height)
			flock[i].location.y = 0;
		else if(flock[i].location.y <= 0)
			flock[i].location.x = config->video.screen_height;

	}
}

void* flock_render_pthread(void* arg)
{
	const flock_render_data* args = (flock_render_data*)arg;

	while(args->run)
	{
		SDL_FillRect(args->screen, NULL, 0xFFFFFF);

		int i;
		for(i = 0; i < args->config->flock.num_boids; i++)
		{
			SDL_Rect offset;

			offset.x = args->flock[i].location.x;
			offset.y = args->flock[i].location.y;

			SDL_BlitSurface(args->config->boid_sprite, NULL, args->screen, &offset);
		}

		SDL_Flip(args->screen);
	}
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


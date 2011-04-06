#include "flock.h"

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
	flock_render_data* args = (flock_render_data*)arg;

	while(args->run)
	{
		SDL_FillRect(args->screen, NULL, 0xFFFFFF);

		int i;
		for(i = 0; i < args->config->flock.num_boids; i++)
		{
			SDL_Rect offset;

			offset.x = args->flock[i].location.x;
			offset.y = args->flock[i].location.y;

			SDL_BlitSurface(args->flock[i].sprite, NULL, args->screen, &offset);
		}

		SDL_Flip(args->screen);
	}
}

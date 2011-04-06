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

		else if(flock[i].location.y >= config->video.screen_height)
			flock[i].location.y = 0;
		else if(flock[i].location.y <= 0)
			flock[i].location.y = config->video.screen_height;
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

void  flock_influence(vector* v, boid* flock, boid* b, configuration* config)
{
	init_vector_scalar(v, 0);

     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vector influence[2];

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2];

	int i;
	for(i = 0; i < 2; i++)
		init_vector_scalar(&influence[i], 0);

	float neighborhood_radius_squared = pow(config->flock.neighborhood_radius, 2);
	float min_boid_separation_squared = pow(config->flock.min_boid_separation, 2);

	for(i = 0; i < config->flock.num_boids; i++)
	{
		if(&flock[i] != b)
		{
			float distance = vector_distance_nosqrt(&flock[i].location, &b->location);

			if(distance < neighborhood_radius_squared)
			{
				vector_add(&influence[0], &flock[i].velocity);
				vector_add(&influence[0], &flock[i].location);

				population[0] += 2;

				if(distance < min_boid_separation_squared)
				{
					vector loc;
					copy_vector(&loc, &b->location);

					vector_sub(&loc, &flock[i].location);
					vector_normalize(&loc);
					vector_div_scalar(&loc, distance);

					vector_add(&influence[1], &loc);

					population[1]++;
                                }
                        }
                }
        }

	for(i = 0; i < 2; i++)
	{
		if(population[i] > 0)
			vector_div_scalar(&influence[i], population[i]);
		else
			init_vector_scalar(&influence[i], 0);

		if(vector_magnitude(&influence[i]) > 0)
		{
			vector_normalize(&influence[i]);
			vector_mul_scalar(&influence[i], config->flock.max_boid_velocity);
			vector_sub(&influence[i], &b->velocity);
			vector_mul_scalar(&influence[i], config->flock.max_boid_steering_force);
		}
	}

	for(i = 0; i < 2; i++)
		vector_add(v, &influence[i]);
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


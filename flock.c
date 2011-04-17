#include "flock.h"

#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

boid* create_flock(configuration* config)
{
	boid* flock;

	flock = malloc(sizeof(boid) * config->flock.num_boids);

	int i;
	for(i = 0; i < config->flock.num_boids; i++)
	{
		// We add 0.0001 * i to the position of each boid so they don't spawn on top of each other.
		init_boid(&flock[i], (0.0001 * i), (0.0001 * i),
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


int flock_update_worker(void* arg)
{
	flock_update_worker_args* args = (flock_update_worker_args*)arg;

	int work_size = args->config->flock.num_boids / args->config->num_threads;

	int begin_work = work_size * args->thread_id;
	int end_work = begin_work + work_size - 1;

	int i;
	for(i = begin_work; i < end_work; i++)
	{
		flock_influence(&args->flock[i].acceleration, args->flock_copy, &args->flock_copy[i], args->config);

		vector_add(&args->flock[i].velocity, &args->flock[i].acceleration);
		vector_add(&args->flock[i].location, &args->flock[i].velocity);

		// Reset the acceleration vectors for the flock
		init_vector_scalar(&args->flock[i].acceleration, 0.0);

		// If the boid goes off the screen, wrap the location around to the other side
		if(args->flock[i].location.x >= args->config->video.screen_width)
			args->flock[i].location.x = 0;
		else if(args->flock[i].location.x <= 0)
			args->flock[i].location.x = args->config->video.screen_width;

		else if(args->flock[i].location.y >= args->config->video.screen_height)
			args->flock[i].location.y = 0;
		else if(args->flock[i].location.y <= 0)
			args->flock[i].location.y = args->config->video.screen_height;
	}

	return 0;
}

void flock_update(boid* flock, configuration* config, vector* cursor_pos, int* cursor_interaction)
{
	SDL_Thread** workers = malloc(sizeof(SDL_Thread*) * config->num_threads);

	flock_update_worker_args* worker_args = malloc(sizeof(flock_update_worker_args) * config->num_threads);

	boid* flock_copy = malloc(sizeof(boid) * config->flock.num_boids);
	memcpy(flock_copy, flock, sizeof(boid) * config->flock.num_boids);

	int i;
	for(i = 0; i < config->num_threads; i++)
	{
		worker_args[i].thread_id = i;
		worker_args[i].flock = flock;
		worker_args[i].flock_copy = flock_copy;
		worker_args[i].config = config;

		workers[i] = SDL_CreateThread(flock_update_worker, (void*)&worker_args[i]);
	}

	for(i = 0; i < config->num_threads; i++)
		SDL_WaitThread(workers[i], NULL);

	free(worker_args);
	free(flock_copy);
	free(workers);
}

void flock_render(boid* flock, configuration* config, SDL_Surface* screen)
{
	SDL_Rect center;
	center.x = config->video.screen_width / 2;
	center.y = config->video.screen_height / 2;

	SDL_FillRect(screen, NULL, 0xFFFFFF);

	if(config->video.draw_anchor) SDL_BlitSurface(config->anchor_sprite, NULL, screen, &center);

	int i;
	for(i = 0; i < config->flock.num_boids; i++)
	{
		SDL_Rect offset;

		offset.x = flock[i].location.x;
		offset.y = flock[i].location.y;

		SDL_BlitSurface(config->boid_sprite, NULL, screen, &offset);
	}

	SDL_Flip(screen);
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


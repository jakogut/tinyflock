#include "flock.h"

boid* create_flock(configuration* config)
{
	boid* flock = malloc(sizeof(boid) * config->flock.size);

	int i;
	for(i = 0; i < config->flock.size; i++)
	{
		// We add 0.0001 * i to the x and y coordinates of each boid so they don't spawn on top of each other.
		init_boid(&flock[i], (0.0001 * i), (0.0001 * i),
				     rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity),
				     rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity));
	}

	return flock;
}

void destroy_flock(boid* flock)
{
	if(flock) free(flock);
	flock = NULL;
}

int status_thread(void* arg)
{
	status_args* args = (status_args*)arg;
	uint32_t start_time_fps = SDL_GetTicks(), start_time_ups = SDL_GetTicks();
	uint32_t start_fcount = *args->frame_count, start_ucount = *args->update_count;

	int sample_size = 10;

	int last_fps = 0, current_fps = 0;
	int last_ups = 0, current_ups = 0;

	while(*args->run)
	{
		if(*args->frame_count >= (start_fcount + sample_size))
		{
			int end_time = SDL_GetTicks();

			last_fps = current_fps;
			current_fps = 1000 / ((end_time - start_time_fps) / sample_size);
			current_fps = (last_fps * 0.2) + (current_fps * 0.8);

			start_fcount = *args->frame_count;
			start_time_fps = SDL_GetTicks();
		}

		if(*args->update_count >= (start_ucount + sample_size))
		{
			int end_time = SDL_GetTicks();

			last_ups = current_ups;
			current_ups = 1000 / ((end_time - start_time_ups) / sample_size);
			current_ups = (last_ups * 0.2) + (current_ups * 0.8);

			start_ucount = *args->update_count;
			start_time_ups = SDL_GetTicks();
		}

		printf("\rFrames per second: %i, Updates per second: %i ", current_fps, current_ups);
		fflush(stdout);
	}

	printf("\n");
}

int flock_update_worker_thread(void* arg)
{
	flock_update_worker_args* args = (flock_update_worker_args*)arg;

	int work_size = args->config->flock.size / args->config->num_threads;

	int begin_work = work_size * args->thread_id;
	int end_work = begin_work + work_size - 1;

	int i;
	for(i = begin_work; i < end_work; i++)
	{
		// Calculate boid movement
		flock_influence(&args->flock[i].acceleration, args->flock_copy, &args->flock_copy[i], args->config);

		// Handle mouse input
		switch(*args->cursor_interaction)
		{
			case 0: break;
			case 1: if(vector_distance(&args->flock[i].location, args->cursor_pos) < args->config->input.influence_radius)
					boid_approach(&args->flock[i], args->cursor_pos, 1.5); break;
			case 2: if(vector_distance(&args->flock[i].location, args->cursor_pos) < args->config->input.influence_radius)
					boid_flee(&args->flock[i], args->cursor_pos, 1.0); break;
			default: break;
		};

		vector_add(&args->flock[i].velocity, &args->flock[i].acceleration);
		vector_add(&args->flock[i].location, &args->flock[i].velocity);

		// Reset the acceleration vectors for the flock
		vector_init_scalar(&args->flock[i].acceleration, 0.0);

		// Wrap coordinates
		args->flock[i].location.x -= args->config->video.screen_width * (args->flock[i].location.x > args->config->video.screen_width);
		args->flock[i].location.x += args->config->video.screen_width * (args->flock[i].location.x < 0);

		args->flock[i].location.y -= args->config->video.screen_height * (args->flock[i].location.y > args->config->video.screen_height);
		args->flock[i].location.y += args->config->video.screen_height * (args->flock[i].location.y < 0);
	}

	return 0;
}

int flock_update_thread(void* arg)
{
	flock_update_args* args = (flock_update_args*)arg;

	SDL_Thread** workers = malloc(sizeof(SDL_Thread*) * args->config->num_threads);
	flock_update_worker_args* worker_args = malloc(sizeof(flock_update_worker_args) * args->config->num_threads);

	boid* flock_copy = malloc(sizeof(boid) * args->config->flock.size);

	int i;
	for(i = 0; i < args->config->num_threads; i++)
		worker_args[i] = (flock_update_worker_args){i, args->flock, flock_copy, args->config, args->cursor_pos, args->cursor_interaction};

	while(*args->run)
	{
		memcpy(flock_copy, args->flock, sizeof(boid) * args->config->flock.size);

		for(i = 0; i < args->config->num_threads; i++)
			workers[i] = SDL_CreateThread(flock_update_worker_thread, (void*)&worker_args[i]);

		for(i = 0; i < args->config->num_threads; i++)
			SDL_WaitThread(workers[i], NULL);

		*args->update_count++;
	}

	free(worker_args);
	free(flock_copy);
	free(workers);

	return 0;
}

void flock_influence(vector* v, boid* flock, boid* b, configuration* config)
{
	vector_init_scalar(v, 0);

     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vector influence[2];

	int i;
	for(i = 0; i < 2; i++)
		vector_init_scalar(&influence[i], 0);

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2] = {0, 0};

	register float neighborhood_radius_squared = powf(config->flock.neighborhood_radius, 2);
	register float min_boid_separation_squared = powf(config->flock.min_separation, 2);

	for(i = 0; i < config->flock.size; i++)
	{
		if(&flock[i] != b)
		{
			register float distance = vector_distance_nosqrt(&flock[i].location, &b->location);

			if(distance <= neighborhood_radius_squared)
			{
				vector temp;
				vector_copy(&temp, &flock[i].velocity);

				vector_mul_scalar(&temp, 0.2f);

				vector_add(&influence[0], &temp);
				vector_add(&influence[0], &flock[i].location);

				population[0] += 2;
			}

			if(distance <= min_boid_separation_squared)
			{
				vector loc;
				vector_copy(&loc, &b->location);

				vector_sub(&loc, &flock[i].location);
				vector_normalize(&loc);
				vector_div_scalar(&loc, distance);

				vector_add(&influence[1], &loc);

				population[1]++;
			}
		}
	}

	for(i = 0; i < 2; i++)
	{
		if(population[i] > 0)
			vector_div_scalar(&influence[i], population[i]);
		else
			vector_init_scalar(&influence[i], 0);

		if(vector_magnitude(&influence[i]) > 0)
		{
			vector_normalize(&influence[i]);
			vector_mul_scalar(&influence[i], config->flock.max_velocity);
			vector_sub(&influence[i], &b->velocity);
			vector_mul_scalar(&influence[i], config->flock.max_steering_force);
		}
	}

	for(i = 0; i < 2; i++)
		vector_add(v, &influence[i]);
}

void boid_approach(boid* b, vector* v, float weight)
{
	vector heading;
	vector_copy(&heading, v);
	vector_sub(&heading, &b->location);

	vector_normalize(&heading);

	vector_mul_scalar(&heading, weight);

	vector_add(&b->acceleration, &heading);
}

void boid_flee(boid* b, vector* v, float weight)
{
	vector heading;
	vector_copy(&heading, v);
	vector_sub(&heading, &b->location);

	vector_normalize(&heading);

	vector_mul_scalar(&heading, weight);

	vector_sub(&b->acceleration, &heading);
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


#include "flock.h"

#include <math.h>

flock* flock_create(configuration* config)
{
	flock* f = malloc(sizeof(flock));

	f->location = malloc(sizeof(vector) * config->flock.size);
	f->acceleration = malloc(sizeof(vector) * config->flock.size);
	f->velocity = malloc(sizeof(vector) * config->flock.size);

	int i;
	for(i = 0; i < config->flock.size; i++)
	{
		f->location[i].x = rand_range(0.0f, config->video.screen_width);
		f->location[i].y = rand_range(0.0f, config->video.screen_height);
		f->location[i].z = 0;

		f->velocity[i].x = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);
		f->velocity[i].y = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);

        }

	return f;
}

void flock_destroy(flock* f)
{
	free(f->location);
	free(f->acceleration);
	free(f->velocity);

	free(f);
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
			current_fps = (last_fps * 0.8) + (current_fps * 0.2);

			start_fcount = *args->frame_count;
			start_time_fps = SDL_GetTicks();
		}

		if(*args->update_count >= (start_ucount + sample_size))
		{
			int end_time = SDL_GetTicks();

			last_ups = current_ups;
			current_ups = 1000 / ((end_time - start_time_ups) / sample_size);
			current_ups = (last_ups * 0.8) + (current_ups * 0.2);

			start_ucount = *args->update_count;
			start_time_ups = SDL_GetTicks();
		}

		printf("\rFrames per second: %i, Updates per second: %i ", current_fps, current_ups);
		fflush(stdout);
	}

	printf("\n");

	return 0;
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
		flock_influence(&args->f->acceleration[i], args->f, i, args->config);

		// Handle mouse input
		switch(*args->cursor_interaction)
		{
			case 0: break;
			case 1: if(vector_distance(&args->f->location[i], args->cursor_pos) < args->config->input.influence_radius)
					boid_approach(args->f, i, args->cursor_pos, args->config->flock.max_velocity / 4); break;
			case 2: if(vector_distance(&args->f->location[i], args->cursor_pos) < args->config->input.influence_radius)
					boid_flee(args->f, i, args->cursor_pos, args->config->flock.max_velocity / 4); break;
			default: break;
		};

		vector_add(&args->f->velocity[i], &args->f->acceleration[i]);
		vector_add(&args->f->location[i], &args->f->velocity[i]);

		// Reset the acceleration vectors for the flock
		vector_zero(&args->f->acceleration[i]);

		// Wrap coordinates
		args->f->location[i].x -= args->config->video.screen_width * (args->f->location[i].x > args->config->video.screen_width);
		args->f->location[i].x += args->config->video.screen_width * (args->f->location[i].x < 0);

		args->f->location[i].y -= args->config->video.screen_height * (args->f->location[i].y > args->config->video.screen_height);
		args->f->location[i].y += args->config->video.screen_height * (args->f->location[i].y < 0);
	}

	return 0;
}

int flock_update_thread(void* arg)
{
	flock_update_args* args = (flock_update_args*)arg;

	SDL_Thread** workers = malloc(sizeof(SDL_Thread*) * args->config->num_threads);
	flock_update_worker_args* worker_args = malloc(sizeof(flock_update_worker_args) * args->config->num_threads);

	int i;
	for(i = 0; i < args->config->num_threads; i++)
		worker_args[i] = (flock_update_worker_args){i, args->f, args->config, args->cursor_pos, args->cursor_interaction};

	while(*args->run)
	{
		for(i = 0; i < args->config->num_threads; i++)
			workers[i] = SDL_CreateThread(flock_update_worker_thread, (void*)&worker_args[i]);

		for(i = 0; i < args->config->num_threads; i++)
			SDL_WaitThread(workers[i], NULL);

		*args->update_count += args->config->num_threads;
	}

	free(worker_args);
	free(workers);

	return 0;
}

void flock_influence(vector* v, flock* f, int boid_id, configuration* config)
{
     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vector influence[2];

	int i;
	for(i = 0; i < 2; i++)
		vector_zero(&influence[i]);

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2] = {0, 0};

	register float neighborhood_radius_squared = powf(config->flock.neighborhood_radius, 2);
	register float min_boid_separation_squared = powf(config->flock.min_separation, 2);

	for(i = 0; i < config->flock.size; i++)
	{
		register float distance = vector_distance_nosqrt(&f->location[i], &f->location[boid_id]);

		vector temp;
		if(distance <= min_boid_separation_squared)
		{
			vector_copy(&temp, &f->location[boid_id]);
			vector_sub(&temp, &f->location[i]);
			vector_normalize(&temp);
			vector_add(&influence[1], &temp);

			population[1]++;
		}
		else if(distance <= neighborhood_radius_squared)
		{
			vector_copy(&temp, &f->velocity[i]);
			vector_normalize(&temp);
			vector_add(&influence[0], &temp);

			population[0]++;
		}
	}

	for(i = 0; i < 2; i++)
	{
		if(vector_magnitude(&influence[i]) > 0)
		{
			vector_div_scalar(&influence[i], population[i]);

			vector_normalize(&influence[i]);
			vector_mul_scalar(&influence[i], config->flock.max_velocity);
			vector_sub(&influence[i], &f->velocity[boid_id]);
			vector_mul_scalar(&influence[i], config->flock.max_steering_force);

			vector_add(v, &influence[i]);
		}
	}
}

void boid_approach(flock* f, int boid_id, vector* v, float weight)
{
	vector heading;
	vector_copy(&heading, v);
	vector_sub(&heading, &f->location[boid_id]);

	vector_normalize(&heading);

	vector_mul_scalar(&heading, weight);

	vector_add(&f->acceleration[boid_id], &heading);
}

void boid_flee(flock* f, int boid_id, vector* v, float weight)
{
	vector heading;
	vector_copy(&heading, v);
	vector_sub(&heading, &f->location[boid_id]);

	vector_normalize(&heading);

	vector_mul_scalar(&heading, weight);

	vector_sub(&f->acceleration[boid_id], &heading);
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


#include "flock.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#define TPS_BUFFER_SIZE 5

flock* flock_create(configuration* config)
{
	flock* f = calloc(1, sizeof(flock));

	f->location = calloc(config->flock.size, sizeof(vec2_t));
	f->acceleration = calloc(config->flock.size, sizeof(vec2_t));
	f->velocity = calloc(config->flock.size, sizeof(vec2_t));

	flock_randomize_location(f, config);
	flock_randomize_velocity(f, config);

	return f;
}

void flock_destroy(flock* f) {
	free(f->location);
	free(f->acceleration);
	free(f->velocity);

	free(f);
}

void flock_randomize_location(flock* f, configuration* config) {

	for(int i = 0; i < config->flock.size; i++)
	{
		f->location[i][0] = rand_range(0.0f, config->video.screen_width);
		f->location[i][1] = rand_range(0.0f, config->video.screen_height);

	}

}

void flock_randomize_velocity(flock* f, configuration* config)
{
	for(int i = 0; i < config->flock.size; i++)
	{
		float* mv = &config->flock.max_velocity;
		f->velocity[i][0] = rand_range(-(*mv), *mv);
		f->velocity[i][1] = rand_range(-(*mv), *mv);
	}
}

static void boid_wrap_coord(flock *f, int idx, configuration *config)
{
	int sw = config->video.screen_width, sh = config->video.screen_height;

	f->location[idx][0] -= sw * (f->location[idx][0] > sw);
	f->location[idx][0] += sw * (f->location[idx][0] < 0);

	f->location[idx][1] -= sh * (f->location[idx][1] > sh);
	f->location[idx][1] += sh * (f->location[idx][1] < 0);
}

void* flock_update_worker_thread(void* arg)
{
	flock_update_worker_args* args = (flock_update_worker_args*)arg;

	int work_size = args->config->flock.size / args->config->num_threads;
	int begin_work = work_size * args->thread_id;

	if(args->thread_id == args->config->num_threads - 1)
		work_size += args->config->flock.size % args->config->num_threads;
	int end_work = begin_work + work_size;

	long tps_buffer[TPS_BUFFER_SIZE];
	struct timespec curr_time, new_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);

	while(*args->run)
	{

		clock_gettime(CLOCK_MONOTONIC, &new_time);
		long tick_time_nsec = 	(new_time.tv_nsec - curr_time.tv_nsec) +
					(1000000000 * (new_time.tv_sec - curr_time.tv_sec));
		curr_time = new_time;

		long ticks_per_second = 1000000000 / tick_time_nsec;

		for(int i = TPS_BUFFER_SIZE - 1; i != 0; i--) tps_buffer[i] = tps_buffer[i - 1];
		tps_buffer[0] = ticks_per_second;

		long tps_avg = 0;
		for(int i = 0; i < TPS_BUFFER_SIZE; i++) tps_avg += tps_buffer[i];
		tps_avg /= TPS_BUFFER_SIZE;
		args->ticks[args->thread_id] = tps_avg;

		for(int i = begin_work; i < end_work; i++)
		{
			// Calculate boid movement
			float delta = args->config->flock.max_velocity * (60.0 / tps_avg);
			flock_influence(&args->f->acceleration[i], args->f, i, delta, args->config);

			// Handle mouse input
			switch(*args->cursor_interaction)
			{
				case 0: break;
				case 1: if(vec2_distance(args->f->location[i], *args->cursor_pos) < args->config->input.influence_radius)
						boid_approach(args->f, i, *args->cursor_pos, (INFLUENCE_WEIGHT / tps_avg)); break;
				case 2: if(vec2_distance(args->f->location[i], *args->cursor_pos) < args->config->input.influence_radius)
						boid_flee(args->f, i, *args->cursor_pos, (INFLUENCE_WEIGHT / tps_avg)); break;
				default: break;
			};

			vec2_add(args->f->velocity[i], args->f->acceleration[i]);
			vec2_add(args->f->location[i], args->f->velocity[i]);

			// Reset the acceleration vectors for the flock
			vec2_zero(args->f->acceleration[i]);

			// Wrap location coordinates
			boid_wrap_coord(args->f, i, args->config);
		}

		++(*args->ticks);
	}

	return NULL;
}

void flock_influence(vec2_t* v, flock* f, int boid_id, float max_velocity, configuration* config)
{
     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vec2_t influence[2];

	for(int i = 0; i < 2; i++)
		vec2_zero(influence[i]);

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2] = {0, 0};

	register float neighborhood_radius_squared = powf(config->flock.neighborhood_radius, 2);
	register float min_boid_separation_squared = powf(config->flock.min_separation, 2);

	const int sample_size = 10;
	int sample_indices[sample_size] = {0};     // A sample of boid indices that are within the neighborhood radius of flock[boid_id]
	float sample_distances[sample_size] = {0}; // distance^2 from flock[boid_id] to flock[sample_indices[i]]

	// Giving the sample a random offset every frame helps separation be more effective
	int offset = rand() % config->flock.size;
	for(int i = offset, s = 0; i != (offset - 1) && s < sample_size; i++)
	{
		if(i == config->flock.size) i = 0;

		float distance = vec2_distance_squared(f->location[i], f->location[boid_id]);
		if(distance <= neighborhood_radius_squared)
		{
			sample_distances[s] = distance;
			sample_indices[s++] = i;
		}
	}

	for(int idx = 0; idx < sample_size; idx++)
	{
		vec2_t heading;
		if(sample_distances[idx] <= min_boid_separation_squared)
		{
			// Separation
			vec2_copy(heading, f->location[sample_indices[idx]]);
			vec2_sub(heading, f->location[boid_id]);
			vec2_normalize(&heading);

			vec2_sub(influence[1], heading);

			population[1]++;
		}
		else
		{
			// Alignment
			vec2_copy(heading, f->velocity[sample_indices[idx]]);
			vec2_normalize(&heading);

			vec2_add(influence[0], heading);

			// Cohesion
			vec2_copy(heading, f->location[sample_indices[idx]]);
			vec2_sub(heading, f->location[boid_id]);
			vec2_normalize(&heading);

			// The cohesion is much too strong without weighting
			vec2_mul_scalar(heading, 0.15);

			vec2_add(influence[0], heading);

			population[0] += 2;
		}
	}

	for(int i = 0; i < 2; i++)
	{
		if(vec2_magnitude(influence[i]) > 0)
		{
			vec2_div_scalar(influence[i], population[i]);

			vec2_normalize(&influence[i]);
			vec2_mul_scalar(influence[i], max_velocity);
			vec2_sub(influence[i], f->velocity[boid_id]);
			vec2_mul_scalar(influence[i], config->flock.max_steering_force);

			for(int j = 0; j < 2; j++) (*v)[j] += influence[i][j];
		}
	}
}

void boid_approach(flock* f, int boid_id, vec2_t v, float weight)
{
	vec2_t heading;
	vec2_copy(heading, v);
	vec2_sub(heading, f->location[boid_id]);

	vec2_normalize(&heading);

	vec2_mul_scalar(heading, weight);

	vec2_add(f->acceleration[boid_id], heading);
}

void boid_flee(flock* f, int boid_id, vec2_t v, float weight)
{
	vec2_t heading;
	vec2_copy(heading, v);
	vec2_sub(heading, f->location[boid_id]);

	vec2_normalize(&heading);

	vec2_mul_scalar(heading, weight);

	vec2_sub(f->acceleration[boid_id], heading);
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


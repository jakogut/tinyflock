#include "flock.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GL/glfw.h>

int fractional_flock_size;
int* flock_sample;

flock* flock_create(configuration* config)
{
	flock* f = calloc(1, sizeof(flock));

	f->location = calloc(config->flock.size, sizeof(vec3_t));
	f->acceleration = calloc(config->flock.size, sizeof(vec3_t));
	f->velocity = calloc(config->flock.size, sizeof(vec3_t));

	for(int i = 0; i < config->flock.size; i++)
	{
		f->location[i].scalars.x = rand_range(0.0f, config->video.screen_width);
		f->location[i].scalars.y = rand_range(0.0f, config->video.screen_height);
		f->location[i].scalars.z = 0;

		f->velocity[i].scalars.x = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);
		f->velocity[i].scalars.y = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);
		f->velocity[i].scalars.z = 0;
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

void flock_update_worker_thread(void* arg)
{
	flock_update_worker_args* args = (flock_update_worker_args*)arg;

	int work_size = args->config->flock.size / args->config->num_threads;
	int begin_work = work_size * args->thread_id;

	if(args->thread_id == args->config->num_threads - 1) work_size += args->config->flock.size % args->config->num_threads;
	int end_work = begin_work + work_size - 1;

	printf("Thread ID: %i, Work Size: %i Begin Work: %i, End Work: %i\n", args->thread_id, work_size, begin_work, end_work);

	while(*args->run)
	{
		for(int i = begin_work; i < end_work; i++)
		{
			// Calculate boid movement
			flock_influence(&args->f->acceleration[i], args->f, i, args->config);

			// Handle mouse input
			switch(*args->cursor_interaction)
			{
				case 0: break;
				case 1: if(vec3_distance(args->f->location[i], *args->cursor_pos) < args->config->input.influence_radius)
						boid_approach(args->f, i, *args->cursor_pos, (args->config->flock.max_velocity / 8)); break;
				case 2: if(vec3_distance(args->f->location[i], *args->cursor_pos) < args->config->input.influence_radius)
						boid_flee(args->f, i, *args->cursor_pos, args->config->flock.max_velocity / 8); break;
				default: break;
			};

			vec3_add(args->f->velocity[i], args->f->acceleration[i]);
			vec3_add(args->f->location[i], args->f->velocity[i]);

			// Reset the acceleration vectors for the flock
			vec3_zero(args->f->acceleration[i]);

			// Wrap coordinates
			args->f->location[i].scalars.x -= args->config->video.screen_width * (args->f->location[i].scalars.x > args->config->video.screen_width);
			args->f->location[i].scalars.x += args->config->video.screen_width * (args->f->location[i].scalars.x < 0);

			args->f->location[i].scalars.y -= args->config->video.screen_height * (args->f->location[i].scalars.y > args->config->video.screen_height);
			args->f->location[i].scalars.y += args->config->video.screen_height * (args->f->location[i].scalars.y < 0);
		}

		++(*args->ticks);
	}
}

void flock_influence(vec3_t* v, flock* f, int boid_id, configuration* config)
{
     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vec3_t influence[2];

	for(int i = 0; i < 2; i++)
		vec3_zero(influence[i]);

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2] = {0, 0};

	register float neighborhood_radius_squared = powf(config->flock.neighborhood_radius, 2);
	register float min_boid_separation_squared = powf(config->flock.min_separation, 2);

	for(int idx = 0; idx < fractional_flock_size; idx++)
	{
		int i = flock_sample[idx];
		register float distance = vec3_distance_squared(f->location[i], f->location[boid_id]);

		vec3_t heading;
		if(distance <= min_boid_separation_squared)
		{
			// Separation
			vec3_copy(heading, f->location[i]);
			vec3_sub(heading, f->location[boid_id]);
			vec3_normalize(&heading);

			vec3_sub(influence[1], heading);

			population[1]++;
		}
		else if(distance <= neighborhood_radius_squared)
		{
			// Alignment
			vec3_copy(heading, f->velocity[i]);
			vec3_normalize(&heading);

			vec3_add(influence[0], heading);

			// Cohesion
			vec3_copy(heading, f->location[i]);
			vec3_sub(heading, f->location[boid_id]);
			vec3_normalize(&heading);

			// The cohesion is much too strong without weighting
			vec3_mul_scalar(heading, 0.1);

			vec3_add(influence[0], heading);

			population[0] += 2;
		}
	}

	for(int i = 0; i < 2; i++)
	{
		if(vec3_magnitude(influence[i]) > 0)
		{
			vec3_div_scalar(influence[i], population[i]);

			vec3_normalize(&influence[i]);
			vec3_mul_scalar(influence[i], config->flock.max_velocity);
			vec3_sub(influence[i], f->velocity[boid_id]);
			vec3_mul_scalar(influence[i], config->flock.max_steering_force);

			for(int j = 0; j < 3; j++) v->xyz[j] += influence[i].xyz[j];
		}
	}
}

void boid_approach(flock* f, int boid_id, vec3_t v, float weight)
{
	vec3_t heading;
	vec3_copy(heading, v);
	vec3_sub(heading, f->location[boid_id]);

	vec3_normalize(&heading);

	vec3_mul_scalar(heading, weight);

	vec3_add(f->acceleration[boid_id], heading);
}

void boid_flee(flock* f, int boid_id, vec3_t v, float weight)
{
	vec3_t heading;
	vec3_copy(heading, v);
	vec3_sub(heading, f->location[boid_id]);

	vec3_normalize(&heading);

	vec3_mul_scalar(heading, weight);

	vec3_sub(f->acceleration[boid_id], heading);
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}


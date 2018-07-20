#include "flock.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

struct flock* flock_create(struct configuration* config)
{
	struct flock* f = calloc(1, sizeof(struct flock));

	f->config = config;

	f->location = calloc(config->flock.size, sizeof(vec2_t));
	f->acceleration = calloc(config->flock.size, sizeof(vec2_t));
	f->velocity = calloc(config->flock.size, sizeof(vec2_t));

	flock_randomize_location(f);
	flock_randomize_velocity(f);

	return f;
}

void flock_destroy(struct flock* f)
{
	free(f->location);
	free(f->acceleration);
	free(f->velocity);

	free(f);
}

void flock_randomize_location(struct flock* f)
{
	for(int i = 0; i < f->config->flock.size; i++) {
		f->location[i][0] = rand_range(0.0f, f->config->video.screen_width);
		f->location[i][1] = rand_range(0.0f, f->config->video.screen_height);

	}
}

void flock_randomize_velocity(struct flock* f)
{
	for(int i = 0; i < f->config->flock.size; i++) {
		float* mv = &f->config->flock.max_velocity;
		f->velocity[i][0] = rand_range(-(*mv), *mv);
		f->velocity[i][1] = rand_range(-(*mv), *mv);
	}
}

static void boid_wrap_coord(struct flock *f, int idx)
{
	int sw = f->config->video.screen_width, sh = f->config->video.screen_height;

	f->location[idx][0] -= sw * (f->location[idx][0] > sw);
	f->location[idx][0] += sw * (f->location[idx][0] < 0);

	f->location[idx][1] -= sh * (f->location[idx][1] > sh);
	f->location[idx][1] += sh * (f->location[idx][1] < 0);
}

void flock_update(struct flock *f, float tps_avg)
{
	for(int i = 0; i < f->config->flock.size; i++) {
		// Calculate boid movement
		float delta = f->config->flock.max_velocity * (60.0 / tps_avg);
		flock_influence(&f->acceleration[i], f, i, delta);

		vec2_add(f->velocity[i], f->acceleration[i]);
		vec2_add(f->location[i], f->velocity[i]);

		// Reset the acceleration vectors for the flock
		vec2_zero(f->acceleration[i]);

		// Wrap location coordinates
		boid_wrap_coord(f, i);
	}
}

void flock_influence(vec2_t* v, struct flock* f, int boid_id, float max_velocity)
{
     /* influence[0] = alignment & cohesion,
	influence[2] = separation */
	vec2_t influence[2];

	for(int i = 0; i < 2; i++)
		vec2_zero(influence[i]);

     /* The first population is a total of the boids within the neighborhood of the target boid.
	The second population is a total of the boids infringing on the target boid's space.*/
	int population[2] = {0, 0};

	float nbhd_rad_sqd = powf(f->config->flock.neighborhood_radius, 2);
	float min_bsep_sqd = powf(f->config->flock.min_separation, 2);

	/* This is an interesting bit of code that reduces the problem space of the flocking function dramatically,
	 * while changing the original O(n^n) complexity to O(n). The idea came from a paper I read a while back that
	 * I can no longer source, which described how real flocks tend not to observe neighbors as individuals,
	 * but rather the flock as a whole. We can imitate this by only considering a small random subset of the
	 * flock per frame. In testing, I've found that it holds up with surprisingly small samples. */

	const int sample_size = 10;
	int sample_indices[sample_size] = {0};
	float sample_distances[sample_size] = {0};

	/* We want a fairly random subset of the flock here, but using a rand() for each boid is very slow,
	 * not to mention thread-unsafe. Given the chaotic nature of the flock, we can simply get one rand(),
	 * and use it as an offset for the starting point of our search for other boids inside our neighborhood.
	 * This gives us a surprisingly efficient and effective sample generation method */
	int offset = rand() % f->config->flock.size;
	for(int i = offset, s = 0; i != (offset - 1) && s < sample_size; i++) {
		// If we reach the end of the flock without filling the queue, loop around
		if(i == f->config->flock.size) i = 0;

		float distance = vec2_distance_squared(f->location[i], f->location[boid_id]);
		if(distance <= nbhd_rad_sqd) {
			sample_distances[s] = distance;
			sample_indices[s++] = i;
		}
	}

	for(int idx = 0; idx < sample_size; idx++)
	{
		vec2_t heading;
		if(sample_distances[idx] <= min_bsep_sqd) {
			// Separation
			vec2_copy(heading, f->location[sample_indices[idx]]);
			vec2_sub(heading, f->location[boid_id]);
			vec2_normalize(&heading);

			vec2_sub(influence[1], heading);

			population[1]++;
		}
		else {
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

	for(int i = 0; i < 2; i++) {
		if(vec2_magnitude(influence[i]) > 0) {
			vec2_div_scalar(influence[i], population[i]);

			vec2_normalize(&influence[i]);
			vec2_mul_scalar(influence[i], max_velocity);
			vec2_sub(influence[i], f->velocity[boid_id]);
			vec2_mul_scalar(influence[i], f->config->flock.max_steering_force);

			for(int j = 0; j < 2; j++) (*v)[j] += influence[i][j];
		}
	}
}

void boid_approach(struct flock* f, int boid_id, vec2_t v, float weight)
{
	vec2_t heading;
	vec2_copy(heading, v);
	vec2_sub(heading, f->location[boid_id]);

	vec2_normalize(&heading);

	vec2_mul_scalar(heading, weight);

	vec2_add(f->acceleration[boid_id], heading);
}

void boid_flee(struct flock* f, int boid_id, vec2_t v, float weight)
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

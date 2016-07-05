#include "flock.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <fann.h>
#include <GLFW/glfw3.h>

#define TPS_BUFFER_SIZE 5

#ifdef __WIN32
#define CORE_COUNT omp_get_num_threads()
#else
#define CORE_COUNT sysconf(_SC_NPROCESSORS_ONLN)
#endif

struct flock* flock_create(struct configuration* config)
{
	struct flock* f = calloc(1, sizeof(struct flock));

	f->config = config;

	f->location = calloc(config->flock.size, sizeof(vec2_t));
	f->acceleration = calloc(config->flock.size, sizeof(vec2_t));
	f->velocity = calloc(config->flock.size, sizeof(vec2_t));

	unsigned nthreads = CORE_COUNT;

	f->history = calloc(1, sizeof(struct flock_snapshot));
	f->history_tail = f->history;

	f->sample.size = f->config->flock.size * 0.0015;
	f->sample.indices = calloc(sizeof(int*), nthreads);
	f->sample.distances = calloc(sizeof(float*), nthreads);

	for (int i = 0; i < nthreads; i++) {
		f->sample.indices[i] = calloc(sizeof(int), f->sample.size);
		f->sample.distances[i] = calloc(sizeof(float), f->sample.size);
	}

	flock_randomize_location(f);
	flock_randomize_velocity(f);

	return f;
}

void flock_destroy(struct flock* f)
{
	for (int i = 0; i < CORE_COUNT; i++) {
		free(f->sample.indices[i]);
		free(f->sample.distances[i]);
	}

	free(f->sample.indices);
	free(f->sample.distances);

	free(f->location);
	free(f->acceleration);
	free(f->velocity);

	free(f);
}

void flock_snapshot(struct flock *f, int boid_id, int *sample)
{
	f->history_tail->location = malloc(sizeof(vec2_t) * f->sample.size + 1);
	f->history_tail->velocity = malloc(sizeof(vec2_t) * f->sample.size + 1);
	f->history_tail->acceleration = malloc(sizeof(vec2_t));

	for (int i = 0; i < f->sample.size + 1; i++) {
		int src_idx = (i == 0 ? boid_id : sample[i]);
		vec2_copy(f->history_tail->location[i], f->location[src_idx]);
		vec2_copy(f->history_tail->velocity[i], f->velocity[src_idx]);
	}

	vec2_copy(f->history_tail->acceleration, f->acceleration[boid_id]);

	f->history_tail->next = calloc(1, sizeof(struct flock_snapshot));
	f->history_tail = f->history_tail->next;
}

int write_history(char *filename, struct flock *f)
{
	FILE *filp = fopen(filename, "w");
	if (filp == NULL) return -1;

	struct flock_snapshot *ptr = f->history;

	// There are four inputs per boid, X and Y of location and velocity,
	// and an additional boid for the target
	unsigned num_inputs = ((f->sample.size+1 * 2) * 2);
	unsigned num_samples = 0;

	while (ptr) {
		num_samples++;
		ptr = ptr->next;
	}

	printf("\nWriting flock history to FANN training file\n");

	fprintf(filp, "%i %i 2\n", num_samples, num_inputs);

	int idx = 0;
	int pct_complete = 0;

	int screen_width  = f->config->video.screen_width,
	    screen_height = f->config->video.screen_height;

	ptr = f->history;
	while (ptr) {
		if (!ptr->location || !ptr->velocity || !ptr->acceleration)
			break;

		for (int i = 0; i < f->sample.size+1; i++) {
			fprintf(filp, "%.6f ", ((0.5 * screen_width) - ptr->location[i][0]) / screen_width);
			fprintf(filp, "%.6f ", ((0.5 * screen_height) - ptr->location[i][1]) / screen_height);

			for (int j = 0; j < 2; j++)
				fprintf(filp, "%.6f ", ptr->velocity[i][j] / f->config->flock.max_velocity);
		}

		fprintf(filp, "\n");

		for (int i = 0; i < 2; i++)
			fprintf(filp, "%.6f ", (*ptr->acceleration)[i]);

		fprintf(filp, "\n");

		int new_pct = ((100.0f / num_samples) * idx);
		if (new_pct != pct_complete) {
			printf("\r%i percent complete", new_pct);
			fflush(stdout);

			pct_complete = new_pct;
		}

		ptr = ptr->next;
		idx++;
	}
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

static inline void boid_wrap_coord(struct flock *f, int idx)
{
	int sw = f->config->video.screen_width, sh = f->config->video.screen_height;

	f->location[idx][0] -= sw * (f->location[idx][0] > sw);
	f->location[idx][0] += sw * (f->location[idx][0] < 0);

	f->location[idx][1] -= sh * (f->location[idx][1] > sh);
	f->location[idx][1] += sh * (f->location[idx][1] < 0);
}

static long tps_buffer[TPS_BUFFER_SIZE];

void *flock_update(void *arg)
{
	struct update_thread_arg *args = (struct update_thread_arg *)arg;

	struct timespec curr_time, new_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);

	while(*args->run) {
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
		*args->ticks = tps_avg;

		omp_lock_t snapshot_lock;
		omp_init_lock(&snapshot_lock);

		#pragma omp parallel for
		for(int i = 0; i < args->f->config->flock.size; i++) {
			// Calculate boid movement
			float delta = args->f->config->flock.max_velocity * (60.0 / tps_avg);
			if (args->f->config->mode == TF_MODE_FLOCK_CONV)
				flock_influence(&args->f->acceleration[i], args->f, i, delta);
			else if (args->f->config->mode == TF_MODE_FLOCK_NN)
				flock_influence_nn(&args->f->acceleration[i], args->f, i, delta);

			int tid = omp_get_thread_num();
			
			if (strlen(args->f->config->capture_filename)) {
				omp_set_lock(&snapshot_lock);
				flock_snapshot(args->f, i, args->f->sample.indices[tid]);
				omp_unset_lock(&snapshot_lock);
			}

			// Handle mouse input
			switch(*args->cursor_interaction)
			{
				case 0: break;
				case 1: if(vec2_distance(args->f->location[i], *args->cursor_pos) < args->f->config->input.influence_radius)
						boid_approach(args->f, i, *args->cursor_pos, (INFLUENCE_WEIGHT / tps_avg)); break;
				case 2: if(vec2_distance(args->f->location[i], *args->cursor_pos) < args->f->config->input.influence_radius)
						boid_flee(args->f, i, *args->cursor_pos, (INFLUENCE_WEIGHT / tps_avg)); break;
				default: break;
			};


			vec2_add(args->f->velocity[i], args->f->acceleration[i]);
			vec2_add(args->f->location[i], args->f->velocity[i]);

			// Reset the acceleration vectors for the flock
			vec2_zero(args->f->acceleration[i]);

			// Wrap location coordinates
			boid_wrap_coord(args->f, i);
		}

	}

	if (strlen(args->f->config->capture_filename)) {
		int res;
		res = write_history(args->f->config->capture_filename, args->f);
	}

	return NULL;
}

/* This is an interesting bit of code that reduces the problem space of the flocking function dramatically,
 * while changing the original O(n^n) complexity to O(n). The idea came from a paper I read a while back that
 * I can no longer source, which described how real flocks tend not to observe neighbors as individuals,
 * but rather the flock as a whole. We can imitate this by only considering a small random subset of the
 * flock per frame. In testing, I've found that it holds up with surprisingly small samples.  */
static void flock_gen_sample(struct flock *f, unsigned boid_id)
{
	/* We want a fairly random subset of the flock here, but using a rand() for each boid is very slow,
	 * not to mention thread-unsafe. Given the chaotic nature of the flock, we can simply get one rand(),
	 * and use it as an offset for the starting point of our search for other boids inside our neighborhood.
	 * This gives us a surprisingly efficient and effective sample generation method */

	unsigned tid = omp_get_thread_num();

	memset(f->sample.indices[tid],   -1, f->sample.size * sizeof(int));
	memset(f->sample.distances[tid], -1, f->sample.size * sizeof(float));

	int offset = rand() % f->config->flock.size;
	for(int i = offset, s = 0; i != (offset - 1) && s < f->sample.size; i++) {
		// If we reach the end of the flock without filling the queue, loop around
		if (i == f->config->flock.size) i = 0;

		float distance = vec2_distance_squared(f->location[i], f->location[boid_id]);
		float nbhd_rad_sqd = powf(f->config->flock.neighborhood_radius, 2);

		if (distance <= nbhd_rad_sqd) {
			f->sample.distances[tid][s] = distance;
			f->sample.indices[tid][s++] = i;
		}

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
	float min_bsep_sqd = powf(f->config->flock.min_separation, 2);
	unsigned tid = omp_get_thread_num();

	flock_gen_sample(f, boid_id);

	for(int idx = 0; idx < f->sample.size; idx++)
	{
		if(f->sample.indices[tid][idx] == -1)
			continue;

		vec2_t heading;
		if(f->sample.distances[tid][idx] <= min_bsep_sqd) {
			// Separation
			vec2_copy(heading, f->location[f->sample.indices[tid][idx]]);
			vec2_sub(heading, f->location[boid_id]);
			vec2_normalize(&heading);

			vec2_sub(influence[1], heading);

			population[1]++;
		}
		else {
			// Alignment
			vec2_copy(heading, f->velocity[f->sample.indices[tid][idx]]);
			vec2_normalize(&heading);

			vec2_add(influence[0], heading);

			// Cohesion
			vec2_copy(heading, f->location[f->sample.indices[tid][idx]]);
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

void flock_influence_nn(vec2_t *v, struct flock *f, int boid_id, float max_velocity)
{
	flock_gen_sample(f, boid_id);
	unsigned tid = omp_get_thread_num();

	fann_type *input = malloc(sizeof(fann_type) * f->sample.size);
	fann_type *output;

	struct fann *ann = fann_create_from_file((const char *)f->config->flock_nn.trained_net);

	if (ann == NULL) { 
		printf("ANN creation failed!\n");
		return;
	}

	if (input == NULL) {
		printf("Failed creating ANN input array!\n");
		return;
	}

	for (int i = 0; i < f->sample.size+1; i++) {
		int idx = 0;
		if (i == 0) idx = boid_id;
		else idx = f->sample.indices[tid][i-1];

		input[i*4+0] = f->location[idx][0];
		input[i*4+1] = f->location[idx][1];
		input[i*4+2] = f->velocity[idx][0];
		input[i*4+3] = f->velocity[idx][1];
	}

	output = fann_run(ann, input);
	vec2_copy(v, output);

	if (input) free(input);
	fann_destroy(ann);
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
